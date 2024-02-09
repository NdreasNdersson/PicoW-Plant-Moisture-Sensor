#ifndef __NETWORK__REST_API__
#define __NETWORK__REST_API__

extern "C" {
#include "tcp_server.h"
}

#include <array>
#include <memory>
#include <string>

#include "device_info.h"

#define N_DEVICES 1

class RestApi {
   public:
    RestApi();
    ~RestApi();

    bool start();
    bool register_device(const std::string &device_name,
                         const std::string &init_value);
    bool set_data(const std::string &device_name, const std::string &new_value);

   private:
    std::string m_ip_address;
    int m_port;
    std::unique_ptr<TCP_SERVER_T> m_server_state;
    std::array<DeviceInfo, N_DEVICES> m_devices;

    void update();
};

#endif
