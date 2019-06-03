#include "virt_ctlr_passthrough.h"

#include <vector>

//private

//public
virt_ctlr_passthrough::virt_ctlr_passthrough(phys_ctlr *phys) :
    phys(phys)
{
    // Allow other processes to use the input now.
    phys->ungrab();
}

virt_ctlr_passthrough::~virt_ctlr_passthrough()
{
}

void virt_ctlr_passthrough::handle_events()
{
    // keep track of the triggers' states
    phys->handle_events();
}

bool virt_ctlr_passthrough::contains_phys_ctlr(phys_ctlr const *ctlr) const
{
    return phys == ctlr;
}

bool virt_ctlr_passthrough::contains_phys_ctlr(char const *devpath) const
{
    return phys->get_devpath() == devpath;
}

std::vector<phys_ctlr *> virt_ctlr_passthrough::get_phys_ctlrs()
{
    std::vector<phys_ctlr *> ctlrs = { phys };
    return ctlrs;
}
