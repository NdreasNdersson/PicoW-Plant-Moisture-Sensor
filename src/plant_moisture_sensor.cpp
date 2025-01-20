#include "src/plant_moisture_sensor.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "FreeRTOS.h"
#include "network/wifi_helper.h"
#include "nlohmann/json.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "sensors/sensor_config.h"
#include "sensors/sensor_factory.h"
#include "software_download.h"
#include "src/plant_moisture_sensor.h"
#include "task.h"
#include "utils/button/button_control.h"
#include "utils/logging.h"

static constexpr TickType_t MAIN_LOOP_SLEEP_MS{5000U};

PlantMoistureSensor::PlantMoistureSensor()
    : config_handler_{},
      button_control_{},
      sensors_{},
      led_control_{},
      wifi_config_{},
      rest_api_([this](bool value) { led_control_.set(LedPin::led_b, value); }),
      software_download_{},
      rest_api_command_handler_{} {}

PlantMoistureSensor::~PlantMoistureSensor() {
    if (button_control_ != nullptr) {
        button_control_->detach(ButtonNames::C, this);
    }
}
void PlantMoistureSensor::init() {
    LogDebug(("Started"));

    std::vector<sensor_config_t> sensor_config{};
    {
        auto wifi_config_status{config_handler_.read_config(wifi_config_)};
        if (wifi_config_status && (wifi_config_.ssid != "") &&
            (wifi_config_.password != "")) {
            LogDebug(("Use SSID %s and password %s", wifi_config_.ssid.c_str(),
                      wifi_config_.password.c_str()));
        } else {
            LogInfo(
                ("wifi.ssid and wifi.password are not set. These must be "
                 "set to "
                 "continue.\n Enter SSID <enter> then password <enter>:"));
            char ssid[128];
            char password[128];

            auto status{true};
            if (scanf("%s", ssid) <= 0) {
                status = false;
                LogError(("SSID was not correctly entered"));
            }
            if (scanf("%s", password) <= 0) {
                status = false;
                LogError(("Password was not correctly entered"));
            }
            if (status) {
                wifi_config_.ssid = ssid;
                wifi_config_.password = password;
            }
            config_handler_.write_config(wifi_config_);
        }

        if (config_handler_.read_config(sensor_config)) {
            LogInfo(("%d sensors configured", sensor_config.size()));
            for (const auto &sensor : sensor_config) {
                if (sensor.pin < 1) {
                    LogError(("Sensor config pin must be >= 1"));
                    continue;
                }
            }
        } else {
            LogInfo(("No sensors configured"));
        }
    }

    if (WifiHelper::init()) {
        LogDebug(("Wifi Controller Initialised"));
        set_led_in_not_connected_mode();
    } else {
        LogError(("Failed to initialise controller"));
        set_led_in_failed_mode();
        vTaskDelete(nullptr);
    }

    if (WifiHelper::join(wifi_config_)) {
        set_led_in_connected_mode();
    } else {
        LogError(("Failed to connect to Wifi "));
    }

    // Print MAC Address
    char macStr[20];
    WifiHelper::getMACAddressStr(macStr);
    LogInfo(("MAC ADDRESS: %s", macStr));

    // Print IP Address
    char ipStr[20];
    WifiHelper::getIPAddressStr(ipStr);
    LogInfo(("IP ADDRESS: %s", ipStr));

    button_control_ = std::make_unique<ButtonControl>();
    button_control_->attach(ButtonNames::C, this);
    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory();
    sensor_factory.create(
        sensor_config, sensors_, *button_control_,
        [this](bool value) { led_control_.set(LedPin::led_a, value); },
        static_cast<float>(MAIN_LOOP_SLEEP_MS) / 1000.0F);

    rest_api_command_handler_ =
        std::make_unique<RestApiCommandHandler>(software_download_, sensors_);
    auto get_callback{
        [this](const std::string &resource, std::string &payload) -> bool {
            return rest_api_command_handler_->get_callback(resource, payload);
        }};
    auto post_callback{[this](const std::string &resource,
                              const std::string &payload) -> bool {
        return rest_api_command_handler_->post_callback(resource, payload);
    }};

    if (!rest_api_.start(get_callback, post_callback)) {
        LogError(("RestApi failed to launch"));
        set_led_in_failed_mode();
        vTaskDelete(nullptr);
    }

    nlohmann::json rest_api_data;
}

void PlantMoistureSensor::loop() {
    while (true) {
        vTaskDelay(MAIN_LOOP_SLEEP_MS / portTICK_PERIOD_MS);

        auto any_calibration_complete{false};
        auto any_calibrating{false};
        std::vector<sensor_config_t> temp_sensor_config{};
        for (const auto &sensor : sensors_) {
            sensor_config_t temp_config{};

            auto status{sensor->read(temp_config)};
            if (status == SensorReadStatus::CalibrationComplete) {
                any_calibration_complete = true;
            } else if (status == SensorReadStatus::Calibrating) {
                any_calibrating = true;
            }

            if (temp_config.type != "") {
                temp_sensor_config.push_back(temp_config);
            }
        }

        if (any_calibration_complete && !any_calibrating) {
            config_handler_.write_config(temp_sensor_config);
        }

        if (!WifiHelper::isJoined()) {
            LogError(("AP Link is down"));

            if (WifiHelper::join(wifi_config_)) {
                LogInfo(("Connect to Wifi"));
                set_led_in_connected_mode();
            } else {
                LogError(("Failed to connect to Wifi "));
                set_led_in_not_connected_mode();
            }
        }
    }
}

// Subscriber
void PlantMoistureSensor::update(const int &) {
    const uint32_t reboot_delay{3000};

    LogInfo(("Reboot in 3s and request restore previous app"));
    if (!software_download_.restore(reboot_delay)) {
        LogError(("Failed to restore previous app"));
    }
}

// Private
void PlantMoistureSensor::set_led_in_not_connected_mode() {
    led_control_.set_blink_delay(LedPin::led_c, 500);
    led_control_.start_blink(LedPin::led_c);
}

void PlantMoistureSensor::set_led_in_failed_mode() {
    led_control_.set_blink_delay(LedPin::led_c, 3000);
    led_control_.start_blink(LedPin::led_c);
}

void PlantMoistureSensor::set_led_in_connected_mode() {
    led_control_.set(LedPin::led_c, true);
}
