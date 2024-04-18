#ifndef __NETWORK__WIFI_HELPER__
#define __NETWORK__WIFI_HELPER__

#include <stdlib.h>

#include <string>

#include "pico/stdlib.h"
#include "wifi_config.h"

#ifndef WIFI_RETRIES
#define WIFI_RETRIES 3
#endif

class WifiHelper {
   public:
    WifiHelper();

    virtual ~WifiHelper();

    /***
     * Initialise the controller
     * @return true if successful
     */
    static auto init() -> bool;

    /***
     * Get IP address of unit
     * @param ip - output uint8_t[4]
     * @return - true if IP addres assigned
     */
    static auto getIPAddress(uint8_t *ip) -> bool;

    /***
     * Get IP address of unit
     * @param ips - output char * up to 16 chars
     * @return - true if IP addres assigned
     */
    static auto getIPAddressStr(char *ips) -> bool;

    /***
     * Get Gateway address
     * @param ip - output uint8_t[4]
     */
    static auto getGWAddress(uint8_t *ip) -> bool;

    /***
     * Get Gateway address
     * @param ips - output char * up to 16 chars
     * @return - true if IP addres assigned
     */
    static auto getGWAddressStr(char *ips) -> bool;

    /***
     * Get Net Mask address
     * @param ip - output uint8_t[4]
     */
    static auto getNetMask(uint8_t *ip) -> bool;

    /***
     * Get Net Mask
     * @param ips - output char * up to 16 chars
     * @return - true if IP addres assigned
     */
    static auto getNetMaskStr(char *ips) -> bool;

    /***
     * Get the mac address as a string
     * @param macStr: pointer to string of at least 14 characters
     * @return true if successful
     */
    static auto getMACAddressStr(char *macStr) -> bool;

    /***
     *  Join a Wifi Network
     * @param sid - string of the SID
     * @param password - Password for network
     * @param retries - Number of times to retry, defalts to 3.
     * @return true if successful
     */
    static auto join(const wifi_config_t &config,
                     uint8_t retries = WIFI_RETRIES) -> bool;

    /***
     * Returns if joined to the network and we have a link
     * @return true if joined.
     */
    static auto isJoined() -> bool;

   private:
};

#endif
