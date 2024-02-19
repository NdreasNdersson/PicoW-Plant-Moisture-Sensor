#ifndef __NETWORK__TCP_SERVER__
#define __NETWORK__TCP_SERVER__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "semphr.h"

#define DEBUG_printf printf
#define MAX_BUF_SIZE 128
#define MAX_PACKAGES 2
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    uint8_t buffer_sent[MAX_PACKAGES][MAX_BUF_SIZE];
    uint8_t buffer_recv[MAX_PACKAGES][MAX_BUF_SIZE];
    int buffer_send_len[MAX_PACKAGES];
    int packages_send_len;
    int sent_len;
    int recv_len;
    int run_count;
    SemaphoreHandle_t buffer_mutex;
} TCP_SERVER_T;

err_t tcp_server_close(void *arg);

err_t tcp_server_result(void *arg, int status);

err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

err_t tcp_server_send_data(void *arg, struct tcp_pcb *tpcb);

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                      err_t err);

void tcp_server_err(void *arg, err_t err);

err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);

#ifdef __cplusplus
}
#endif

#endif
