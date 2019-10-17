#ifndef JOYCOND_VIRT_CTLR_PASSTHROUGH
#define JOYCOND_VIRT_CTLR_PASSTHROUGH

#include "virt_ctlr.h"
#include "phys_ctlr.h"

#include <memory>

class virt_ctlr_passthrough : public virt_ctlr
{
    private:
        std::shared_ptr<phys_ctlr> phys;

    public:
        virt_ctlr_passthrough(std::shared_ptr<phys_ctlr> phys);
        virtual ~virt_ctlr_passthrough();

        virtual void handle_events(int fd);
        virtual bool contains_phys_ctlr(std::shared_ptr<phys_ctlr> const ctlr) const;
        virtual bool contains_phys_ctlr(char const *devpath) const;
        virtual bool contains_fd(int fd) const;
        virtual std::vector<std::shared_ptr<phys_ctlr>> get_phys_ctlrs();
};

#endif
