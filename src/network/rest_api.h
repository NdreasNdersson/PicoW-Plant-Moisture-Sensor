#ifndef PICO_REST_SENSOR_NETWORK_REST_API_H_
#define PICO_REST_SENSOR_NETWORK_REST_API_H_

#include <functional>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "network/rest_api_command_handler.h"
#include "nlohmann/json.hpp"
#include "pico/cyw43_arch.h"
#include "sensors/ads1115_adc.h"

class RestApi {
   public:
    RestApi(std::function<void(bool)> led_control,
            std::vector<Ads1115Adc> &sensors);
    ~RestApi();

    auto start() -> bool;

   private:
    using TCP_SERVER_T = struct TCP_SERVER_T_ {
        struct tcp_pcb *server_pcb;
        struct tcp_pcb *client_pcb;
        std::function<void(bool)> led_control;
        std::function<bool(const std::string &resource, std::string &payload)>
            get_callback;
        std::function<bool(const std::string &resource,
                           const std::string &payload)>
            post_callback;
    };

    std::unique_ptr<RestApiCommandHandler> m_rest_api_command_handler;
    std::string m_ip_address;
    int m_port;
    std::unique_ptr<TCP_SERVER_T> m_server_state;

    void update();
    static auto tcp_client_close(void *arg) -> err_t;
    static auto tcp_server_close(void *arg) -> err_t;
    static auto tcp_server_send(void *arg, struct tcp_pcb *tpcb,
                                const std::string &data) -> err_t;
    static auto tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                                err_t err) -> err_t;
    static void tcp_server_err(void *arg, err_t err);
    static auto tcp_server_accept(void *arg, struct tcp_pcb *client_pcb,
                                  err_t err) -> err_t;
};

#endif  // PICO_REST_SENSOR_NETWORK_REST_API_H_
