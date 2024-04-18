#include <functional>
#include <string>
#include <vector>

#include "FreeRTOS.h"
#include "hal/task_priorities.h"
#include "network/rest_api.h"
#include "network/wifi_config.h"
#include "network/wifi_helper.h"
#include "nlohmann/json.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "sensors/ads1115_adc.h"
#include "sensors/sensor_config.h"
#include "sensors/sensor_factory.h"
#include "task.h"
#include "utils/button/button_control.h"
#include "utils/config_handler.h"
#include "utils/json_converter.h"
#include "utils/led/led_control.h"
#include "utils/logging.h"

#define PICO_UART uart0
#define PICO_UART_BAUD_RATE PICO_DEFAULT_UART_BAUD_RATE
enum { PICO_UART_TX_PIN = 16, PICO_UART_RX_PIN = 17 };

#define PRINT_TASK_INFO (1)

static constexpr TickType_t MAIN_LOOP_SLEEP_MS{500U};

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
    led_control.set(LedPin::led_c, true);
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
        vTaskDelete(nullptr);
    }

    auto button_control = ButtonControl();
    auto led_control = LedControl();
    if (WifiHelper::init()) {
        LogDebug(("Wifi Controller Initialised"));
        set_led_in_not_connected_mode(led_control);
    } else {
        LogError(("Failed to initialise controller"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(nullptr);
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

    auto rest_api{RestApi(
        [&led_control](bool value) { led_control.set(LedPin::led_b, value); })};
    if (!rest_api.start()) {
        LogError(("RestApi failed to launch"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(nullptr);
    }

    std::vector<sensor_config_t> sensor_config;
    if (config_handler.read_config(sensor_config)) {
        for (auto sensor : sensor_config) {
            if (sensor.pin < 1) {
                LogError(("Sensor config pin must be >= 1"));
                continue;
            }

            float value{};
            std::string name{};
        }
        rest_api.set_device_config(nlohmann::json{sensor_config});
    } else {
        LogInfo(("No sensors configured"));
    }

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory();
    std::vector<Ads1115Adc> sensors;
    sensor_factory.create(
        sensor_config, sensors, button_control,
        [&led_control](bool value) { led_control.set(LedPin::led_a, value); },
        static_cast<float>(MAIN_LOOP_SLEEP_MS) / 1000.0F);

    if (sensor_config.size() != sensors.size()) {
        LogError(("Not all sensors was configured"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(nullptr);
    }

    std::string received_data;
    nlohmann::json rest_api_data;
    while (true) {
        vTaskDelay(MAIN_LOOP_SLEEP_MS / portTICK_PERIOD_MS);

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

        auto save_config{false};
        auto update_rest_api{true};
        for (auto i{0}; i < sensors.size(); i++) {
            float value{};
            std::string name{};
            sensors[i].get_name(name);
            if (sensors[i].read() == SensorReadStatus::Calibrating) {
                save_config = true;
            }
            sensors[i].get_config(sensor_config[i]);

            rest_api_data[name]["value"] = sensors[i].get_value();
            rest_api_data[name]["raw_value"] = sensors[i].get_raw_value();
        }
        if (update_rest_api) {
            rest_api.set_device_data(rest_api_data);
        }

        if (save_config) {
            config_handler.write_config(sensor_config);
            rest_api.set_device_config(nlohmann::json{sensor_config});
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

void vLaunch() {
    xTaskCreate(main_task, "MainThread", 2048, nullptr, MAIN_TASK_PRIORITY,
                nullptr);
    xTaskCreate(print_task, "LoggerThread", configMINIMAL_STACK_SIZE, nullptr,
                LOGGER_TASK_PRIORITY, nullptr);
#if PRINT_TASK_INFO == 1
    xTaskCreate(status_task, "StatusThread", configMINIMAL_STACK_SIZE, nullptr,
                STATUS_TASK_PRIORITY, nullptr);
#endif

    vTaskStartScheduler();
}

auto main() -> int {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);

    sleep_ms(1000);
    init_queue();

    vLaunch();

    return 0;
}
