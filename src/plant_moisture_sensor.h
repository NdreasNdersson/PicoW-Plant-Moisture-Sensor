#ifndef PICO_REST_SENSOR_MAIN_H_
#define PICO_REST_SENSOR_MAIN_H_

#include <memory>

#include "network/rest_api.h"
#include "network/wifi_config.h"
#include "sensors/sensor.h"
#include "utils/button/button_control.h"
#include "utils/config_handler.h"
#include "utils/led/led_control.h"

class PlantMoistureSensor {
   public:
    PlantMoistureSensor();
    void init();
    void loop();

   private:
    void set_led_in_not_connected_mode();
    void set_led_in_failed_mode();
    void set_led_in_connected_mode();

    ConfigHandler config_handler_;
    std::shared_ptr<ButtonControl> button_control_;
    std::vector<std::shared_ptr<Sensor>> sensors_;
    LedControl led_control_;
    wifi_config_t wifi_config_;
    RestApi rest_api_;
    std::unique_ptr<RestApiCommandHandler> rest_api_command_handler_;
};

#endif  // PICO_REST_SENSOR_MAIN_H_
