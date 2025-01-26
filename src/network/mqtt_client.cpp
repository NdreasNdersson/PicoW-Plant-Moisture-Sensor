#include "mqtt_client.h"

#include <cstring>
#include <string>

#include "lwip/err.h"
#include "nlohmann/json.hpp"
#include "utils/config_handler/configs/mqtt_config.h"
#include "utils/logging.h"

static void mqtt_pub_request_cb(void *arg, err_t result) {
    if (result != ERR_OK) {
        LogError(("Publish result: %d\n", result));
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg,
                               mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        LogInfo(("Successfully connected"));
    } else {
        LogError(("Disconnected, reason: %d", status));
    }
}

MqttClient::MqttClient(const mqtt_config_t &config,
                       const std::vector<std::shared_ptr<Sensor>> &sensors)
    : config_{config}, mqtt_client_{mqtt_client_new()}, sensors_{sensors} {
    for (const auto &sensor : sensors_) {
        sensor->attach(this);
    }
}

MqttClient::~MqttClient() {
    for (const auto &sensor : sensors_) {
        sensor->detach(this);
    }
    disconnect();
}

// Subscriber
void MqttClient::update(const Measurement_t &measurement) {
    if (mqtt_client_ == nullptr) {
        return;
    }

    auto topic{config_.client_name + "/" + measurement.name};
    nlohmann::json json{{"value", measurement.value},
                        {"raw_value", measurement.raw_value}};
    const u8_t qos = 2;
    const u8_t retain = 0;

    mqtt_publish(mqtt_client_, topic.c_str(), json.dump().c_str(),
                 json.dump().size(), qos, retain, mqtt_pub_request_cb, nullptr);
}

bool MqttClient::connect() {
    if (mqtt_client_ == nullptr) {
        return false;
    }

    const struct mqtt_connect_client_info_t client_info {
        config_.client_name.c_str(), config_.user.c_str(),
            config_.password.c_str(), 0, nullptr, nullptr, 0, 0
    };

    ip_addr_t ip_addr;
    ipaddr_aton(config_.ip.c_str(), &ip_addr);

    auto err{mqtt_client_connect(mqtt_client_, &ip_addr, config_.port,
                                 mqtt_connection_cb, nullptr, &client_info)};

    return err == ERR_OK;
}

void MqttClient::disconnect() {
    if (mqtt_client_ != nullptr) {
        mqtt_disconnect(mqtt_client_);
    }
}
