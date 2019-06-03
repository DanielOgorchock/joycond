#ifndef JOYCOND_VIRT_CTLR_COMBINED
#define JOYCOND_VIRT_CTLR_COMBINED

#include "virt_ctlr.h"
#include "phys_ctlr.h"

class virt_ctlr_combined : public virt_ctlr
{
    private:
        phys_ctlr *physl;
        phys_ctlr *physr;

    public:
        virt_ctlr_combined(phys_ctlr *physl, phys_ctlr *physr);
        virtual ~virt_ctlr_combined();

        virtual void handle_events();
        virtual bool contains_phys_ctlr(phys_ctlr const *ctlr) const;
        virtual bool contains_phys_ctlr(char const *devpath) const;
        virtual bool contains_fd(int fd) const;
        virtual std::vector<phys_ctlr *> get_phys_ctlrs();
};

#endif
