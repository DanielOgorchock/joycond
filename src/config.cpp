#include "config.h"
#include "moonlight/file.h"
#include "moonlight/shlex.h"

static const Config _config;

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

    } else {
        std::cout << "Unknown directive in config file: " << tokens[0] << std::endl;
    }
}
