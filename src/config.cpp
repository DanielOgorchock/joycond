#include "config.h"
#include "moonlight/file.h"
#include "moonlight/shlex.h"

static const Config _config;
static const ControllerProps _default_props;

// ctor
Config::Config(const std::string& filename) {
    try {
        auto infile = moonlight::file::open_r(filename);
        process_config_file(infile);

    } catch (const moonlight::core::RuntimeError& e) {
        std::cout << "Failed to open config file " << filename << ", skipping." << std::endl;
    }
}

// public static
const Config& Config::get() {
    return _config;
}

// public
std::optional<phys_ctlr::Model> Config::get_mac_device_override(const std::string& mac_addr) const {
    auto iter = _mac_model_overrides.find(mac_addr);
    if (iter == _mac_model_overrides.end()) {
        return {};
    }
    return iter->second;
}

const ControllerProps& Config::get_combined_joycons_props(const CombinedMACs& macs) const {
    auto iter = _combined_joycons_props.find(macs);
    if (iter == _combined_joycons_props.end()) {
        return _default_props;
    }

    return iter->second;
}

void Config::set_combined_joycons_props(const CombinedMACs& macs, const ControllerProps& props) {
    _combined_joycons_props.insert({macs, props});
}

// private
void Config::process_config_file(std::istream& infile) {
    std::string line;
    while (std::getline(infile, line)) {
        process_config_line(line);
    }
}

// private
void Config::process_config_line(const std::string& line) {
    auto tokens = moonlight::shlex::split(line);
    if (tokens.size() < 1) {
        return;
    }

    if (tokens[0].starts_with("#")) {
        return;
    }

    if (tokens[0] == "mac_device") {
        if (tokens.size() != 3) {
            std::cout << "Malformed `mac_device` directive in config file." << std::endl;
            return;
        }

        auto mac_address = tokens[1];
        auto device_name = tokens[2];

        auto iter = phys_ctlr::models_by_name().find(device_name);
        if (iter == phys_ctlr::models_by_name().end()) {
            std::cout << "Invalid device name in `mac_device` directive: " << device_name << std::endl;
            return;
        }

        _mac_model_overrides[mac_address] = iter->second;


    } else if (tokens[0] == "prop") {
        if (tokens.size() < 5) {
            std::cout << "Malformed `prop` directive in config file." << std::endl;
            return;
        }

        auto controller_type = tokens[1];

        if (controller_type == "combined_joycons") {
            if (tokens.size() < 6) {
                std::cout << "Malformed `prop` directive for `combined_joycons` controller in config file." << std::endl;
                return;
            }

            auto left_mac = tokens[2];
            auto right_mac = tokens[3];
            auto name = tokens[4];
            auto value = tokens[5];

            std::cout << "Setting controller prop " << name << " to " << value << " for combined joycons " << left_mac << " and " << right_mac << std::endl;

            auto props = get_combined_joycons_props({left_mac, right_mac});
            parse_and_set_controller_prop(props, name, value);
            set_combined_joycons_props({left_mac, right_mac}, props);

        } else {
            std::cout << "Unsupported controller type for `prop` directive: " << controller_type << std::endl;
            return;
        }

    } else {
        std::cout << "Unknown directive in config file: " << tokens[0] << std::endl;
    }
}

// private
void Config::parse_and_set_controller_prop(ControllerProps& props, const std::string& name, const std::string& value) const {
    if (name == "xbox_orientation") {
        try {
            props.xbox_orientation = std::stoi(value);

        } catch (...) {
            std::cout << "Failed to parse value for `xbox_orientation`." << std::endl;
            return;
        }
    } else {
        std::cout << "Unsupported property: " << name << std::endl;
    }
}
