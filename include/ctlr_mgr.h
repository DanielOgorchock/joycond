#ifndef JOYCOND_CTLR_MANAGER_H
#define JOYCOND_CTLR_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <vector>

#include "epoll_mgr.h"
#include "phys_ctlr.h"
#include "virt_ctlr.h"

class ctlr_mgr
{
    private:
        epoll_mgr& epoll_manager;
        std::map<std::string, std::shared_ptr<phys_ctlr>> unpaired_controllers;
        std::map<std::string, std::shared_ptr<epoll_subscriber>> subscribers;
        std::vector<std::unique_ptr<virt_ctlr>> paired_controllers;
        std::vector<std::unique_ptr<virt_ctlr>> stale_controllers;

        std::shared_ptr<phys_ctlr> left;
        std::shared_ptr<phys_ctlr> right;

        void epoll_event_callback(int event_fd);
        void add_passthrough_ctlr(std::shared_ptr<phys_ctlr> phys);
        void add_combined_ctlr();
        void add_virt_procon_ctlr(std::shared_ptr<phys_ctlr> phys);

    public:
        ctlr_mgr(epoll_mgr& epoll_manager);
        ~ctlr_mgr();

        void add_ctlr(const std::string& devpath, const std::string& devname);
        void remove_ctlr(const std::string& devpath);
};

#endif

