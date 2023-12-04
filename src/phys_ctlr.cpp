#include "phys_ctlr.h"

#include <fcntl.h>
#include <iostream>
#include <glob.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//private
std::optional<std::string> phys_ctlr::get_first_glob_path(std::string const &pattern)
{
    glob_t globbuf;

    glob(pattern.c_str(), 0, NULL, &globbuf);
    if (globbuf.gl_pathc) {
        std::string match(globbuf.gl_pathv[0]);
        globfree(&globbuf);
        return { match };
    }

    std::cout << "no match found for " << pattern << std::endl;
    globfree(&globbuf);
    return std::nullopt;
}

std::optional<std::string> phys_ctlr::get_led_path(std::string const &name)
{
    // Android links sysfs differently
#if defined(ANDROID) || defined(__ANDROID__)
    return get_first_glob_path(std::string("/sys/") + devpath + "/device/leds/*" + name);
#else
    return get_first_glob_path(std::string("/sys/") + devpath + "/device/device/leds/*" + name);
#endif
}

void phys_ctlr::init_leds()
{
    std::optional<std::string> tmp;
    for (unsigned int i = 0; i < 100; i++) {
        tmp = get_led_path("player*1");
        if (tmp.has_value()) {
            player_leds[0].open(tmp.value() + "/brightness");
            if (!player_leds[0].is_open()) {
                std::cerr << "Failed to open player1 led brightness\n";
                usleep(10);
                continue;
            }
            player_led_triggers[0].open(tmp.value() + "/trigger");
            if (!player_led_triggers[0].is_open()) {
                std::cerr << "Failed to open player1 trigger\n";
                usleep(10);
                continue;
            }
            break;
        }
        usleep(10);
    }

    for (unsigned int i = 0; i < 100; i++) {
        tmp = get_led_path("player*2");
        if (tmp.has_value()) {
            player_leds[1].open(tmp.value() + "/brightness");
            if (!player_leds[1].is_open()) {
                std::cerr << "Failed to open player2 led brightness\n";
                usleep(10);
                continue;
            }
            player_led_triggers[1].open(tmp.value() + "/trigger");
            if (!player_led_triggers[1].is_open()) {
                std::cerr << "Failed to open player2 trigger\n";
                usleep(10);
                continue;
            }
            break;
        }
        usleep(10);
    }

    for (unsigned int i = 0; i < 100; i++) {
        tmp = get_led_path("player*3");
        if (tmp.has_value()) {
            player_leds[2].open(tmp.value() + "/brightness");
            if (!player_leds[2].is_open()) {
                std::cerr << "Failed to open player3 led brightness\n";
                usleep(10);
                continue;
            }
            player_led_triggers[2].open(tmp.value() + "/trigger");
            if (!player_led_triggers[2].is_open()) {
                std::cerr << "Failed to open player3 trigger\n";
                usleep(10);
                continue;
            }
            break;
        }
        usleep(10);
    }

    for (unsigned int i = 0; i < 100; i++) {
        tmp = get_led_path("player*4");
        if (tmp.has_value()) {
            player_leds[3].open(tmp.value() + "/brightness");
            if (!player_leds[3].is_open()) {
                std::cerr << "Failed to open player4 led brightness\n";
                usleep(10);
                continue;
            }
            player_led_triggers[3].open(tmp.value() + "/trigger");
            if (!player_led_triggers[3].is_open()) {
                std::cerr << "Failed to open player4 trigger\n";
                usleep(10);
                continue;
            }
            break;
        }
        usleep(10);
    }

    if (model != Model::Left_Joycon) {
        for (unsigned int i = 0; i < 100; i++) {
            tmp = get_led_path("player*5");
            if (!tmp.has_value()) {
                tmp = get_led_path("home");
            }
            if (tmp.has_value()) {
                home_led.open(tmp.value() + "/brightness");
                if (!home_led.is_open()) {
                    std::cerr << "Failed to open home led brightness\n";
                    usleep(10);
                    continue;
                }
                break;
            }
            usleep(10);
        }
    }

}

