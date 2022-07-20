#include "virt_ctlr_pro.h"

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
void virt_ctlr_pro::relay_events(std::shared_ptr<phys_ctlr> phys)
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
#if defined(ANDROID) || defined(__ANDROID__)
            /* remap the ZL and ZR buttons to analog trigger on android */
            if (ev.type == EV_KEY && ev.code == BTN_TL2) {
                libevdev_uinput_write_event(uidev, EV_ABS, ABS_Z, ev.value);
                ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
                continue;
            } else if (ev.type == EV_KEY && ev.code == BTN_TR2) {
                libevdev_uinput_write_event(uidev, EV_ABS, ABS_RZ, ev.value);
                ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
                continue;
            }
#endif
            libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
        }
        ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    }
}

void virt_ctlr_pro::handle_uinput_event()
{
    struct input_event ev;
    int ret;

    while ((ret = read(get_uinput_fd(), &ev, sizeof(ev))) == sizeof(ev)) {
        switch (ev.type) {
            case EV_FF:
                {
                    struct input_event redirected = ev;

                    if (!rumble_effects.count(ev.code)) {
                        if (ev.code < FF_GAIN)
                            std::cerr << "ERROR: ff_effect with id=" << ev.code << " is not in map\n";
                    } else {
                        redirected.code = rumble_effects[ev.code].id;
                    }

                    /* Just forward this FF event on to the actual devices */
                    if (write(phys->get_fd(), &redirected, sizeof(redirected)) != sizeof(redirected))
                        std::cerr << "Failed to forward EV_FF to phys\n";
                    break;
                }

            case EV_UINPUT:
                switch (ev.code) {
                    case UI_FF_UPLOAD:
                        {
                            struct uinput_ff_upload upload = { 0 };
                            struct ff_effect effect = { 0 };
                            bool allocate_new_effect;

                            upload.request_id = ev.value;
                            if (ioctl(get_uinput_fd(), UI_BEGIN_FF_UPLOAD, &upload))
                                std::cerr << "Failed to get uinput_ff_upload: " << strerror(errno) << std::endl;

                            effect = upload.effect;
                            /* Check if already allocated or it needs an update */
                            allocate_new_effect = !rumble_effects.count(effect.id);
                            if (allocate_new_effect)
                                effect.id = -1;

                            /* upload the effect to the real device */
                            upload.retval = 0;
                            if (ioctl(phys->get_fd(), EVIOCSFF, &effect) == -1)
                                upload.retval = errno;

                            upload.effect = effect;

                            if (upload.retval)
                                std::cerr << "UI_FF_UPLOAD failed: " << strerror(upload.retval) << std::endl;

                            if (allocate_new_effect)
                                rumble_effects[effect.id] = effect;

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
                            if (ioctl(phys->get_fd(), EVIOCRMFF, erase.effect_id) == -1)
                                erase.retval = errno;

                            if (erase.retval)
                                std::cerr << "UI_FF_ERASE failed: " << strerror(erase.retval) << std::endl;
                            else if (!rumble_effects.count(erase.effect_id))
                                std::cerr << "WARNING: effect_id not in rumble_effects map\n";
                            else
                                rumble_effects.erase(erase.effect_id);

                            if (ioctl(get_uinput_fd(), UI_END_FF_ERASE, &erase))
                                std::cerr << "Failed to end uinput_ff_erase: " << strerror(errno) << std::endl;

                            break;
                        }
                    default:
                        std::cerr << "Unhandled EV_UNINPUT code=" << ev.code <<std::endl;
                        break;
                }
                break;

            case EV_LED:
                if (ev.value == 0) {
                    libevdev_uinput_write_event(uidev, EV_LED, ev.code, !ev.value);
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
virt_ctlr_pro::virt_ctlr_pro(std::shared_ptr<phys_ctlr> phys, epoll_mgr& epoll_manager) :
    phys(phys),
    epoll_manager(epoll_manager),
    subscriber(nullptr),
    virt_evdev(nullptr),
    uidev(nullptr),
    uifd(-1),
    rumble_effects(),
    mac(phys->get_mac_addr())
{
    int ret;

    uifd = open("/dev/uinput", O_RDWR);
    if (uifd < 0) {
        std::cerr << "Failed to open uinput; errno=" << errno << std::endl;
        exit(1);
    }

    // Create a virtual evdev on which the uinput will be based
    virt_evdev = libevdev_new();
    if (!virt_evdev) {
        std::cerr << "Failed to create virtual evdev\n";
        exit(1);
    }

    libevdev_set_name(virt_evdev, "Nintendo Switch Virtual Pro Controller");

    // Make sure that all of this configuration remains in sync with the hid-nintendo driver.
    libevdev_enable_event_type(virt_evdev, EV_KEY);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_SELECT, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_Z, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_THUMBL, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_START, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_MODE, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_THUMBR, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_SOUTH, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_EAST, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_NORTH, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_WEST, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_TL, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_TR, NULL);
#if !defined(ANDROID) && !defined(__ANDROID__)
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_TL2, NULL);
    libevdev_enable_event_code(virt_evdev, EV_KEY, BTN_TR2, NULL);
#endif

    struct input_absinfo absconfig = { 0 };
    absconfig.minimum = -32767;
    absconfig.maximum = 32767;
    absconfig.fuzz = 250;
    absconfig.flat = 500;
    libevdev_enable_event_type(virt_evdev, EV_ABS);
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_X, &absconfig);
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_Y, &absconfig);
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_RX, &absconfig);
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_RY, &absconfig);

    // HAT for dpad
    struct input_absinfo dpad_absconfig = { 0 };
    dpad_absconfig.minimum = -1;
    dpad_absconfig.maximum = 1;
    dpad_absconfig.fuzz = 0;
    dpad_absconfig.flat = 0;
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_HAT0X, &dpad_absconfig);
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_HAT0Y, &dpad_absconfig);

    // Emulate analog triggers for android
