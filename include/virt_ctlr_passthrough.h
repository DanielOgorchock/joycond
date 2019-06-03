#ifndef JOYCOND_VIRT_CTLR_PASSTHROUGH
#define JOYCOND_VIRT_CTLR_PASSTHROUGH

#include "virt_ctlr.h"
#include "phys_ctlr.h"

class virt_ctlr_passthrough : public virt_ctlr
{
    private:
        phys_ctlr *phys;

    public:
        virt_ctlr_passthrough(phys_ctlr *phys);
        virtual ~virt_ctlr_passthrough();

        virtual void handle_events();
        virtual bool contains_phys_ctlr(phys_ctlr const *ctlr) const;
        virtual bool contains_phys_ctlr(char const *devpath) const;
        virtual std::vector<phys_ctlr *> get_phys_ctlrs();
};

#endif
