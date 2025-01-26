#ifndef PICO_REST_SENSOR_NETWORK_MQTT_CLIENT
#define PICO_REST_SENSOR_NETWORK_MQTT_CLIENT

#include <memory>
#include <vector>

#include "lwip/apps/mqtt.h"
#include "patterns/subscriber.h"
#include "sensors/sensor.h"
#include "utils/config_handler/configs/mqtt_config.h"

class MqttClient : public Subscriber<Measurement_t> {
   public:
    MqttClient(const mqtt_config_t &config,
               const std::vector<std::shared_ptr<Sensor>> &sensors);
    ~MqttClient();

    bool connect();
    void disconnect();

    // Subscriber
    void update(const Measurement_t &) override;

   private:
    mqtt_config_t config_;
    mqtt_client_t *mqtt_client_;
    std::vector<std::shared_ptr<Sensor>> sensors_;
};

#endif  // !PICO_REST_SENSOR_NETWORK_MQTT_CLIENT
