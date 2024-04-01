#include <string>
#include <vector>

#include "FreeRTOS.h"
#include "network/rest_api.h"
#include "network/wifi_config.h"
#include "network/wifi_helper.h"
#include "nlohmann/json.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "sensors/sensor_config.h"
#include "sensors/sensor_factory.h"
#include "task.h"
#include "utils/config_handler.h"
#include "utils/json_converter.h"
#include "utils/led/led_control.h"
#include "utils/logging.h"

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 10UL)
#define STATUS_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define LOGGER_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define REST_API_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)

#define PICO_UART uart0
#define PICO_UART_BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
#define PICO_UART_TX_PIN 16
#define PICO_UART_RX_PIN 17

#define PRINT_TASK_INFO (0)

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

    auto config_handler = ConfigHandler();

    wifi_config_t wifi_config{};
    if (config_handler.read_config(wifi_config)) {
        LogDebug(("Get SSID %s and password %s", wifi_config.ssid.c_str(),
                  wifi_config.password.c_str()));
    } else {
        LogError(
            ("wifi.ssid and wifi.password are not set. These must be set to "
             "continue"));
        vTaskDelete(NULL);
    }

    auto led_control = LedControl();
    if (WifiHelper::init()) {
        LogDebug(("Wifi Controller Initialised"));
        set_led_in_not_connected_mode(led_control);
    } else {
        LogError(("Failed to initialise controller"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    if (WifiHelper::join(wifi_config)) {
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

    std::vector<sensor_config_t> sensor_config;
    if (config_handler.read_config(sensor_config)) {
        for (auto sensor : sensor_config) {
            if (sensor.pin < 1) {
                LogError(("Sensor config pin must be >= 1"));
                continue;
            }

            std::string name{sensor.type + "_" + std::to_string(sensor.pin)};
            if (!rest_api.register_device(name, "0")) {
                LogError(("Failed to register %s device", name));
                set_led_in_failed_mode(led_control);
                vTaskDelete(NULL);
            }
        }
    } else {
        LogInfo(("No sensors configured"));
    }

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory();
    auto sensors = sensor_factory.create(sensor_config);
    if (sensors.empty()) {
        LogError(("Failed to initialise sensors"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(NULL);
    }

    std::string received_data;
    while (true) {
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        if (!WifiHelper::isJoined()) {
            LogError(("AP Link is down"));

            if (WifiHelper::join(wifi_config)) {
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

        if (rest_api.get_data(received_data)) {
            auto json_data =
                nlohmann::json::parse(received_data, nullptr, false);

            if (json_data.is_discarded()) {
                LogError(("Failed to parse json from received data"));
                continue;
            }

            if (json_data.contains("sensors") &&
                json_data["sensors"].is_array()) {
                std::vector<sensor_config_t> config =
                    json_data["sensors"].get<std::vector<sensor_config_t>>();
                config_handler.write_config(config);
            }
        }
    }
}

void vLaunch(void) {
    xTaskCreate(main_task, "MainThread", 2048, NULL, MAIN_TASK_PRIORITY, NULL);
    xTaskCreate(print_task, "LoggerThread", configMINIMAL_STACK_SIZE, NULL,
                LOGGER_TASK_PRIORITY, NULL);
#if PRINT_TASK_INFO == 1
    xTaskCreate(status_task, "StatusThread", configMINIMAL_STACK_SIZE, NULL,
                STATUS_TASK_PRIORITY, NULL);
#endif

    vTaskStartScheduler();
}

int main(void) {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);

    sleep_ms(1000);
    init_queue();

    vLaunch();

    return 0;
}
