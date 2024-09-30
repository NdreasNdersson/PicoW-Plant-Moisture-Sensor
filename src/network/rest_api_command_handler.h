#ifndef PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_
#define PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_

#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "hal/pico_interface.h"
#include "nlohmann/json.hpp"
#include "patterns/subscriber.h"
#include "semphr.h"
#include "sensors/sensor.h"
#include "software_download.h"
#include "utils/config_handler.h"

class RestApiCommandHandler : public Subscriber<Measurement_t> {
   public:
    explicit RestApiCommandHandler(
        PicoInterface &pico_interface, SoftwareDownload &software_download,
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
    ConfigHandler config_handler_;
    PicoInterface &pico_interface_;
    SoftwareDownload &software_download_;
    std::vector<std::shared_ptr<Sensor>> sensors_;
    nlohmann::json rest_api_data_;
    SemaphoreHandle_t semaphore_;
};

#endif  // PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_
