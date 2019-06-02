#ifndef JOYCOND_PHYS_CTLR_H
#define JOYCOND_PHYS_CTLR_H

#include <fstream>
#include <libevdev/libevdev.h>
#include <optional>
#include <string>

class phys_ctlr
{
    private:
        std::string devpath;
        std::string devname;
        struct libevdev *evdev;
        std::fstream player_leds[4];
        std::fstream home_led;

        std::optional<std::string> get_first_glob_path(std::string const &pattern);
        std::optional<std::string> get_led_path(std::string const &name);
        void init_leds();

    public:
        phys_ctlr(std::string const &devpath, std::string const &devname);
        ~phys_ctlr();

        std::string const &get_devpath() const { return devpath; }
        bool set_player_led(int index, bool on);
        bool set_all_player_leds(bool on);
        bool set_home_led(unsigned short brightness);
};

#endif
