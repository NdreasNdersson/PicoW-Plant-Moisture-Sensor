#ifndef PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_
#define PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_

#include <memory>
#include <string>

#include "hal/freertos_interface.h"
#include "hal/pico_interface.h"
#include "nlohmann/json.hpp"
#include "patterns/subscriber.h"
#include "semphr.h"
#include "sensors/sensor.h"
#include "software_download.h"
#include "software_download_api.h"
#include "utils/config_handler/config_handler.h"

class RestApiCommandHandler : public Subscriber<Measurement_t> {
   public:
    RestApiCommandHandler(
        ConfigHandler &config_handler,
        PicoBootloader::SoftwareDownloadApi &software_download,
        FreertosInterface &freertos_interface,
        std::vector<std::shared_ptr<Sensor>> sensors);
    ~RestApiCommandHandler();
    RestApiCommandHandler(const RestApiCommandHandler &) = default;
    RestApiCommandHandler(RestApiCommandHandler &&) = default;
    RestApiCommandHandler &operator=(const RestApiCommandHandler &) = default;
    RestApiCommandHandler &operator=(RestApiCommandHandler &&) = default;

    auto get_callback(const std::string &resource, std::string &payload)
        -> bool;
    auto post_callback(const std::string &resource, const std::string &payload)
        -> bool;

    // Subscriber
    void update(const Measurement_t &) override;

   private:
    auto validate_sensors_content(const nlohmann::json &json_data) -> bool;
    auto validate_mqtt_content(const nlohmann::json &json_data) -> bool;
    auto validate_swdl_content(const nlohmann::json &json_data) -> bool;
    ConfigHandler &config_handler_;
    std::shared_ptr<PicoInterface> pico_interface_;
    PicoBootloader::SoftwareDownloadApi &software_download_;
    FreertosInterface &freertos_interface_;
    std::vector<std::shared_ptr<Sensor>> sensors_;
    nlohmann::json rest_api_data_;
    SemaphoreHandle_t semaphore_;
};

#endif  // PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_
