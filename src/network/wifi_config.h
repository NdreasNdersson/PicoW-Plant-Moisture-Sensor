#ifndef __NETWORK__WIFI_CONFIG__
#define __NETWORK__WIFI_CONFIG__

#include <string>

using wifi_config_t = struct wifi_config {
    std::string ssid;
    std::string password;
};

#endif
