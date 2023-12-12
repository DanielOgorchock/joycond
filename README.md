joycond is a linux daemon which uses the evdev devices provided by hid-nintendo (formerly known as hid-joycon) to implement joycon pairing.

hid-nintendo is currently in review on the linux-input mailing list. The most recent patches are currently found at https://github.com/DanielOgorchock/linux

# Installation
1. clone the repo
2. Install requirements (`sudo apt install libevdev-dev libudev-dev` or `sudo dnf install libevdev-devel libudev-devel`)
3. `cmake .`
4. `sudo make install`
5. `sudo systemctl enable --now joycond`

# Usage
When a joy-con or pro controller is connected via bluetooth or USB, the player LEDs should start blinking periodically. This signals that the controller is in pairing mode.

For the pro controller, pressing both triggers will "pair" it.

For the pro controller, pressing Plus and Minus will pair it as a virtual controller.
This is useful when using Steam.

With the joy-cons, to use a single contoller alone, hold ZL and L at the same time (ZR and R for the right joy-con). Alternatively, hold both S triggers at once.

To combine two joy-cons into a virtual input device, press a *single* trigger on both of them at the same time. A new uinput device will be created called "Nintendo Switch Combined Joy-Cons".

Rumble support is now functional for the combined joy-con uinput device.
