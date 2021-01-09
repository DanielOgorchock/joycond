#include "ctlr_detector_android.h"

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <netlink/msg.h>
#include <libevdev/libevdev.h>

//private
bool ctlr_detector_android::check_ctlr_attributes(std::string devpath)
{
    struct libevdev *evdev;

    // Open device to confirm the vendor and product id, not given in uevent
    int fd = open(devpath.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open " << devpath << " ; errno=" << errno << std::endl;
        return false;
    }

    if (libevdev_new_from_fd(fd, &evdev)) {
        std::cerr << "Failed to create evdev from fd" << devpath << "\n";
        close(fd);
        return false;
    }

    int pid = libevdev_get_id_product(evdev);
    int vid = libevdev_get_id_vendor(evdev);
    int is_accel = libevdev_has_property(evdev, INPUT_PROP_ACCELEROMETER);

    libevdev_free(evdev);
    close(fd);

    std::cout << "Input device connected vid: 0x" << std::hex << vid << " pid: 0x" << std::hex << pid << " accel: " << is_accel << std::endl;

    if (vid != 0x57e)
        return false;

    if (pid != 0x2009 && pid != 0x2007 && pid != 0x2006 && pid != 0x2017)
        return false;

    if (is_accel)
        return false;

    return true;
}

//private
void ctlr_detector_android::scan_removed_ctlrs() {
    // Scan all controllers we think are connected to double check they actually are, removing if needed

    for (auto ctlr : ctlr_dev_map) {
        if (access(ctlr.second.c_str(), F_OK)) {
            // Controller has been disconnected, it's event file is missing
            ctlr_manager.remove_ctlr(ctlr.first);
            ctlr_dev_map.erase(ctlr.first);
            return;
        }
    }
}

//private
void ctlr_detector_android::epoll_event_callback(int event_fd)
{
    char buf[8192];
    struct iovec event_iovec = { buf, sizeof(buf) };
    struct sockaddr_nl event_sockaddr;
    struct msghdr event_msg;
    int event_len, pos = 0;

    event_msg = { &event_sockaddr, sizeof(event_sockaddr), &event_iovec, 1, NULL, 0, 0 };
    event_len = recvmsg(event_fd, &event_msg, 0);

    std::string devpath, devnode, key, val;

    bool action = false;
    bool correct = false;

    scan_removed_ctlrs();

    while (pos < event_len) {
        std::istringstream iss(&buf[pos]);
        while (std::getline(std::getline(iss, key, '=') >> std::ws, val)) {
            if ((key.find("ACTION") != std::string::npos)) {
                if (val == "add") {
                    correct = true;
                    action = true;
                } else if (val == "remove") {
                    correct = true;
                    action = false;
                }
            }

            if (key == "SUBSYSTEM") {
                correct = (val == "input") && correct;
            }

            if (key == "DEVPATH")
                devpath = val;

            if (key == "DEVNAME" && val.find("/dev/") == std::string::npos)
                devnode = "/dev/" + val;
        }
        pos += strlen(&buf[pos]) + 1;
    }

    if (!correct)
        return;

    // Only accept event* devices and complete requests
    if (devpath.empty() || devnode.empty() || ((devnode.find("event") == std::string::npos) && (devnode.find("hid") == std::string::npos)))
        return;

    devpath = "/class/input/" + std::string(basename(devnode.c_str())) + "/device";

    // Sleep a bit to let driver load
    usleep(100000);

    // Check the MAC to handle replacements - disconnects are not reported instantly so otherwise we can end up desynced
    std::ifstream funiq("/sys/" + devpath + "/uniq");
    std::string mac_addr = "";
    std::getline(funiq, mac_addr);

    if (ctlr_mac_map.count(mac_addr)) {
        // Remove old controller
        ctlr_manager.remove_ctlr(ctlr_dev_map[ctlr_mac_map[mac_addr]]);
        ctlr_dev_map.erase(ctlr_mac_map[mac_addr]);
    }

    if (!action) {
        ctlr_dev_map.erase(devnode);
        std::cout << "Remove controller from map: " << devpath << std::endl;
        ctlr_manager.remove_ctlr(devpath);
        return;
    }

    if (check_ctlr_attributes(devnode)) {
        ctlr_manager.add_ctlr(devpath, devnode);
        std::cout << "Add controller to map: " << devpath << std::endl;
        ctlr_dev_map.insert({devpath, devnode});
        ctlr_mac_map.insert({mac_addr, devpath});
    }
}

//public
ctlr_detector_android::ctlr_detector_android(ctlr_mgr& ctlr_manager, epoll_mgr& epoll_manager) :
    ctlr_manager(ctlr_manager),
    epoll_manager(epoll_manager)
{
    struct sockaddr_nl uevent_socket;
    struct pollfd uevent_pollfd;
    struct dirent *event_dirent;
    std::string event_path;
    std::string sysfs_event_path;
    DIR *input_dir;

    input_dir = opendir("/dev/input/");

    while ((event_dirent = readdir(input_dir)) != NULL) {
        if (event_dirent->d_type & DT_DIR)
            continue;

        event_path = "/dev/input/" + std::string(event_dirent->d_name);
        sysfs_event_path = "/class/input/" + std::string(event_dirent->d_name) + "/device";

        if (check_ctlr_attributes(event_path)) {
            ctlr_manager.add_ctlr(sysfs_event_path, event_path);

            std::ifstream funiq("/sys/" + sysfs_event_path + "/uniq");
            std::string mac_addr = "";
            std::getline(funiq, mac_addr);

            std::cout << "Add controller to map: " << sysfs_event_path << std::endl;
            ctlr_dev_map.insert({sysfs_event_path, event_path});
            ctlr_mac_map.insert({mac_addr, sysfs_event_path});
        }
    }

    // Open netlink socket
    memset(&uevent_socket,0,sizeof(struct sockaddr_nl));
    uevent_socket.nl_family = AF_NETLINK;
    uevent_socket.nl_pid = getpid();
    uevent_socket.nl_groups = -1;
    uevent_pollfd.events = POLLIN;
    uevent_pollfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (uevent_pollfd.fd == -1) {
        std::cerr << "Unable to create polling fd:" << std::endl;
        return;
    }

    // Listen to netlink socket
    if (bind(uevent_pollfd.fd, (struct sockaddr*)&uevent_socket, sizeof(struct sockaddr_nl))) {
        std::cerr << "Unable to create bind poll fd" << std::endl;
        return;
    }

    subscriber = std::make_shared<epoll_subscriber>(std::vector({uevent_pollfd.fd}),
                                                    [=](int event_fd){epoll_event_callback(event_fd);});
    epoll_manager.add_subscriber(subscriber);

}

ctlr_detector_android::~ctlr_detector_android()
{
    epoll_manager.remove_subscriber(subscriber);
}

