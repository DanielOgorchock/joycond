#include <iostream>
#include <libudev.h>
#include <sys/epoll.h>

#include "phys_ctlr.h"

const int MAX_EVENTS = 10;

int main(int argc, char *argv[])
{
    struct udev *udev;
    struct udev_monitor *mon;
    int udev_mon_fd;
    int epoll_fd;
    struct epoll_event udev_mon_event;
    struct epoll_event events[MAX_EVENTS];

    udev = udev_new();
    if (!udev) {
        std::cerr << "Failed to create udev\n";
        return 1;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_tag(mon, "joycond");
    udev_monitor_enable_receiving(mon);
    udev_mon_fd = udev_monitor_get_fd(mon);

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

        nfds = epoll_pwait(epoll_fd, events, MAX_EVENTS, -1, nullptr);
        if (nfds == -1) {
            std::cerr << "epoll_pwait failure\n";
            return 1;
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == udev_mon_fd) {
                struct udev_device *dev;

                dev = udev_monitor_receive_device(mon);
                if (dev) {
                    std::cout << "DEVNAME="    << udev_device_get_sysname(dev);
                    std::cout << " ACTION="    << udev_device_get_action(dev);
                    std::cout << " DEVPATH="   << udev_device_get_devpath(dev);
                    std::cout << std::endl;

                    if (std::string("add") == udev_device_get_action(dev)) {
                        phys_ctlr test { udev_device_get_devpath(dev) };

                        test.set_home_led(12);
                        test.set_player_led(0, true);
                        test.set_player_led(1, false);
                        test.set_player_led(2, false);
                        test.set_player_led(3, true);
                    }
                }
            }
        }
    }

    return 0;
}
