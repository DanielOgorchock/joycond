#ifndef JOYCOND_CONFIG_H
#define JOYCOND_CONFIG_H

#include "phys_ctlr.h"
#include <map>

class Config {
public:
    Config(const std::string& filename = "/etc/joycond.conf");
    ~Config() { }

    static const Config& get();

    std::optional<phys_ctlr::Model> get_mac_device_override(const std::string& mac_addr) const;

private:
    void process_config_file(std::istream& infile);
    void process_config_line(const std::string& line);

    std::map<std::string, phys_ctlr::Model> _mac_model_overrides;
};

#endif /* !JOYCOND_CONFIG_H */
