#include <iostream>
#include "ctlr_mgr.h"
#include "epoll_mgr.h"
#include "ctlr_detector_udev.h"

int main(int argc, char *argv[])
{
    epoll_mgr epoll_manager;
    ctlr_mgr ctlr_manager(epoll_manager);
    ctlr_detector_udev udev_detector(ctlr_manager, epoll_manager);

    while (true) {
        epoll_manager.loop();
    }

    return 0;
}
