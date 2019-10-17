#ifndef JOYCOND_CTLR_DETECTOR_UDEV_H
#define JOYCOND_CTLR_DETECTOR_UDEV_H

#include "ctlr_mgr.h"
#include "epoll_mgr.h"

class ctlr_detector_udev
{
    private:
        ctlr_mgr& ctlr_manager;
        epoll_mgr& epoll_manager;
        std::shared_ptr<epoll_subscriber> subscriber;

        struct udev *udev;
        struct udev_monitor *mon;
        int udev_mon_fd;

        void epoll_event_callback(int event_fd);

    public:
        ctlr_detector_udev(ctlr_mgr& ctlr_manager, epoll_mgr& epoll_manager );
        ~ctlr_detector_udev();
};

#endif

