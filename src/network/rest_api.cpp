#include "rest_api.h"

#include <cstring>

#include "FreeRTOS.h"
#include "nlohmann/json.hpp"
#include "semphr.h"
#include "utils/logging.h"

static const std::string HTTP_OK_RESPONSE{"HTTP/1.0 200 OK\r\n"};

static const std::string HTTP_BAD_RESPONSE{"HTTP/1.0 400 NOK\r\n"};

static const std::string HTTP_CONTENT_TYPE{
    "Content-type: application/json\r\n\r\n"};

RestApi::RestApi(std::function<void(bool)> led_control)
    : m_ip_address{"0.0.0.0"},
      m_port{80},
      m_server_state{std::make_unique<TCP_SERVER_T>()} {
    m_server_state->buffer_mutex = xSemaphoreCreateMutex();
    m_server_state->led_control = led_control;
}

RestApi::~RestApi() {}

bool RestApi::start() {
    if (m_server_state == NULL) {
        return false;
    }

    LogInfo(("Starting server at %s on port %u\n",
             ip4addr_ntoa(netif_ip4_addr(netif_list)), m_port));

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        LogError(("failed to create pcb\n"));
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, m_port);
    if (err) {
        LogError(("failed to bind to port %u\n", m_port));
        return false;
    }

    m_server_state->server_pcb = tcp_listen_with_backlog(pcb, 2);
    if (!m_server_state->server_pcb) {
        LogError(("failed to listen\n"));
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(m_server_state->server_pcb, m_server_state.get());
    tcp_accept(m_server_state->server_pcb, tcp_server_accept);

    return true;
}

bool RestApi::set_data(const nlohmann::json &data) {
    bool status{false};
    if (xSemaphoreTake(m_server_state->buffer_mutex,
                       static_cast<TickType_t>(100)) == pdTRUE) {
        const auto data_string{data["sensors"].dump()};
        const auto json_data_str{data_string.c_str()};

        std::memcpy(m_server_state->device_data, json_data_str,
                    strlen(json_data_str));
        m_server_state->device_data_len = strlen(json_data_str);
        xSemaphoreGive(m_server_state->buffer_mutex);
        status = true;
    }
    return status;
}

bool RestApi::get_data(std::string &data) {
    auto status{false};
    if (xSemaphoreTake(m_server_state->buffer_mutex,
                       static_cast<TickType_t>(100)) == pdTRUE) {
        if (m_server_state->data_received) {
            data = std::string(
                reinterpret_cast<char const *>(m_server_state->buffer_recv),
                m_server_state->buffer_recv_len);
            status = true;
            m_server_state->data_received = false;
        }
        xSemaphoreGive(m_server_state->buffer_mutex);
    }

    return status;
}

err_t RestApi::tcp_client_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            LogError(("close failed %d, calling abort", err));
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }

    LogDebug(("Client close"));
    state->led_control(false);
    return err;
}

err_t RestApi::tcp_server_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            LogError(("close failed %d, calling abort", err));
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }

    LogDebug(("Server close"));
    state->led_control(false);
    return err;
}

err_t RestApi::tcp_server_send(void *arg, struct tcp_pcb *tpcb,
                               std::string data) {
    cyw43_arch_lwip_check();
    err_t err = tcp_write(tpcb, data.c_str(), strlen(data.c_str()),
                          TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        LogError(("Failed to write data %d", err));
        return tcp_client_close(arg);
    } else {
        LogDebug(("Sent %u bytes", strlen(data.c_str())));
    }
    return ERR_OK;
}

err_t RestApi::tcp_server_send_measured_data(void *arg, struct tcp_pcb *tpcb) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;

    cyw43_arch_lwip_check();
    if (xSemaphoreTake(state->buffer_mutex, (TickType_t)100) == pdTRUE) {
        err_t err = tcp_write(tpcb, state->device_data, state->device_data_len,
                              TCP_WRITE_FLAG_COPY);
        xSemaphoreGive(state->buffer_mutex);
        if (err != ERR_OK) {
            LogError(("Failed to write data %d", err));
            return tcp_client_close(arg);
        }
    }
    return ERR_OK;
}

err_t RestApi::tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                               err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    if (!p) {
        LogDebug(("tcp_server_recv pointer empty"));
        return tcp_client_close(arg);
    }
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        std::string payload(reinterpret_cast<char const *>(p->payload), p->len);

        if (payload.rfind("GET", 0) == 0) {
            LogDebug(("Received GET"));

            if (tcp_server_send(arg, tpcb,
                                HTTP_OK_RESPONSE + HTTP_CONTENT_TYPE) ==
                ERR_OK) {
                tcp_server_send_measured_data(arg, state->client_pcb);
            }
        } else if (payload.rfind("POST", 0) == 0) {
            LogDebug(("Received POST"));

            std::size_t start_idx = payload.find("{");
            if (start_idx != std::string::npos) {
                auto json_data{payload.substr(start_idx)};
                LogDebug(("Received Json: %s", json_data.c_str()));
                if (xSemaphoreTake(state->buffer_mutex, (TickType_t)100) ==
                    pdTRUE) {
                    std::memset(state->buffer_recv, 0, MAX_BUF_SIZE);
                    std::memcpy(state->buffer_recv, json_data.c_str(),
                                strlen(json_data.c_str()));
                    state->buffer_recv_len = strlen(json_data.c_str());
                    state->data_received = true;
                    xSemaphoreGive(state->buffer_mutex);
                }
                tcp_server_send(arg, tpcb, HTTP_OK_RESPONSE);
            } else {
                tcp_server_send(arg, tpcb, HTTP_BAD_RESPONSE);
            }
        } else {
            LogDebug(("Command not implemented. Received:\n", payload.c_str()));
            tcp_server_send(arg, tpcb, HTTP_BAD_RESPONSE);
        }
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    return tcp_client_close(arg);
}

void RestApi::tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        LogError(("tcp_client_err_fn %d", err));
        tcp_server_close(arg);
    }
}

err_t RestApi::tcp_server_accept(void *arg, struct tcp_pcb *client_pcb,
                                 err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        LogError(("Failure in accept"));
        tcp_client_close(arg);
        return ERR_VAL;
    }

    LogDebug(("Client connected"));
    state->led_control(true);

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}
