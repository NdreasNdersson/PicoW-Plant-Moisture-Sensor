#ifndef __NETWORK__REST_API__
#define __NETWORK__REST_API__

#include <functional>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "nlohmann/json.hpp"
#include "pico/cyw43_arch.h"
#include "semphr.h"

enum {
    N_DEVICES = (4 * 4),
    MAX_BUF_SIZE = 1024,
    MAX_PACKAGES = 2,
    POLL_TIME_S = 5
};

class RestApi {
   public:
    RestApi(std::function<void(bool)> led_control);
    ~RestApi();

    auto start() -> bool;
    auto set_device_data(const nlohmann::json &data) -> bool;
    auto set_device_config(const nlohmann::json &data) -> bool;
    auto get_data(std::string &data) -> bool;

   private:
    using TCP_SERVER_T = struct TCP_SERVER_T_ {
        struct tcp_pcb *server_pcb;
        struct tcp_pcb *client_pcb;
        uint8_t device_data[MAX_BUF_SIZE];
        uint8_t device_config[MAX_BUF_SIZE];
        uint8_t buffer_recv[MAX_BUF_SIZE];
        bool data_received;
        int device_data_len;
        int device_config_len;
        int buffer_recv_len;
        SemaphoreHandle_t buffer_mutex;
        std::function<void(bool)> led_control;
    };

    std::string m_ip_address;
    int m_port{80};
    std::unique_ptr<TCP_SERVER_T> m_server_state;

    void update();
    static err_t tcp_client_close(void *arg);
    static err_t tcp_server_close(void *arg);
    static err_t tcp_server_send(void *arg, struct tcp_pcb *tpcb,
                                 std::string data);
    static err_t tcp_server_send_measured_data(void *arg, struct tcp_pcb *tpcb);
    static err_t tcp_server_send_config_data(void *arg, struct tcp_pcb *tpcb);
    static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb,
                                 struct pbuf *p, err_t err);
    static void tcp_server_err(void *arg, err_t err);
    static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb,
                                   err_t err);
};

#endif
