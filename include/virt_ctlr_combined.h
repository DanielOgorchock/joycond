#ifndef JOYCOND_VIRT_CTLR_COMBINED
#define JOYCOND_VIRT_CTLR_COMBINED

#include "virt_ctlr.h"
#include "phys_ctlr.h"
#include "epoll_mgr.h"

#include <libevdev/libevdev.h>
#include <memory>

class virt_ctlr_combined : public virt_ctlr
{
    private:
        std::shared_ptr<phys_ctlr> physl;
        std::shared_ptr<phys_ctlr> physr;
        epoll_mgr& epoll_manager;
        std::shared_ptr<epoll_subscriber> subscriber;
        struct libevdev_uinput *uidev;
        int uifd;

        void relay_events(std::shared_ptr<phys_ctlr> phys);
        void handle_uinput_event();
    public:
        virt_ctlr_combined(std::shared_ptr<phys_ctlr> physl, std::shared_ptr<phys_ctlr> physr, epoll_mgr& epoll_manager);
        virtual ~virt_ctlr_combined();

        virtual void handle_events(int fd);
        virtual bool contains_phys_ctlr(std::shared_ptr<phys_ctlr> const ctlr) const;
        virtual bool contains_phys_ctlr(char const *devpath) const;
        virtual bool contains_fd(int fd) const;
        virtual std::vector<std::shared_ptr<phys_ctlr>> get_phys_ctlrs();
        virtual int get_uinput_fd();
};

#endif
