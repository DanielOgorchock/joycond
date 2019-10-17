#include "ctlr_detector_udev.h"

#include <iostream>
#include <libudev.h>
#include <stdlib.h>

//private
void ctlr_detector_udev::epoll_event_callback(int event_fd)
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

        if (std::string("add") == action) {
            ctlr_manager.add_ctlr(udev_device_get_devpath(dev), udev_device_get_devnode(dev));
        } else if (std::string("remove") == action) {
            ctlr_manager.remove_ctlr(udev_device_get_devpath(dev));
        }
        udev_device_unref(dev);
    }
}

//public
ctlr_detector_udev::ctlr_detector_udev(ctlr_mgr& ctlr_manager, epoll_mgr& epoll_manager) :
    ctlr_manager(ctlr_manager),
    epoll_manager(epoll_manager)
{
    udev = udev_new();
    if (!udev) {
        std::cerr << "Failed to create udev\n";
        exit(EXIT_FAILURE);
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon) {
        std::cerr << "Failed to create udev monitor\n";
        exit(EXIT_FAILURE);
    }
    udev_monitor_filter_add_match_tag(mon, "joycond");
    udev_monitor_enable_receiving(mon);
    udev_mon_fd = udev_monitor_get_fd(mon);

    subscriber = std::make_shared<epoll_subscriber>(std::vector({udev_mon_fd}),
                                                    [=](int event_fd){epoll_event_callback(event_fd);});
    epoll_manager.add_subscriber(subscriber);

    // Detect any existing controllers prior to daemon start
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devlist;
    struct udev_list_entry *deventry;

    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        std::cerr << "Failed to create new udev enumeration\n";
        exit(EXIT_FAILURE);
    }
    udev_enumerate_add_match_tag(enumerate, "joycond");
    udev_enumerate_scan_devices(enumerate);
    devlist = udev_enumerate_get_list_entry(enumerate);
    if (devlist) {
        udev_list_entry_foreach(deventry, devlist) {
            char const *path = udev_list_entry_get_name(deventry);
            struct udev_device *dev = udev_device_new_from_syspath(udev, path);
            std::string devpath = udev_device_get_devpath(dev);
            std::string devname = udev_device_get_devnode(dev);

            ctlr_manager.add_ctlr(devpath, devname);
            udev_device_unref(dev);
        }
    }
    udev_enumerate_unref(enumerate);
}

ctlr_detector_udev::~ctlr_detector_udev()
{
    epoll_manager.remove_subscriber(subscriber);
}

