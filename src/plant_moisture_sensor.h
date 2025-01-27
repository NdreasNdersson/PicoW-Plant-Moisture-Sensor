#ifndef PICO_REST_SENSOR_MAIN_H_
#define PICO_REST_SENSOR_MAIN_H_

#include <memory>

#include "hal/freertos_interface_impl.h"
#include "hal/pico_interface_impl.h"
#include "network/mqtt_client.h"
#include "network/rest_api.h"
#include "patterns/subscriber.h"
#include "sensors/sensor.h"
#include "software_download.h"
#include "utils/button/button_control.h"
#include "utils/config_handler/config_handler_impl.h"
#include "utils/config_handler/configs/wifi_config.h"
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

    PicoInterfaceImpl pico_interface_;
    FreertosInterfaceImpl freertos_interface_;
    ConfigHandlerImpl config_handler_;
    std::unique_ptr<ButtonControl> button_control_;
    std::vector<std::shared_ptr<Sensor>> sensors_;
    LedControl led_control_;
    wifi_config_t wifi_config_;
    RestApi rest_api_;
    std::unique_ptr<MqttClient> mqtt_client_;
    PicoBootloader::SoftwareDownload software_download_;
    std::unique_ptr<RestApiCommandHandler> rest_api_command_handler_;
};

#endif  // PICO_REST_SENSOR_MAIN_H_
