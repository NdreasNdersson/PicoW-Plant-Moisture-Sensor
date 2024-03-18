#ifndef __NETWORK__REST_API__
#define __NETWORK__REST_API__

#include <array>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "device_info.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "semphr.h"

#define N_DEVICES (4 * 4)
#define MAX_BUF_SIZE 1024
#define MAX_PACKAGES 2
#define POLL_TIME_S 5

class RestApi {
   public:
    RestApi();
    ~RestApi();

    bool start();
    bool register_device(const std::string &device_name,
                         const std::string &init_value);
    bool set_data(const std::string &device_name, const std::string &new_value);
    bool get_data(std::string &data);

   private:
    typedef struct TCP_SERVER_T_ {
        struct tcp_pcb *server_pcb;
        struct tcp_pcb *client_pcb;
        uint8_t device_data[MAX_BUF_SIZE];
        uint8_t buffer_recv[MAX_BUF_SIZE];
        bool data_received;
        int device_data_len;
        int buffer_recv_len;
        SemaphoreHandle_t buffer_mutex;
    } TCP_SERVER_T;

    std::string m_ip_address;
    int m_port;
    std::array<DeviceInfo, N_DEVICES> m_devices;
    std::unique_ptr<TCP_SERVER_T> m_server_state;

    void update();
    static err_t tcp_client_close(void *arg);
    static err_t tcp_server_close(void *arg);
    static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
    static err_t tcp_server_send(void *arg, struct tcp_pcb *tpcb,
                                 std::string data);
    static err_t tcp_server_send_measured_data(void *arg, struct tcp_pcb *tpcb);
    static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb,
                                 struct pbuf *p, err_t err);
    static void tcp_server_err(void *arg, err_t err);
    static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb,
                                   err_t err);
};

#endif
