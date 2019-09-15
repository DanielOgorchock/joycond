#include "virt_ctlr_combined.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libevdev/libevdev-uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

//private
void virt_ctlr_combined::relay_events(struct phys_ctlr *phys)
{
    struct input_event ev;
    struct libevdev *evdev = phys->get_evdev();

    int ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    while (ret == LIBEVDEV_READ_STATUS_SYNC || ret == LIBEVDEV_READ_STATUS_SUCCESS) {
        if (ret == LIBEVDEV_READ_STATUS_SYNC) {
            std::cout << "handle sync\n";
            while (ret == LIBEVDEV_READ_STATUS_SYNC) {
                libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
                ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_SYNC, &ev);
            }
        } else if (ret == LIBEVDEV_READ_STATUS_SUCCESS) {
            /* First filter out the SL and SR buttons on each physical controller */
            if (phys == physl && ev.type == EV_KEY && (ev.code == BTN_TR || ev.code == BTN_TR2)) {
                ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
                continue;
            } else if (phys == physr && ev.type == EV_KEY && (ev.code == BTN_TL || ev.code == BTN_TL2)) {
                ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
                continue;
            }
            libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
        }
        ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    }
}

//public
virt_ctlr_combined::virt_ctlr_combined(phys_ctlr *physl, phys_ctlr *physr) :
    physl(physl),
    physr(physr)
{
    int ret;

    uifd = open("/dev/uinput", O_RDWR);
    if (uidev < 0) {
        std::cerr << "Failed to open uinput; errno=" << errno << std::endl;
        exit(1);
    }

    char *tmp = strdup(libevdev_get_name(physl->get_evdev()));
    libevdev_set_name(physl->get_evdev(), "Nintendo Switch Combined Joy-Cons");
    ret = libevdev_uinput_create_from_device(physl->get_evdev(), uifd, &uidev);
    libevdev_set_name(physl->get_evdev(), tmp);
    free(tmp);
    if (ret) {
        std::cerr << "Failed to create libevdev_uinput; " << ret << std::endl;
        exit(1);
    }
}

virt_ctlr_combined::~virt_ctlr_combined()
{
    libevdev_uinput_destroy(uidev);
    close(uifd);
}

void virt_ctlr_combined::handle_events()
{
    relay_events(physl);
    relay_events(physr);
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
