#ifndef JOYCOND_PHYS_CTLR_H
#define JOYCOND_PHYS_CTLR_H

#include <fstream>
#include <libevdev/libevdev.h>
#include <optional>
#include <string>

class phys_ctlr
{
    public:
        enum class Model { Procon, Snescon, Left_Joycon, Right_Joycon, N64, Unknown };
        enum class PairingState { Pairing, Lone, Waiting, Horizontal, Virt_Procon };

    private:
        std::string devpath;
        std::string devname;
        struct libevdev *evdev;
        bool is_serial;
        std::fstream player_leds[4];
        std::fstream player_led_triggers[4];
        std::fstream home_led;
        bool l, zl, r, zr, sl, sr, plus, minus;
        enum Model model;
        std::string mac_addr;

        std::optional<std::string> get_first_glob_path(std::string const &pattern);
        std::optional<std::string> get_led_path(std::string const &name);
        void init_leds();
        void handle_event(struct input_event const &ev);

    public:
        phys_ctlr(std::string const &devpath, std::string const &devname);
        ~phys_ctlr();

        std::string const &get_devpath() const { return devpath; }
        bool set_player_led(int index, bool on);
        bool set_all_player_leds(bool on);
        bool set_player_leds_to_player(int player);
        bool set_home_led(unsigned short brightness);
        bool blink_player_leds();
        int get_fd();
        void handle_events();
        enum Model get_model() const { return model; }
        enum PairingState get_pairing_state() const;
        void grab() { libevdev_grab(evdev, LIBEVDEV_GRAB); }
        void ungrab() { libevdev_grab(evdev, LIBEVDEV_UNGRAB); }
        struct libevdev *get_evdev() { return evdev; }
        void zero_triggers();
        const std::string& get_mac_addr() { return mac_addr; }
        bool is_serial_ctlr() const { return is_serial; }
};

#endif
