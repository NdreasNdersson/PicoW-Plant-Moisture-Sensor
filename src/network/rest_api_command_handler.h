#ifndef PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_
#define PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_

#include <memory>
#include <string>

#include "bootloader_lib.h"
#include "sensors/sensor.h"
#include "utils/config_handler.h"

class RestApiCommandHandler {
   public:
    explicit RestApiCommandHandler(
        std::vector<std::shared_ptr<Sensor>> sensors);
    auto get_callback(const std::string &resource, std::string &payload)
        -> bool;
    auto post_callback(const std::string &resource, const std::string &payload)
        -> bool;

   private:
    ConfigHandler m_config_handler;
    SoftwareDownload m_software_download;
    std::vector<std::shared_ptr<Sensor>> m_sensors;
};

#endif  // PICO_REST_SENSOR_NETWORK_REST_API_COMMAND_HANDLER_H_