void phys_ctlr::handle_event(struct input_event const &ev)
{
    int type = ev.type;
    int code = ev.code;
    int val = ev.value;

    if (type != EV_KEY)
        return;

    switch (model) {
        case Model::Procon:
        case Model::Snescon:
        case Model::N64:
            switch (code) {
                case BTN_TL:
                    l = val;
                    break;
                case BTN_TL2:
                    zl = val;
                    break;
                case BTN_TR:
                    r = val;
                    break;
                case BTN_TR2:
                    zr = val;
                    break;
                case BTN_START:
                    plus = val;
                    break;
                case BTN_SELECT:
                    minus = val;
                    break;
                default:
                    break;
            }
            break;
        case Model::Left_Joycon:
            switch (code) {
                case BTN_TL:
                    l = val;
                    break;
                case BTN_TL2:
                    zl = val;
                    break;
                case BTN_TR:
                    sl = val;
                    break;
                case BTN_TR2:
                    sr = val;
                    break;
                default:
                    break;
            }
            break;
        case Model::Right_Joycon:
            switch (code) {
                case BTN_TL:
                    sl = val;
                    break;
                case BTN_TL2:
                    sr = val;
                    break;
                case BTN_TR:
                    r = val;
                    break;
                case BTN_TR2:
                    zr = val;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

//public
phys_ctlr::phys_ctlr(std::string const &devpath, std::string const &devname) :
    devpath(devpath),
    devname(devname),
    evdev(nullptr),
    is_serial(false)
{

    zero_triggers();

    int fd = open(devname.c_str(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open " << devname << " ; errno=" << errno << std::endl;
        exit(1);
    }
    if (libevdev_new_from_fd(fd, &evdev)) {
        std::cerr << "Failed to create evdev from fd\n";
        exit(1);
    }

    int product_id = libevdev_get_id_product(evdev);
    // Extra checks are required for charging grip
    if (product_id == 0x200e) {
        std::cout << "Found Charging Grip Joy-Con...\n";
        if (libevdev_has_event_code(evdev, EV_KEY, BTN_TL))
            product_id = 0x2006;
        else
            product_id = 0x2007;
    }

    switch (product_id) {
        case 0x2009:
            model = Model::Procon;
            std::cout << "Found Pro Controller\n";
            break;
        case 0x2006:
            model = Model::Left_Joycon;
            std::cout << "Found Left Joy-Con\n";
            break;
        case 0x2007:
            model = Model::Right_Joycon;
            std::cout << "Found Right Joy-Con\n";
            break;
        case 0x2017:
            model = Model::Snescon;
            std::cout << "Found SNES Controller\n";
            break;
        case 0x2019:
            model = Model::N64;
            std::cout << "Found N64 Controller\n";
            break;
        default:
            model = Model::Unknown;
            std::cerr << "Unknown product id = " << std::hex << libevdev_get_id_product(evdev) << std::endl;
            break;
    }

    init_leds();

    // Prevent other users from having access to the evdev until it's paired
    grab();
    if (fchmod(get_fd(), S_IRUSR | S_IWUSR))
        std::cerr << "Failed to change evdev permissions; " << strerror(errno) << std::endl;

    // Check if this is a serial joy-con
#if defined(ANDROID) || defined(__ANDROID__)
    std::ifstream fname("/sys/" + devpath + "/name");
#else
    std::ifstream fname("/sys/" + devpath + "/../name");
#endif
    std::string driver_name;
    std::getline(fname, driver_name);
    std::cout << "driver_name: " << driver_name << std::endl;
    if (driver_name.find("Serial") != std::string::npos) {
        std::cout << "Serial joy-con detected\n";
        // Turn off player LEDs by default with serial joycons by default
        set_all_player_leds(false);
        is_serial = true;
    }

    // Attempt to read MAC address from uniq attribute
    mac_addr = "";
#if defined(ANDROID) || defined(__ANDROID__)
    std::ifstream funiq("/sys/" + devpath + "/uniq");
#else
     std::ifstream funiq("/sys/" + devpath + "/../uniq");
#endif
    std::getline(funiq, mac_addr);
    std::cout << "MAC: " << mac_addr << std::endl;
}

phys_ctlr::~phys_ctlr()
{
    if (evdev) {
        int fd = libevdev_get_fd(evdev);
        libevdev_free(evdev);
        close(fd);
    }
}

bool phys_ctlr::set_player_led(int index, bool on)
{
    if (index > 3 || !player_leds[index].is_open() || is_serial)
        return false;

    player_leds[index] << (on ? '1' : '0');
    player_leds[index].flush();
    return true;
}

bool phys_ctlr::set_all_player_leds(bool on)
{
    for (int i = 0; i < 4; i++) {
        if (!set_player_led(i, on))
            return false;
    }
    return true;
}

bool phys_ctlr::set_player_leds_to_player(int player)
{
    if (player < 1 || player > 4) {
        std::cerr << player << " is not a valid player led value\n";
        return false;
    }

    set_all_player_leds(false);
    for (int i = 0; i < player; i++) {
        set_player_led(i, true);
    }
    return true;
}

bool phys_ctlr::set_home_led(unsigned short brightness)
{
    if (brightness > 15 || !home_led.is_open())
        return false;

    home_led << brightness;
    home_led.flush();
    return true;
}

bool phys_ctlr::blink_player_leds()
{
    /* start with all player leds off */
    set_all_player_leds(false);

    for (int i = 0; i < 4; i++) {
        try {
            player_led_triggers[i] << "timer";
            player_led_triggers[i].flush();
        } catch (std::exception& e) {
            std::cerr << "Failed to select LED timer trigger. Is ledtrig-timer module probed?\n";
            return false;
        }
    }
    return true;
}

int phys_ctlr::get_fd()
{
    return libevdev_get_fd(evdev);
}

void phys_ctlr::handle_events()
{
    struct input_event ev;

    int ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    while (ret == LIBEVDEV_READ_STATUS_SYNC || ret == LIBEVDEV_READ_STATUS_SUCCESS) {
        if (ret == LIBEVDEV_READ_STATUS_SYNC) {
            std::cout << "handle sync\n";
            while (ret == LIBEVDEV_READ_STATUS_SYNC) {
                handle_event(ev);
                ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_SYNC, &ev);
            }
        } else if (ret == LIBEVDEV_READ_STATUS_SUCCESS) {
            handle_event(ev);
        }
        ret = libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
    }
}

enum phys_ctlr::PairingState phys_ctlr::get_pairing_state() const
{
    enum phys_ctlr::PairingState state = PairingState::Pairing;

    // leds don't function on android joycons so no way of the user knowing the pair state
#if defined(ANDROID) || defined(__ANDROID__)
    if (model != Model::Procon && model != Model::Snescon && model != Model::N64)
        return PairingState::Waiting;
    else
        return PairingState::Lone;
#endif

    if (libevdev_get_id_product(evdev) == 0x200e)
        return PairingState::Waiting;

    // uart joy-cons should just always be willing to pair
    if (is_serial)
        return PairingState::Waiting;

    switch (model) {
        case Model::Procon:
        case Model::Snescon:
        case Model::N64:
            if ((l | zl) && (r | zr))
                state = PairingState::Lone;
            else if (plus && minus)
                state = PairingState::Virt_Procon;
            break;
        case Model::Left_Joycon:
            if (l ^ zl)
                state = PairingState::Waiting;
            else if (sl && sr)
                state = PairingState::Horizontal;
            else if (l && zl)
                state = PairingState::Lone;
            break;
        case Model::Right_Joycon:
            if (r ^ zr)
                state = PairingState::Waiting;
            else if (sl && sr)
                state = PairingState::Horizontal;
            else if (r && zr)
                state = PairingState::Lone;
            break;
        default:
            break;
    }
    return state;
}

void phys_ctlr::zero_triggers()
{
    l = zl = r = zr = sl = sr = plus = minus = 0;
}
