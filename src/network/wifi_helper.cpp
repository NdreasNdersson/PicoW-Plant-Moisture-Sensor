#include "wifi_helper.h"

#include "FreeRTOS.h"
#include "pico/cyw43_arch.h"
#include "pico/util/datetime.h"
#include "task.h"
#include "utils/logging.h"

WifiHelper::WifiHelper() {
    // NOP
}

WifiHelper::~WifiHelper() {
    // NOP
}

/***
 * Initialise the controller
 * @return true if successful
 */
auto WifiHelper::init() -> bool {
    // Init Wifi Adapter
    int res = cyw43_arch_init();
    if (res) {
        return false;
    }

    cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM);

    return true;
}

auto WifiHelper::join(const wifi_config_t &config, uint8_t retries) -> bool {
    cyw43_arch_enable_sta_mode();
    LogInfo(("Connecting to WiFi... %s", config.ssid.c_str()));

    // Loop trying to connect to Wifi
    int r = -1;
    uint8_t attempts = 0;
    while (r < 0) {
        attempts++;
        r = cyw43_arch_wifi_connect_timeout_ms(config.ssid.c_str(),
                                               config.password.c_str(),
                                               CYW43_AUTH_WPA2_AES_PSK, 60000);

        if (r) {
            if (attempts >= retries) {
                return false;
            }
            vTaskDelay(2000);
        }
    }
    return true;
}

/***
 * Get IP address of unit
 * @param ip - output uint8_t[4]
 */
auto WifiHelper::getIPAddress(uint8_t *ip) -> bool {
    memcpy(ip, netif_ip4_addr(&cyw43_state.netif[0]), 4);
    return true;
}

/***
 * Get IP address of unit
 * @param ips - output char * up to 16 chars
 * @return - true if IP addres assigned
 */
auto WifiHelper::getIPAddressStr(char *ips) -> bool {
    char *s = ipaddr_ntoa(netif_ip4_addr(&cyw43_state.netif[0]));
    strcpy(ips, s);
    return true;
}

/***
 * Get Gateway address
 * @param ip - output uint8_t[4]
 */
auto WifiHelper::getGWAddress(uint8_t *ip) -> bool {
    memcpy(ip, netif_ip4_gw(&cyw43_state.netif[0]), 4);
    return true;
}

/***
 * Get Gateway address
 * @param ips - output char * up to 16 chars
 * @return - true if IP addres assigned
 */
auto WifiHelper::getGWAddressStr(char *ips) -> bool {
    char *s = ipaddr_ntoa(netif_ip4_gw(&cyw43_state.netif[0]));
    strcpy(ips, s);
    return true;
}

/***
 * Get Net Mask address
 * @param ip - output uint8_t[4]
 */
auto WifiHelper::getNetMask(uint8_t *ip) -> bool {
    memcpy(ip, netif_ip4_netmask(&cyw43_state.netif[0]), 4);
    return true;
}

/***
 * Get Net Mask
 * @param ips - output char * up to 16 chars
 * @return - true if IP addres assigned
 */
auto WifiHelper::getNetMaskStr(char *ips) -> bool {
    char *s = ipaddr_ntoa(netif_ip4_netmask(&cyw43_state.netif[0]));
    strcpy(ips, s);
    return true;
}

/***
 * Get the mac address as a string
 * @param macStr: pointer to string of at least 14 characters
 * @return true if successful
 */
auto WifiHelper::getMACAddressStr(char *macStr) -> bool {
    uint8_t mac[6];
    int r = cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);

    if (r == 0) {
        for (uint8_t i = 0; i < 6; i++) {
            if (mac[i] < 16) {
                sprintf(&macStr[i * 2], "0%X", mac[i]);
            } else {
                sprintf(&macStr[i * 2], "%X", mac[i]);
            }
        }
        macStr[13] = 0;
        return true;
    }
    return false;
}

/***
 * Returns if joined to the network and we have a link
 * @return true if joined.
 */
auto WifiHelper::isJoined() -> bool {
    int res = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    return (res >= 0);
}
