#ifndef JOYCOND_VIRT_CTLR_COMBINED
#define JOYCOND_VIRT_CTLR_COMBINED

#include "virt_ctlr.h"
#include "phys_ctlr.h"
#include "epoll_mgr.h"

#include <libevdev/libevdev.h>
#include <map>
#include <memory>

class virt_ctlr_combined : public virt_ctlr
{
    private:
        std::shared_ptr<phys_ctlr> physl;
        std::shared_ptr<phys_ctlr> physr;
        epoll_mgr& epoll_manager;
        std::shared_ptr<epoll_subscriber> subscriber;
        struct libevdev *virt_evdev;
        struct libevdev_uinput *uidev;
        int uifd;
        std::map<int, std::pair<struct ff_effect, struct ff_effect>> rumble_effects;
        std::string left_mac;
        std::string right_mac;

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
        virtual void remove_phys_ctlr(const std::shared_ptr<phys_ctlr> phys);
        virtual void add_phys_ctlr(std::shared_ptr<phys_ctlr> phys);
        virtual enum phys_ctlr::Model needs_model();
        virtual bool supports_hotplug() {return true;}
        virtual bool no_ctlrs_left();
        virtual bool mac_belongs(const std::string& mac) const;
        virtual bool set_player_led(int index, bool on);
        virtual bool set_all_player_leds(bool on);
        virtual bool set_player_leds_to_player(int player);
};

#endif
