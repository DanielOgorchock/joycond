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
};

#endif
