#ifndef JOYCOND_CONFIG_H
#define JOYCOND_CONFIG_H

#include "phys_ctlr.h"
#include <map>

typedef std::pair<std::string, std::string> CombinedMACs;

struct ControllerProps {
    bool xbox_orientation = false;
};

class Config {
public:
    Config(const std::string& filename = "/etc/joycond.conf");
    ~Config() { }

    static const Config& get();

    std::optional<phys_ctlr::Model> get_mac_device_override(const std::string& mac_addr) const;
    const ControllerProps& get_combined_joycons_props(const CombinedMACs& macs) const;
    void set_combined_joycons_props(const CombinedMACs& macs, const ControllerProps& props);

private:
    void process_config_file(std::istream& infile);
    void process_config_line(const std::string& line);
    void parse_and_set_controller_prop(ControllerProps& props, const std::string& name, const std::string& value) const;

    std::map<std::string, phys_ctlr::Model> _mac_model_overrides;
    std::map<CombinedMACs, ControllerProps> _combined_joycons_props;
};

#endif /* !JOYCOND_CONFIG_H */
