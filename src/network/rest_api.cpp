#include "rest_api.h"

#include <algorithm>

#include "FreeRTOS.h"
#include "nlohmann/json.hpp"
#include "semphr.h"
#include "utils/logging.h"
using json = nlohmann::json;

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

    std::string header_str{
        "HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n"};
    std::string body_str{"{}"};
    for (int i = 0; i < header_str.size(); i++) {
        m_server_state->buffer_sent[0][i] = header_str[i];
    }
    for (int i = 0; i < body_str.size(); i++) {
        m_server_state->buffer_sent[1][i] = body_str[i];
    }
    m_server_state->buffer_send_len[0] = header_str.size();
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
    /* char *dest = reinterpret_cast<char *>(m_server_state->buffer_sent[1]); */

    json json_data;
    for (auto &device : m_devices) {
        if (device.is_registered()) {
            json_data[device.get_name()] = device.get_value();
        }
    }

    if (xSemaphoreTake(m_server_state->buffer_mutex,
                       static_cast<TickType_t>(100)) == pdTRUE) {
        const char *json_data_str{json_data.dump().c_str()};

        memcpy(m_server_state->buffer_sent[1], json_data_str,
               strlen(json_data_str));
        m_server_state->buffer_send_len[1] = strlen(json_data_str);
        xSemaphoreGive(m_server_state->buffer_mutex);
    }
}
