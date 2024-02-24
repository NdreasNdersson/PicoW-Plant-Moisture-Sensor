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
#include "utils/json_handler.h"
#include "utils/led/led_control.h"
#include "utils/logging.h"

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 10UL)
#define STATUS_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define LOGGER_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define REST_API_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)
#define PRINT_TASK_INFO (0)
#define CALIBRATE_SENSORS (1)

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
    led_control.set_blink_delay(LedPin::led_c, 100);
    led_control.start_blink(LedPin::led_c);
}

void set_led_in_connected_mode(LedControl &led_control) {
    led_control.set_on(LedPin::led_c);
}

void main_task(void *params) {
    LogDebug(("Started"));

    vTaskDelay(3000);
    auto led_control = LedControl();

    if (WifiHelper::init()) {
        set_led_in_not_connected_mode(led_control);
        LogDebug(("Wifi Controller Initialised"));
    } else {
        LogError(("Failed to initialise controller"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    auto json_handler = JsonHandler();
    auto config_handler = ConfigHandler(json_handler);

    ConfigHandler::return_value_t config_wifi_ssid{};
    ConfigHandler::return_value_t config_wifi_password{};
    char const *wifi_ssid;
    char const *wifi_password;

    if (config_handler.get_config_value("wifi_ssid", config_wifi_ssid) &&
        config_handler.get_config_value("wifi_password",
                                        config_wifi_password)) {
        wifi_ssid = config_wifi_ssid;
        wifi_password = config_wifi_password;
    } else {
        LogError(
            ("wifi_ssid and wifi_password are not set. These must be set to "
             "continue"));
        char str[256]{};
        gets(str);
        char str_copy[256]{};
        strcpy(str_copy, str);
        json_handler.parse_json(str);

        wifi_ssid = json_handler.get_value("wifi_ssid");
        wifi_password = json_handler.get_value("wifi_password");
        if (wifi_ssid == NULL || wifi_password == NULL) {
            LogError(("wifi_ssid and wifi_password could not be parsed"));
            set_led_in_failed_mode(led_control);
            vTaskDelete(NULL);
        }

        config_handler.write_json_structure(str_copy, sizeof(str_copy));
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

    std::vector<std::string> device_names{"moisture_1", "moisture_6"};
    for (auto &device : device_names) {
        if (!rest_api.register_device(device, "0")) {
            LogError(("Failed to register %s device", device));
            set_led_in_failed_mode(led_control);
            vTaskDelete(NULL);
        }
    }

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory(2);
#if CALIBRATE_SENSORS == 1
    auto sensors = sensor_factory.create({1, 6}, true);
#else
    auto sensors = sensor_factory.create({1, 6}, false);
#endif
    if (sensors.empty()) {
        LogError(("Failed to initialise sensors"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

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

        auto counter{0};
        for (auto sensor : sensors) {
            auto value = sensor();
            rest_api.set_data(device_names[counter], std::to_string(value));
            counter++;
        }
    }
}

void vLaunch(void) {
    xTaskCreate(main_task, "MainThread", 1024, NULL, MAIN_TASK_PRIORITY, NULL);
#if PRINT_TASK_INFO == 1
    xTaskCreate(status_task, "StatusThread", 256, NULL, STATUS_TASK_PRIORITY,
                NULL);
#endif
    xTaskCreate(print_task, "LoggerThread", 256, NULL, LOGGER_TASK_PRIORITY,
                NULL);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void) {
    stdio_init_all();

    sleep_ms(1000);
    init_queue();

    vLaunch();

    return 0;
}
