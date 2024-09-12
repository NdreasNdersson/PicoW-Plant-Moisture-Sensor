#include "rest_api.h"

#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <utility>

#include "FreeRTOS.h"
#include "network/rest_api_command_handler.h"
#include "nlohmann/json.hpp"
#include "semphr.h"
#include "utils/logging.h"

static const std::string HTTP_OK_RESPONSE{"HTTP/1.0 200 OK\r\n"};
static const std::string HTTP_BAD_RESPONSE{"HTTP/1.0 400 NOK\r\n"};
static const std::string HTTP_CONTENT_TYPE{
    "Content-type: application/json\r\n\r\n"};

RestApi::RestApi(std::function<void(bool)> led_control,
                 const std::vector<std::shared_ptr<Sensor>> &sensors)
    : m_rest_api_command_handler{std::make_unique<RestApiCommandHandler>(
          sensors)},
      m_ip_address{"0.0.0.0"},
      m_port{80},

      m_server_state{std::make_unique<TCP_SERVER_T>()} {
    m_server_state->led_control = std::move(led_control);

    m_server_state->get_callback = [this](const std::string &resource,
                                          std::string &payload) -> bool {
        return m_rest_api_command_handler->get_callback(resource, payload);
    };
    m_server_state->post_callback = [this](const std::string &resource,
                                           const std::string &payload) -> bool {
        return m_rest_api_command_handler->post_callback(resource, payload);
    };
}

auto RestApi::start() -> bool {
    if (m_server_state == nullptr) {
        return false;
    }

    LogInfo(("Starting server at %s on port %u\n",
             ip4addr_ntoa(netif_ip4_addr(netif_list)), m_port));

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        LogError(("failed to create pcb\n"));
        return false;
    }

    err_t err = tcp_bind(pcb, nullptr, m_port);
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

auto RestApi::tcp_client_close(void *arg) -> err_t {
    auto *state = (TCP_SERVER_T *)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != nullptr) {
        tcp_arg(state->client_pcb, nullptr);
        tcp_poll(state->client_pcb, nullptr, 0);
        tcp_sent(state->client_pcb, nullptr);
        tcp_recv(state->client_pcb, nullptr);
        tcp_err(state->client_pcb, nullptr);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            LogError(("close failed %d, calling abort", err));
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = nullptr;
    }

    state->led_control(false);
    return err;
}

auto RestApi::tcp_server_close(void *arg) -> err_t {
    auto *state = (TCP_SERVER_T *)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != nullptr) {
        tcp_arg(state->client_pcb, nullptr);
        tcp_poll(state->client_pcb, nullptr, 0);
        tcp_sent(state->client_pcb, nullptr);
        tcp_recv(state->client_pcb, nullptr);
        tcp_err(state->client_pcb, nullptr);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            LogError(("close failed %d, calling abort", err));
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = nullptr;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, nullptr);
        tcp_close(state->server_pcb);
        state->server_pcb = nullptr;
    }

    LogDebug(("Server close"));
    state->led_control(false);
    return err;
}

auto RestApi::tcp_server_send(void *arg, struct tcp_pcb *tpcb,
                              const std::string &data) -> err_t {
    cyw43_arch_lwip_check();
    err_t err = tcp_write(tpcb, data.c_str(), data.size(), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        LogError(("Failed to write data %d", err));
        return tcp_client_close(arg);
    }
    return ERR_OK;
}

auto RestApi::tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                              err_t /*err*/) -> err_t {
    auto *state = (TCP_SERVER_T *)arg;
    if (!p) {
        LogError(("tcp_server_recv pointer empty"));
        return tcp_client_close(arg);
    }
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        std::string payload(reinterpret_cast<char const *>(p->payload), p->len);

        std::stringstream string_stream{payload};

        std::string command;
        std::string resource;
        getline(string_stream, command, ' ');
        getline(string_stream, resource, ' ');

        size_t pos = resource.find_first_not_of('/');
        resource = pos == std::string::npos ? resource : resource.substr(pos);

        pos = resource.find_last_not_of('/');
        resource =
            pos == std::string::npos ? resource : resource.substr(0, pos + 1);

        if (command == "GET") {
            std::string json_data;
            if (state->get_callback(resource, json_data)) {
                if (tcp_server_send(arg, tpcb,
                                    HTTP_OK_RESPONSE + HTTP_CONTENT_TYPE) ==
                    ERR_OK) {
                    err_t err =
                        tcp_write(tpcb, json_data.c_str(), json_data.size(),
                                  TCP_WRITE_FLAG_COPY);
                    if (err != ERR_OK) {
                        LogError(("Failed to write data %d", err));
                        return tcp_client_close(arg);
                    }
                }
            } else {
                tcp_server_send(arg, tpcb, HTTP_BAD_RESPONSE);
            }
        } else if (command == "POST") {
            std::size_t start_idx = payload.find('{');
            if (start_idx != std::string::npos) {
                auto json_data{payload.substr(start_idx - 1)};
                if (state->post_callback(resource, json_data)) {
                    tcp_server_send(arg, tpcb, HTTP_OK_RESPONSE);
                } else {
                    tcp_server_send(arg, tpcb, HTTP_BAD_RESPONSE);
                }
            } else {
                tcp_server_send(arg, tpcb, HTTP_BAD_RESPONSE);
            }
        } else {
            LogError(("Command not implemented"));
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

auto RestApi::tcp_server_accept(void *arg, struct tcp_pcb *client_pcb,
                                err_t err) -> err_t {
    auto *state = (TCP_SERVER_T *)arg;
    if (err != ERR_OK || client_pcb == nullptr) {
        LogError(("Failure in accept"));
        tcp_client_close(arg);
        return ERR_VAL;
    }

    state->led_control(true);

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}
