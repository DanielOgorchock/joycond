#include "virt_ctlr_passthrough.h"

#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <vector>

//private

//public
virt_ctlr_passthrough::virt_ctlr_passthrough(std::shared_ptr<phys_ctlr> phys) :
    phys(phys)
{
    // Allow other processes to use the input now.
    if (fchmod(phys->get_fd(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))
        std::cerr << "Failed to change evdev permissions; " << strerror(errno) << std::endl;
    // Inform other processes the input is usable now.
    std::ofstream uevent;
    uevent.open("/sys" + phys->get_devpath() + "/uevent");
    uevent << "change";
    uevent.close();
    phys->ungrab();
}

virt_ctlr_passthrough::~virt_ctlr_passthrough()
{
}

void virt_ctlr_passthrough::handle_events(int fd)
{
    phys->handle_events();
}

bool virt_ctlr_passthrough::contains_phys_ctlr(std::shared_ptr<phys_ctlr> const ctlr) const
{
    return phys == ctlr;
}

bool virt_ctlr_passthrough::contains_phys_ctlr(char const *devpath) const
{
    return phys->get_devpath() == devpath;
}

bool virt_ctlr_passthrough::contains_fd(int fd) const
{
    return phys->get_fd() == fd;
}

std::vector<std::shared_ptr<phys_ctlr>> virt_ctlr_passthrough::get_phys_ctlrs()
{
    std::vector<std::shared_ptr<phys_ctlr>> ctlrs = { phys };
    return ctlrs;
}

void virt_ctlr_passthrough::remove_phys_ctlr(const std::shared_ptr<phys_ctlr> phys)
{
    std::cerr << "ERROR: Cannot remove phys_ctlr from a passthrough controller\n";
    exit(EXIT_FAILURE);
}

void virt_ctlr_passthrough::add_phys_ctlr(std::shared_ptr<phys_ctlr> phys)
{
    std::cerr << "ERROR: Cannot add phys_ctlr to a passthrough controller\n";
    exit(EXIT_FAILURE);
}

enum phys_ctlr::Model virt_ctlr_passthrough::needs_model()
{
    return phys_ctlr::Model::Unknown;
}
