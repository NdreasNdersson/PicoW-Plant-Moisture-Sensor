#ifndef __NETWORK__WIFI_CONFIG__
#define __NETWORK__WIFI_CONFIG__

#include <string>

typedef struct {
    std::string ssid;
    std::string password;
} wifi_config_t;

#endif
