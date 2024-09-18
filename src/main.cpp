#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "FreeRTOS.h"
#include "common_definitions.h"
#include "hal/task_priorities.h"
#include "network/rest_api.h"
#include "network/wifi_config.h"
#include "network/wifi_helper.h"
#include "nlohmann/json.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "sensors/sensor.h"
#include "sensors/sensor_config.h"
#include "sensors/sensor_factory.h"
#include "task.h"
#include "utils/button/button_control.h"
#include "utils/config_handler.h"
#include "utils/led/led_control.h"
#include "utils/logging.h"

#define PRINT_TASK_INFO (0)

static constexpr TickType_t MAIN_LOOP_SLEEP_MS{5000U};

void status_task(void *) {
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

void main_task(void *) {
    LogDebug(("Started"));
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    wifi_config_t wifi_config{};
    std::vector<sensor_config_t> sensor_config{};
    ConfigHandler config_handler{};
    {
        auto wifi_config_status{config_handler.read_config(wifi_config)};
        if (wifi_config_status && (wifi_config.ssid != "") &&
            (wifi_config.password != "")) {
            LogDebug(("Use SSID %s and password %s", wifi_config.ssid.c_str(),
                      wifi_config.password.c_str()));
        } else {
            LogInfo((
                "wifi.ssid and wifi.password are not set. These must be set to "
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
                wifi_config.ssid = ssid;
                wifi_config.password = password;
            }
            config_handler.write_config(wifi_config);
        }

        if (config_handler.read_config(sensor_config)) {
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

    auto button_control = std::make_shared<ButtonControl>();
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

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory();
    auto sensors{std::vector<std::shared_ptr<Sensor>>()};
    sensor_factory.create(
        sensor_config, sensors, button_control,
        [&led_control](bool value) { led_control.set(LedPin::led_a, value); },
        static_cast<float>(MAIN_LOOP_SLEEP_MS) / 1000.0F);

    auto rest_api{RestApi(
        [&led_control](bool value) { led_control.set(LedPin::led_b, value); },
        sensors)};
    if (!rest_api.start()) {
        LogError(("RestApi failed to launch"));
        set_led_in_failed_mode(led_control);
        vTaskDelete(nullptr);
    }

    nlohmann::json rest_api_data;
    while (true) {
        vTaskDelay(MAIN_LOOP_SLEEP_MS / portTICK_PERIOD_MS);

        auto save_config{false};
        std::vector<sensor_config_t> temp_sensor_config{};
        for (const auto &sensor : sensors) {
            sensor_config_t temp_config{};
            if (sensor->read(temp_config) ==
                SensorReadStatus::CalibrationComplete) {
                save_config = true;
                if (temp_config.type != "") {
                    temp_sensor_config.push_back(temp_config);
                }
            }
        }

        if (save_config) {
            config_handler.write_config(temp_sensor_config);
        }

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
    }
}

void vLaunch() {
    xTaskCreate(main_task, "MainThread", 2048, nullptr, MAIN_TASK_PRIORITY,
                nullptr);
#if PRINT_TASK_INFO == 1
    xTaskCreate(status_task, "StatusThread", configMINIMAL_STACK_SIZE, nullptr,
                STATUS_TASK_PRIORITY, nullptr);
#endif

    vTaskStartScheduler();
}

auto main() -> int {
    stdio_uart_init_full(PICO_UART, PICO_UART_BAUD_RATE, PICO_UART_TX_PIN,
                         PICO_UART_RX_PIN);

    vLaunch();

    return 0;
}
