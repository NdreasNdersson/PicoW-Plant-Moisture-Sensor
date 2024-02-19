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
#include "utils/logging.h"

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 10UL)
#define STATUS_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define REST_API_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)

std::atomic<bool> wifi_connected{false};
std::atomic<bool> initialized{false};
std::atomic<bool> main_failed{false};

void status_task(void *params) {
    LogDebug(("Started"));

    while (!initialized)
        ;
    while (true) {
        /* runTimeStats(); */

        if (main_failed) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(100);
        } else if (wifi_connected) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(1000);
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(500);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(500);
        }
    }
}

void main_task(void *params) {
    LogDebug(("Started"));

    if (WifiHelper::init()) {
        initialized = true;
        LogDebug(("Wifi Controller Initialised"));
    } else {
        LogError(("Failed to initialise controller"));
        main_failed = true;
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
            main_failed = true;
            vTaskDelete(NULL);
        }

        config_handler.write_json_structure(str_copy, sizeof(str_copy));
    }

    if (WifiHelper::join(wifi_ssid, wifi_password)) {
        LogInfo(("Connect to: %s", wifi_ssid));
        wifi_connected = true;
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
        main_failed = true;
        vTaskDelete(NULL);
    }

    std::vector<std::string> device_names{"moisture_1", "moisture_6"};
    for (auto &device : device_names) {
        if (!rest_api.register_device(device, "0")) {
            LogError(("Failed to register %s device", device));
            main_failed = true;
            vTaskDelete(NULL);
        }
    }

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory(2);
    auto sensors = sensor_factory.create({1, 6});
    if (sensors.empty()) {
        LogError(("Failed to initialise sensors"));
        main_failed = true;
        vTaskDelete(NULL);
    }

    while (true) {
        vTaskDelay(3000);

        if (!WifiHelper::isJoined()) {
            LogError(("AP Link is down"));

            if (WifiHelper::join(wifi_ssid, wifi_password)) {
                LogInfo(("Connect to Wifi"));
                wifi_connected = true;
            } else {
                LogError(("Failed to connect to Wifi "));
                wifi_connected = false;
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
    xTaskCreate(status_task, "StatusThread", 256, NULL, STATUS_TASK_PRIORITY,
                NULL);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void) {
    stdio_init_all();

    sleep_ms(1000);

    const char *rtos_name;
    rtos_name = "FreeRTOS";
    LogInfo(("Starting %s", rtos_name));
    vLaunch();

    return 0;
}
