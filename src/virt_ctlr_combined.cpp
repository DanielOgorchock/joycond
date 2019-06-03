#include "virt_ctlr_combined.h"

#include <vector>

//private

//public
virt_ctlr_combined::virt_ctlr_combined(phys_ctlr *physl, phys_ctlr *physr) :
    physl(physl),
    physr(physr)
{
}

virt_ctlr_combined::~virt_ctlr_combined()
{
}

void virt_ctlr_combined::handle_events()
{
    physl->handle_events();
    physl->handle_events();
}

bool virt_ctlr_combined::contains_phys_ctlr(phys_ctlr const *ctlr) const
{
    return physl == ctlr || physr == ctlr;
}

bool virt_ctlr_combined::contains_phys_ctlr(char const *devpath) const
{
    return physl->get_devpath() == devpath || physr->get_devpath() == devpath;
}

bool virt_ctlr_combined::contains_fd(int fd) const
{
    return physl->get_fd() == fd || physr->get_fd() == fd;
}

std::vector<phys_ctlr *> virt_ctlr_combined::get_phys_ctlrs()
{
    std::vector<phys_ctlr *> ctlrs = { physl, physr };
    return ctlrs;
}
