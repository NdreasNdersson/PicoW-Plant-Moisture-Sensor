#include "rest_api.h"

#include <algorithm>
#include <cstring>

#include "FreeRTOS.h"
#include "nlohmann/json.hpp"
#include "semphr.h"
#include "utils/logging.h"
using json = nlohmann::json;

static const std::string HTTP_OK_RESPONSE{"HTTP/1.0 200 OK\r\n"};

static const std::string HTTP_CONTENT_TYPE{
    "Content-type: application/json\r\n\r\n"};

RestApi::RestApi()
    : m_ip_address{"0.0.0.0"},
      m_port{80},
      m_server_state{std::make_unique<TCP_SERVER_T>()},
      m_devices{} {
    m_server_state->buffer_mutex = xSemaphoreCreateMutex();
}

RestApi::~RestApi() {}

bool RestApi::start() {
    if (m_server_state == NULL) {
        return false;
    }

    std::string body_str{"{}"};
    for (int i = 0; i < body_str.size(); i++) {
        m_server_state->buffer_sent[1][i] = body_str[i];
    }
    m_server_state->buffer_send_len[1] = body_str.size();
    m_server_state->packages_send_len = 2;

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

bool RestApi::register_device(const std::string &device_name,
                              const std::string &init_value) {
    for (auto &device : m_devices) {
        if (device.register_device(device_name, init_value)) {
            return true;
        }
    }
    return false;
}

bool RestApi::set_data(const std::string &device_name,
                       const std::string &new_value) {
    auto predicate{[device_name](DeviceInfo &device) {
        return device.get_name() == device_name;
    }};
    auto device_it =
        std::find_if(std::begin(m_devices), std::end(m_devices), predicate);
    if (device_it == m_devices.end()) {
        return false;
    }
    device_it->set_value(new_value);

    update();

    return true;
}

void RestApi::update() {
    json json_data;
    for (auto &device : m_devices) {
        if (device.is_registered()) {
            json_data[device.get_name()] = device.get_value();
        }
    }

    if (xSemaphoreTake(m_server_state->buffer_mutex,
                       static_cast<TickType_t>(100)) == pdTRUE) {
        const char *json_data_str{json_data.dump().c_str()};

        std::memcpy(m_server_state->buffer_sent[1], json_data_str,
                    strlen(json_data_str));
        m_server_state->buffer_send_len[1] = strlen(json_data_str);
        xSemaphoreGive(m_server_state->buffer_mutex);
    }
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
    return err;
}

err_t RestApi::tcp_server_result(void *arg, int status) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    LogInfo(("Close TCP server, status %d", status));
    return tcp_server_close(arg);
}

err_t RestApi::tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    LogDebug(("tcp_server_sent %u", len));
    state->sent_len += len;

    if (state->sent_len >= MAX_BUF_SIZE) {
        // We should get the data back from the client
        state->recv_len = 0;
        LogDebug(("Waiting for buffer from client"));
    }

    return ERR_OK;
}

err_t RestApi::tcp_server_send(void *arg, struct tcp_pcb *tpcb,
                               std::string data) {
    cyw43_arch_lwip_check();
    err_t err = tcp_write(tpcb, data.c_str(), strlen(data.c_str()),
                          TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        LogError(("Failed to write data %d", err));
        return tcp_client_close(arg);
    }
    return ERR_OK;
}

err_t RestApi::tcp_server_send_measured_data(void *arg, struct tcp_pcb *tpcb) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;

    cyw43_arch_lwip_check();
    if (xSemaphoreTake(state->buffer_mutex, (TickType_t)100) == pdTRUE) {
        err_t err = tcp_write(tpcb, state->buffer_sent[1],
                              state->buffer_send_len[1], TCP_WRITE_FLAG_COPY);
        xSemaphoreGive(state->buffer_mutex);
        if (err != ERR_OK) {
            LogError(("Failed to write data %d", err));
            /* return tcp_server_result(arg, -1); */
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
        LogDebug(
            ("tcp_server_recv %d/%d err %d", p->tot_len, state->recv_len, err));

        // Receive the buffer
        const uint16_t buffer_left = MAX_BUF_SIZE - state->recv_len;
        state->recv_len += pbuf_copy_partial(
            p, state->buffer_recv + state->recv_len,
            p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    LogDebug(("tcp_server_recv buffer ok"));
    std::string temp(reinterpret_cast<char const *>(state->buffer_recv),
                     state->recv_len);

    LogDebug(("Received:\n%s", state->buffer_recv));
    std::memset(state->buffer_recv, 0, MAX_BUF_SIZE);
    state->recv_len = 0;

    if (temp.rfind("GET", 0) == 0) {
        LogDebug(("Received GET"));
        if (tcp_server_send(arg, tpcb, HTTP_OK_RESPONSE + HTTP_CONTENT_TYPE) ==
            ERR_OK) {
            tcp_server_send_measured_data(arg, state->client_pcb);
        }
        return tcp_client_close(arg);
    } else if (temp.rfind("POST", 0) == 0) {
        LogDebug(("Received POST:\n%s", temp));
        std::size_t start_idx = temp.find("{");
        /* std::size_t end_idx = temp.find_last_of("}"); */
        if (start_idx != std::string::npos) {
            LogDebug(("Json:%s", temp.substr(start_idx)));
        }
        if (tcp_server_send(arg, tpcb, HTTP_OK_RESPONSE) == ERR_OK) {
            LogDebug(("Header sent"));
        }
        return tcp_client_close(arg);
    } else {
        LogDebug(("Command not implemented. Received:\n", temp));
        return tcp_client_close(arg);
    }
    return err;
}

void RestApi::tcp_server_err(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        LogError(("tcp_client_err_fn %d", err));
        tcp_server_result(arg, err);
    }
}

err_t RestApi::tcp_server_accept(void *arg, struct tcp_pcb *client_pcb,
                                 err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T *)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        LogError(("Failure in accept"));
        tcp_server_result(arg, err);
        return ERR_VAL;
    }

    LogDebug(("Client connected"));

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);

    /* return tcp_server_send_data(arg, state->client_pcb); */
    return ERR_OK;
}
