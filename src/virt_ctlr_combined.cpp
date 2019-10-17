#include "virt_ctlr_combined.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <libevdev/libevdev-uinput.h>
#include <linux/uinput.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

//private
void virt_ctlr_combined::relay_events(std::shared_ptr<phys_ctlr> phys)
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

void virt_ctlr_combined::handle_uinput_event()
{
    struct input_event ev;
    int ret;

    while ((ret = read(get_uinput_fd(), &ev, sizeof(ev))) == sizeof(ev)) {
        switch (ev.type) {
            case EV_FF:
                switch (ev.code) {
                    default:
                        /* Just forward this FF event on to the actual devices */
                        if (write(physl->get_fd(), &ev, sizeof(ev)) != sizeof(ev))
                            std::cerr << "Failed to forward EV_FF to physl\n";
                        if (write(physr->get_fd(), &ev, sizeof(ev)) != sizeof(ev))
                            std::cerr << "Failed to forward EV_FF to physr\n";
                        break;
                }
                break;

            case EV_UINPUT:
                switch (ev.code) {
                    case UI_FF_UPLOAD:
                        {
                            struct uinput_ff_upload upload = { 0 };
                            struct ff_effect effect = { 0 };

                            upload.request_id = ev.value;
                            if (ioctl(get_uinput_fd(), UI_BEGIN_FF_UPLOAD, &upload))
                                std::cerr << "Failed to get uinput_ff_upload: " << strerror(errno) << std::endl;

                            effect = upload.effect;
                            effect.id = -1;
                            /* upload the effect to both devices */
                            upload.retval = 0;
                            if (ioctl(physl->get_fd(), EVIOCSFF, &effect) == -1)
                                upload.retval = errno;

                            /* reset effect */
                            effect = upload.effect;
                            effect.id = -1;
                            if (ioctl(physr->get_fd(), EVIOCSFF, &effect) == -1)
                                upload.retval = errno;

                            if (upload.retval)
                                std::cerr << "UI_FF_UPLOAD failed: " << strerror(upload.retval) << std::endl;

                            if (ioctl(get_uinput_fd(), UI_END_FF_UPLOAD, &upload))
                                std::cerr << "Failed to end uinput_ff_upload: " << strerror(errno) << std::endl;
                            break;
                        }
                    case UI_FF_ERASE:
                        {
                            struct uinput_ff_erase erase = { 0 };

                            erase.request_id = ev.value;
                            if (ioctl(get_uinput_fd(), UI_BEGIN_FF_ERASE, &erase))
                                std::cerr << "Failed to get uinput_ff_erase: " << strerror(errno) << std::endl;

                            erase.retval = 0;
                            if (ioctl(physl->get_fd(), EVIOCRMFF, erase.effect_id) == -1)
                                erase.retval = errno;
                            if (ioctl(physr->get_fd(), EVIOCRMFF, erase.effect_id) == -1)
                                erase.retval = errno;

                            if (erase.retval)
                                std::cerr << "UI_FF_ERASE failed: " << strerror(erase.retval) << std::endl;

                            if (ioctl(get_uinput_fd(), UI_END_FF_ERASE, &erase))
                                std::cerr << "Failed to end uinput_ff_erase: " << strerror(errno) << std::endl;
                            break;
                        }
                    default:
                        std::cerr << "Unhandled EV_UNINPUT code=" << ev.code <<std::endl;
                        break;
                }
                break;

            default:
                std::cerr << "Unhandled uinput type=" << ev.type << std::endl;
                break;
        }
    }
    if (ret < 0 && errno != EAGAIN) {
        std::cerr << "Failed reading uinput fd; ret=" << strerror(errno) << std::endl;
    } else if (ret > 0) {
        std::cerr << "Uinput incorrect read size of " << ret << std::endl;
    }
}

//public
virt_ctlr_combined::virt_ctlr_combined(std::shared_ptr<phys_ctlr> physl, std::shared_ptr<phys_ctlr> physr, epoll_mgr& epoll_manager) :
    physl(physl),
    physr(physr),
    epoll_manager(epoll_manager)
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

    int flags = fcntl(get_uinput_fd(), F_GETFL, 0);
    fcntl(get_uinput_fd(), F_SETFL, flags | O_NONBLOCK);

    subscriber = std::make_shared<epoll_subscriber>(std::vector({get_uinput_fd()}),
                                                    [=](int event_fd){handle_events(event_fd);});
    epoll_manager.add_subscriber(subscriber);
}

virt_ctlr_combined::~virt_ctlr_combined()
{
    struct epoll_event ctlr_event;

    epoll_manager.remove_subscriber(subscriber);

    libevdev_uinput_destroy(uidev);
    close(uifd);
}

void virt_ctlr_combined::handle_events(int fd)
{
    if (fd == physl->get_fd())
        relay_events(physl);
    else if (fd == physr->get_fd())
        relay_events(physr);
    else if (fd == get_uinput_fd())
        handle_uinput_event();
    else
        std::cerr << "fd=" << fd << " is an invalid fd for this combined controller\n";
}

bool virt_ctlr_combined::contains_phys_ctlr(std::shared_ptr<phys_ctlr> const ctlr) const
{
    return physl == ctlr || physr == ctlr;
}

bool virt_ctlr_combined::contains_phys_ctlr(char const *devpath) const
{
    return physl->get_devpath() == devpath || physr->get_devpath() == devpath;
}

bool virt_ctlr_combined::contains_fd(int fd) const
{
    return physl->get_fd() == fd || physr->get_fd() == fd || libevdev_uinput_get_fd(uidev) == fd;
}

std::vector<std::shared_ptr<phys_ctlr>> virt_ctlr_combined::get_phys_ctlrs()
{
    std::vector<std::shared_ptr<phys_ctlr>> ctlrs = { physl, physr };
    return ctlrs;
}

int virt_ctlr_combined::get_uinput_fd()
{
    return libevdev_uinput_get_fd(uidev);
}
