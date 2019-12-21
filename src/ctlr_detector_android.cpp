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
#include <linux/types.h>
#include <linux/netlink.h>
#include <netlink/msg.h>
#include <libevdev/libevdev.h>

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
   
    std::string devpath, devname, key, val;

    bool action = false; // Removed
    bool correct = false;

    while (pos < event_len) {
        std::istringstream iss(&buf[pos]);
        while(std::getline(std::getline(iss, key, '=') >> std::ws, val)) {
            if ((key.find("ACTION") != std::string::npos)) { //
                if (val == "add") {
                    correct = true;
                    action = true;
                } else if (val == "remove") {
                    correct = true;
                    action = false;
                }
            }

            if ((key == "SUBSYSTEM") && (val == "input"))
                correct = true && correct;

            if (key == "DEVPATH")
                devpath = val;

            if (key == "DEVNAME" && val.find("/dev/") == std::string::npos)
                devname = "/dev/" + val;
        }
        pos += strlen(&buf[pos]) + 1;
    }

    if (!correct)
        return;
    
    // Only accept event* devices and complete requests
    if (devpath.empty() || devname.empty() || (devname.find("event") == std::string::npos))
        return;

    // Sleep a bit to let driver load
    usleep(100000);

    // We obviously can't read the sysfs of a removed device
    if (!action) {
        ctlr_manager.remove_ctlr(devpath);
        return;
    }

    struct libevdev *evdev;

    // Open device to confirm the vendor and product id, not given in uevent
    int fd = open(devname.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open " << devname << " ; errno=" << errno << std::endl;
        exit(1);
    }
    if (libevdev_new_from_fd(fd, &evdev)) {
        std::cerr << "Failed to create evdev from fd\n";
        close(fd);
        return;
    }

    int pid = libevdev_get_id_product(evdev);
    int vid = libevdev_get_id_vendor(evdev);

    libevdev_free(evdev);
    close(fd);
    
    std::cout << "Input device connected vid: 0x" << std::hex << vid <<  " pid: 0x" << std::hex << pid << std::endl;
    if (vid != 0x57e)
        return;

    if (pid != 0x2009 && pid != 0x2007 && pid != 0x2006)
        return;

    ctlr_manager.add_ctlr(devpath, devname);
}

//public
ctlr_detector_android::ctlr_detector_android(ctlr_mgr& ctlr_manager, epoll_mgr& epoll_manager) :
    ctlr_manager(ctlr_manager),
    epoll_manager(epoll_manager)
{
    struct sockaddr_nl uevent_socket;
    struct pollfd uevent_pollfd;
    // Open hotplug event netlink socket
    memset(&uevent_socket,0,sizeof(struct sockaddr_nl));
    uevent_socket.nl_family = AF_NETLINK;
    uevent_socket.nl_pid = getpid();
    uevent_socket.nl_groups = -1;
    uevent_pollfd.events = POLLIN;
    uevent_pollfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (uevent_pollfd.fd==-1)
        std::cout << "Unable to create polling fd: EPERM" << std::endl;

    // Listen to netlink socket
    if (bind(uevent_pollfd.fd, (struct sockaddr*)&uevent_socket, sizeof(struct sockaddr_nl)))
        std::cout << "Unable to create bind poll fd" << std::endl;

    subscriber = std::make_shared<epoll_subscriber>(std::vector({uevent_pollfd.fd}),
                                                    [=](int event_fd){epoll_event_callback(event_fd);});
    epoll_manager.add_subscriber(subscriber);

}

ctlr_detector_android::~ctlr_detector_android()
{
    epoll_manager.remove_subscriber(subscriber);
}