#if defined(ANDROID) || defined(__ANDROID__)
    struct input_absinfo absconfig_fake = { 0 };
    absconfig_fake.minimum = 0;
    absconfig_fake.maximum = 1;
    absconfig_fake.fuzz = 0;
    absconfig_fake.flat = 0;

    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_Z, &absconfig_fake);
    libevdev_enable_event_code(virt_evdev, EV_ABS, ABS_RZ, &absconfig_fake);
#endif
    libevdev_enable_event_type(virt_evdev, EV_FF);
    libevdev_enable_event_code(virt_evdev, EV_FF, FF_RUMBLE, NULL);
    libevdev_enable_event_code(virt_evdev, EV_FF, FF_PERIODIC, NULL);
    libevdev_enable_event_code(virt_evdev, EV_FF, FF_SQUARE, NULL);
    libevdev_enable_event_code(virt_evdev, EV_FF, FF_TRIANGLE, NULL);
    libevdev_enable_event_code(virt_evdev, EV_FF, FF_SINE, NULL);
    libevdev_enable_event_code(virt_evdev, EV_FF, FF_GAIN, NULL);

    // Set the product information to a non-existent product info (but with virtual bus type)
    libevdev_set_id_vendor(virt_evdev, 0x57e);
    libevdev_set_id_product(virt_evdev, 0x2008); // HACK?
    libevdev_set_id_bustype(virt_evdev, BUS_VIRTUAL);
    libevdev_set_id_version(virt_evdev, 0x0000);

    // Enable LED events
    libevdev_enable_event_type(virt_evdev, EV_LED);
    libevdev_enable_event_code(virt_evdev, EV_LED, 0, NULL);
    libevdev_enable_event_code(virt_evdev, EV_LED, 1, NULL);
    libevdev_enable_event_code(virt_evdev, EV_LED, 2, NULL);
    libevdev_enable_event_code(virt_evdev, EV_LED, 3, NULL);

    ret = libevdev_uinput_create_from_device(virt_evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
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

virt_ctlr_pro::~virt_ctlr_pro()
{
    epoll_manager.remove_subscriber(subscriber);

    libevdev_uinput_destroy(uidev);
    close(uifd);
    libevdev_free(virt_evdev);
}

void virt_ctlr_pro::handle_events(int fd)
{
    if (fd == phys->get_fd())
        relay_events(phys);
    else if (fd == get_uinput_fd())
        handle_uinput_event();
    else
        std::cerr << "fd=" << fd << " is an invalid fd for this virtual pro controller\n";
}

bool virt_ctlr_pro::contains_phys_ctlr(std::shared_ptr<phys_ctlr> const ctlr) const
{
    return phys == ctlr;
}

bool virt_ctlr_pro::contains_phys_ctlr(char const *devpath) const
{
    return phys->get_devpath() == devpath;
}

bool virt_ctlr_pro::contains_fd(int fd) const
{
    return phys->get_fd() == fd || libevdev_uinput_get_fd(uidev) == fd;
}

std::vector<std::shared_ptr<phys_ctlr>> virt_ctlr_pro::get_phys_ctlrs()
{
    std::vector<std::shared_ptr<phys_ctlr>> ctlrs;
    ctlrs.push_back(phys);
    return ctlrs;
}

int virt_ctlr_pro::get_uinput_fd()
{
    return libevdev_uinput_get_fd(uidev);
}

void virt_ctlr_pro::remove_phys_ctlr(const std::shared_ptr<phys_ctlr> phys)
{
    std::cerr << "Don't support removing controllers to virtual procon\n";
    exit(EXIT_FAILURE);
}

void virt_ctlr_pro::add_phys_ctlr(std::shared_ptr<phys_ctlr> phys)
{
    std::cerr << "Don't support re-adding controllers to virtual procon\n";
    exit(EXIT_FAILURE);
}

enum phys_ctlr::Model virt_ctlr_pro::needs_model()
{
    enum phys_ctlr::Model model = phys_ctlr::Model::Unknown;
    return model;
}

bool virt_ctlr_pro::set_player_led(int index, bool on)
{
    if (index > 3)
        return false;

    libevdev_uinput_write_event(uidev, EV_LED, index, on);
    return true;
}

bool virt_ctlr_pro::set_all_player_leds(bool on)
{
    for (int i = 0; i < 4; i++) {
        if (!set_player_led(i, on))
            return false;
    }
    return true;
}

bool virt_ctlr_pro::set_player_leds_to_player(int player)
{
    if (player < 1 || player > 4) {
        std::cerr << player << " is not a valid player led value\n";
        return false;
    }

    for (int i = 0; i < player; i++) {
        set_player_led(i, true);
    }
    return true;
}
