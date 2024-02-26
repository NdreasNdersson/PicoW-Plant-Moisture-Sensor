#include <stdio.h>

#include <atomic>
#include <string>

#include "FreeRTOS.h"
#include "network/rest_api.h"
#include "network/wifi_helper.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "sensors/sensor_factory.h"
#include "task.h"
#include "utils/config_handler.h"
#include "utils/led/led_control.h"
#include "utils/logging.h"

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 10UL)
#define STATUS_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define LOGGER_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define REST_API_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)

#define PRINT_TASK_INFO (0)
#define CALIBRATE_SENSORS (0)

void status_task(void *params) {
    while (true) {
        runTimeStats();

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void set_led_in_not_connected_mode(LedControl &led_control) {
    led_control.set_blink_delay(LedPin::led_c, 500);
    led_control.start_blink(LedPin::led_c);
}

void set_led_in_failed_mode(LedControl &led_control) {
    led_control.set_blink_delay(LedPin::led_c, 3000);
    led_control.start_blink(LedPin::led_c);
}

void set_led_in_connected_mode(LedControl &led_control) {
    led_control.set_on(LedPin::led_c);
}

void main_task(void *params) {
    LogDebug(("Started"));
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    auto led_control = LedControl();

    if (WifiHelper::init()) {
        LogDebug(("Wifi Controller Initialised"));
        set_led_in_not_connected_mode(led_control);
    } else {
        LogError(("Failed to initialise controller"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    auto config_handler = ConfigHandler();

    ConfigHandler::return_value_t config_wifi_ssid{};
    ConfigHandler::return_value_t config_wifi_password{};
    char const *wifi_ssid;
    char const *wifi_password;

    auto json_data = config_handler.read_json_from_flash();

    if (json_data.contains("wifi") && (json_data["wifi"]).contains("ssid") &&
        (json_data["wifi"])["ssid"].is_string() &&
        (json_data["wifi"]).contains("password") &&
        (json_data["wifi"])["password"].is_string()) {
        LogDebug(("Get SSID and password"));
        wifi_ssid = (json_data["wifi"])["ssid"].get<std::string>().c_str();
        wifi_password =
            (json_data["wifi"])["password"].get<std::string>().c_str();
        LogDebug(("Get SSID %s and password %s", wifi_ssid, wifi_password));
    } else {
        LogError(
            ("wifi_ssid and wifi_password are not set. These must be set to "
             "continue"));
        vTaskDelete(NULL);
    }

    if (WifiHelper::join(wifi_ssid, wifi_password)) {
        LogInfo(("Connect to: %s", wifi_ssid));
        set_led_in_connected_mode(led_control);
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

    auto rest_api{RestApi()};
    if (!rest_api.start()) {
        LogError(("RestApi failed to launch"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    std::map<int, sensor_config_t> sensor_config{};
    if (json_data.contains("sensors") && json_data["sensors"].is_array()) {
        for (auto &sensor : json_data["sensors"]) {
            sensor_config_t current_sensor_config{};
            int pin{};
            if (sensor.contains("pin")) {
                pin = sensor["pin"].get<int>();
            }
            if (sensor.contains("max")) {
                current_sensor_config.max_value =
                    sensor["max"].get<std::uint16_t>();
            }
            if (sensor.contains("min")) {
                current_sensor_config.min_value =
                    sensor["min"].get<std::uint16_t>();
            }
            if (sensor.contains("type")) {
                current_sensor_config.name = sensor["type"].get<std::string>() +
                                             "_" + std::to_string(pin);
            }
            if (sensor.contains("inversed")) {
                current_sensor_config.inverse_measurement =
                    sensor["inversed"].get<bool>();
            }

#if CALIBRATE_SENSORS == 1
            current_sensor_config.run_calibration = true;
#endif
            sensor_config[pin] = current_sensor_config;

            if (!rest_api.register_device(current_sensor_config.name, "0")) {
                LogError(("Failed to register %s device",
                          current_sensor_config.name));
                set_led_in_failed_mode(led_control);
                vTaskDelete(NULL);
            }
        }
    }

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory(2);
    auto sensors = sensor_factory.create(sensor_config);
    if (sensors.empty()) {
        LogError(("Failed to initialise sensors"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    while (true) {
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        if (!WifiHelper::isJoined()) {
            LogError(("AP Link is down"));

            if (WifiHelper::join(wifi_ssid, wifi_password)) {
                LogInfo(("Connect to Wifi"));
                set_led_in_connected_mode(led_control);
            } else {
                LogError(("Failed to connect to Wifi "));
                set_led_in_not_connected_mode(led_control);
            }
        }

        for (auto &sensor : sensors) {
            float value{};
            std::string name{};
            sensor(value, name);
            rest_api.set_data(name, std::to_string(value));
        }
    }
}

void vLaunch(void) {
    xTaskCreate(main_task, "MainThread", 2048, NULL, MAIN_TASK_PRIORITY, NULL);
    xTaskCreate(print_task, "LoggerThread", 256, NULL, LOGGER_TASK_PRIORITY,
                NULL);
#if PRINT_TASK_INFO == 1
    xTaskCreate(status_task, "StatusThread", 256, NULL, STATUS_TASK_PRIORITY,
                NULL);
#endif

    vTaskStartScheduler();
}

int main(void) {
    stdio_init_all();

    sleep_ms(1000);
    init_queue();

    vLaunch();

    return 0;
}
