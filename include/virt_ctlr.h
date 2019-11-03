#ifndef JOYCOND_VIRT_CTLR_H
#define JOYCOND_VIRT_CTLR_H

#include "phys_ctlr.h"

#include <memory>
#include <vector>

class virt_ctlr
{
    private:

    public:
        virt_ctlr() {}
        virtual ~virt_ctlr() {}

        virtual void handle_events(int fd) = 0;
        virtual bool contains_phys_ctlr(std::shared_ptr<phys_ctlr> const ctlr) const = 0;
        virtual bool contains_phys_ctlr(char const *devpath) const = 0;
        virtual bool contains_fd(int fd) const = 0;
        virtual std::vector<std::shared_ptr<phys_ctlr>> get_phys_ctlrs() = 0;
        virtual void remove_phys_ctlr(const std::shared_ptr<phys_ctlr> phys) = 0;
        virtual void add_phys_ctlr(std::shared_ptr<phys_ctlr> phys) = 0;
        virtual enum phys_ctlr::Model needs_model() = 0;
        virtual bool supports_hotplug() {return false;}
        virtual bool mac_belongs(const std::string& mac) const {return false;}

        // Used to determine if this virtual controller should be removed from paired controllers list
        virtual bool no_ctlrs_left() {return true;}
};

#endif
