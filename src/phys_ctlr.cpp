#include "phys_ctlr.h"

#include <iostream>
#include <glob.h>

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
    return get_first_glob_path(std::string("/sys/") + devpath + "/device/device/leds/*" + name);
}

void phys_ctlr::init_leds()
{
    std::cout << "init_leds\n";
    auto tmp = get_led_path("player1");
    if (tmp.has_value()) {
        player_leds[0].open(tmp.value() + "/brightness");
        if (!player_leds[0].is_open()) {
            std::cerr << "Failed to open player1 led brightness\n";
        }
    }

    tmp = get_led_path("player2");
    if (tmp.has_value()) {
        player_leds[1].open(tmp.value() + "/brightness");
        if (!player_leds[1].is_open()) {
            std::cerr << "Failed to open player2 led brightness\n";
        }
    }

    tmp = get_led_path("player3");
    if (tmp.has_value()) {
        player_leds[2].open(tmp.value() + "/brightness");
        if (!player_leds[2].is_open()) {
            std::cerr << "Failed to open player3 led brightness\n";
        }
    }

    tmp = get_led_path("player4");
    if (tmp.has_value()) {
        player_leds[3].open(tmp.value() + "/brightness");
        if (!player_leds[3].is_open()) {
            std::cerr << "Failed to open player4 led brightness\n";
        }
    }

    tmp = get_led_path("home");
    if (tmp.has_value()) {
        home_led.open(tmp.value() + "/brightness");
        if (!home_led.is_open()) {
            std::cerr << "Failed to open home led brightness\n";
        }
    }

}

//public
phys_ctlr::phys_ctlr(std::string const &devpath) :
    devpath(devpath)

{
    init_leds();
}

phys_ctlr::~phys_ctlr()
{
}

bool phys_ctlr::set_player_led(int index, bool on)
{
    if (index > 3 || !player_leds[index].is_open())
        return false;

    player_leds[index] << on ? '1' : '0';
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

bool phys_ctlr::set_home_led(unsigned short brightness)
{
    if (brightness > 15 || !home_led.is_open())
        return false;

    home_led << brightness;
    home_led.flush();
    return true;
}

