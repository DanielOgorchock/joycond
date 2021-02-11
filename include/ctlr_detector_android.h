#ifndef JOYCOND_CTLR_DETECTOR_ANDROID_H
#define JOYCOND_CTLR_DETECTOR_ANDROID_H

#include "ctlr_mgr.h"
#include "epoll_mgr.h"

class ctlr_detector_android
{
    private:
        ctlr_mgr& ctlr_manager;
        epoll_mgr& epoll_manager;
        std::shared_ptr<epoll_subscriber> subscriber;

        std::map<std::string, std::string> ctlr_dev_map;
        std::map<std::string, std::string> ctlr_mac_map;

        bool check_ctlr_attributes(std::string devpath);
        void scan_removed_ctlrs();
        void epoll_event_callback(int event_fd);
    public:
        ctlr_detector_android(ctlr_mgr& ctlr_manager, epoll_mgr& epoll_manager );
        ~ctlr_detector_android();
};

#endif
