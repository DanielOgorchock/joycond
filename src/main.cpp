#include <map>
#include <chrono>
#include <iostream>
#include <libudev.h>
#include <sys/epoll.h>

#include "phys_ctlr.h"

const int MAX_EVENTS = 10;

std::map<std::string, phys_ctlr *> pairing_ctlrs;

void add_new_ctlr(struct udev_device *dev)
{
    std::string devpath = udev_device_get_devpath(dev);
    std::string devname = udev_device_get_devnode(dev);

    if (!pairing_ctlrs.count(devpath)) {
        auto phys = new phys_ctlr(devpath, devname);
        std::cout << "Creating new phys_ctlr for " << devname << std::endl;
        pairing_ctlrs[devpath] = phys;
    }
}

int init_pairing_ctlrs(struct udev *udev)
{
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devlist;
    struct udev_list_entry *deventry;

    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        std::cerr << "Failed to create new udev enumeration\n";
        return 1;
    }
    udev_enumerate_add_match_tag(enumerate, "joycond");
    udev_enumerate_scan_devices(enumerate);
    devlist = udev_enumerate_get_list_entry(enumerate);
    if (!devlist) {
        std::cerr << "Failed to get udev enumeration list\n";
        return 1;
    }

    udev_list_entry_foreach(deventry, devlist) {
        char const *path = udev_list_entry_get_name(deventry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        std::string devpath = udev_device_get_devpath(dev);

        add_new_ctlr(dev);
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    return 0;
}

void udev_event_handler(struct udev_monitor *mon)
{
    struct udev_device *dev;

    dev = udev_monitor_receive_device(mon);
    if (dev) {
        std::string action = udev_device_get_action(dev);
        std::string devpath = udev_device_get_devpath(dev);

        std::cout << "DEVNAME="    << udev_device_get_sysname(dev);
        std::cout << " ACTION="    << udev_device_get_action(dev);
        std::cout << " DEVPATH="   << udev_device_get_devpath(dev);
        std::cout << std::endl;

        if (std::string("add") == action && !pairing_ctlrs.count(devpath)) {
            add_new_ctlr(dev);
        } else if (std::string("remove") == action) {
            if (pairing_ctlrs.count(devpath)) {
                delete pairing_ctlrs[devpath];
                pairing_ctlrs.erase(devpath);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    struct udev *udev;
    struct udev_monitor *mon;
    int udev_mon_fd;
    int epoll_fd;
    struct epoll_event udev_mon_event;
    struct epoll_event events[MAX_EVENTS];
    bool pairing_leds_on = false;

    udev = udev_new();
    if (!udev) {
        std::cerr << "Failed to create udev\n";
        return 1;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon) {
        std::cerr << "Failed to create udev monitor\n";
        return 1;
    }
    udev_monitor_filter_add_match_tag(mon, "joycond");
    udev_monitor_enable_receiving(mon);
    udev_mon_fd = udev_monitor_get_fd(mon);

    if (init_pairing_ctlrs(udev)) {
        std::cerr << "Failed to init pairing controllers list\n";
        return 1;
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Failed to create epoll; " << epoll_fd << std::endl;
        return 1;
    }

    udev_mon_event.events = EPOLLIN;
    udev_mon_event.data.fd = udev_mon_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udev_mon_fd, &udev_mon_event)) {
        std::cerr << "Failed to add udev_mon to epoll; errno=" << errno << std::endl;
        return 1;
    }

    while (true) {
        int nfds;

        nfds = epoll_pwait(epoll_fd, events, MAX_EVENTS, 500, nullptr);
        if (nfds == -1) {
            std::cerr << "epoll_pwait failure\n";
            return 1;
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == udev_mon_fd) {
                udev_event_handler(mon);
            }
        }

        for (auto& kv : pairing_ctlrs) {
            if (!kv.second->set_all_player_leds(pairing_leds_on))
                std::cerr << "Failed to set player LEDS\n";
        }
        pairing_leds_on = !pairing_leds_on;
    }

    udev_monitor_unref(mon);
    udev_unref(udev);
    return 0;
}
