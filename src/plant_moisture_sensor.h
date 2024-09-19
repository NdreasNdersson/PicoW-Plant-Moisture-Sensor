#ifndef PICO_REST_SENSOR_MAIN_H_
#define PICO_REST_SENSOR_MAIN_H_

#include <memory>

#include "network/rest_api.h"
#include "network/wifi_config.h"
#include "patterns/subscriber.h"
#include "sensors/sensor.h"
#include "utils/button/button_control.h"
#include "utils/config_handler.h"
#include "utils/led/led_control.h"

class PlantMoistureSensor : public Subscriber<int> {
   public:
    PlantMoistureSensor();
    ~PlantMoistureSensor();
    PlantMoistureSensor(const PlantMoistureSensor &) = delete;
    PlantMoistureSensor(PlantMoistureSensor &&) = delete;
    PlantMoistureSensor &operator=(const PlantMoistureSensor &) = delete;
    PlantMoistureSensor &operator=(PlantMoistureSensor &&) = delete;
    void init();
    void loop();
    void update(const int &);

   private:
    void set_led_in_not_connected_mode();
    void set_led_in_failed_mode();
    void set_led_in_connected_mode();

    ConfigHandler config_handler_;
    std::unique_ptr<ButtonControl> button_control_;
    std::vector<std::shared_ptr<Sensor>> sensors_;
    LedControl led_control_;
    wifi_config_t wifi_config_;
    RestApi rest_api_;
    SoftwareDownload software_download_;
    std::unique_ptr<RestApiCommandHandler> rest_api_command_handler_;
};

#endif  // PICO_REST_SENSOR_MAIN_H_
