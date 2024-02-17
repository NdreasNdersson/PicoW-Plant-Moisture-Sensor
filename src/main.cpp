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
#include "utils/json_handler.h"
#include "utils/logging.h"

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 10UL)
#define STATUS_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define REST_API_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)

std::atomic<bool> wifi_connected{false};
std::atomic<bool> initialized{false};

void status_task(void *params) {
    LogDebug(("Started\n"));

    while (!initialized)
        ;
    while (true) {
        /* runTimeStats(); */

        if (wifi_connected) {
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
    LogDebug(("Started\n"));

    if (WifiHelper::init()) {
        initialized = true;
        LogDebug(("Wifi Controller Initialised\n"));
    } else {
        LogError(("Failed to initialise controller\n"));
        return;
    }

    char str[1024];
    LogInfo(
        ("It is now possible to send json formatted strings to set persistant "
         "variables\n"));
    LogError(
        ("wifi_ssid and wifi_password are not set. These must be set to "
         "continue\n"));
    gets(str);
    auto json_handler = JsonHandler();
    json_handler.parse_json(str);
    auto wifi_ssid = json_handler.get_value("wifi_ssid");
    auto wifi_password = json_handler.get_value("wifi_password");
    if (wifi_ssid == NULL || wifi_password == NULL) {
        LogError(("wifi_ssid and wifi_password could not be parsed\n"));
        return;
    }

    LogDebug(("Connecting to WiFi... %s \n", wifi_ssid));

    if (WifiHelper::join(wifi_ssid, wifi_password)) {
        LogInfo(("Connect to Wifi\n"));
        wifi_connected = true;
    } else {
        LogError(("Failed to connect to Wifi \n"));
    }

    // Print MAC Address
    char macStr[20];
    WifiHelper::getMACAddressStr(macStr);
    LogInfo(("MAC ADDRESS: %s\n", macStr));

    // Print IP Address
    char ipStr[20];
    WifiHelper::getIPAddressStr(ipStr);
    LogInfo(("IP ADDRESS: %s\n", ipStr));

    auto rest_api{RestApi()};
    if (!rest_api.start()) {
        LogError(("RestApi failed to launch"));
        return;
    }

    std::vector<std::string> device_names{"moisture 1", "moisture 6"};
    if (!rest_api.register_device(device_names[0], "0")) {
        LogError(("Failed to register %s device", device_names[0]));
        return;
    }
    /* if (!rest_api.register_device(device_names[1], "0")) { */
    /*     LogError(("Failed to register %s device", device_names[1])); */
    /*     return; */
    /* } */

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory(2);
    auto sensors = sensor_factory.create({1, 6});
    if (sensors.empty()) {
        LogError(("Failed to initialise sensors"));
        return;
    }

    while (true) {
        vTaskDelay(3000);

        if (!WifiHelper::isJoined()) {
            LogError(("AP Link is down\n"));

            if (WifiHelper::join(wifi_ssid, wifi_password)) {
                LogInfo(("Connect to Wifi\n"));
                wifi_connected = true;
            } else {
                LogError(("Failed to connect to Wifi \n"));
                wifi_connected = false;
            }
        }

        auto counter{0};
        for (auto sensor : sensors) {
            auto value = sensor();
            printf("Pin %u: %f\n", counter, value);
            if (counter == 0) {
                rest_api.set_data(device_names[counter], std::to_string(value));
            }
            counter++;
        }
    }
}

void vLaunch(void) {
    xTaskCreate(main_task, "MainThread", 2048, NULL, MAIN_TASK_PRIORITY, NULL);
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
    LogInfo(("Starting %s\n", rtos_name));
    vLaunch();

    return 0;
}
