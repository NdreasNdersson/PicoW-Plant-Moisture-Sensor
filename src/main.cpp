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

    auto config_handler = ConfigHandler();

#if (configNUMBER_OF_CORES == 1)
    LogDebug(("Running on one core, write config"));
    std::string json_str{};
    config_handler.write_json_to_flash(json_str);
#endif

    auto json_data = config_handler.read_json_from_flash();

    wifi_config_t wifi_config{};
    if (json_data.contains("wifi")) {
        wifi_config = json_data["wifi"].get<wifi_config_t>();
        LogDebug(("Get SSID %s and password %s", wifi_config.ssid.c_str(),
                  wifi_config.password.c_str()));
    } else {
        LogError(
            ("wifi_ssid and wifi_password are not set. These must be set to "
             "continue"));
        vTaskDelete(NULL);
    }

#if (configNUMBER_OF_CORES == 1)
    LogError(("LWIP FreeRTOS can't run on one core, stopping main thread"));
    vTaskDelete(NULL);
#endif

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

    std::map<int, sensor_config_t> sensor_config{};
    if (json_data.contains("sensors") && json_data["sensors"].is_array()) {
        for (auto &sensor : json_data["sensors"]) {
            auto current_sensor_config{sensor.get<sensor_config_t>()};

#if CALIBRATE_SENSORS == 1
            current_sensor_config.run_calibration = true;
#endif
            if (current_sensor_config.pin < 1) {
                LogError(("Sensor config pin must be >= 1"));
                continue;
            }
            sensor_config[current_sensor_config.pin] = current_sensor_config;

            std::string name{current_sensor_config.type + "_" +
                             std::to_string(current_sensor_config.pin)};
            if (!rest_api.register_device(name, "0")) {
                LogError(("Failed to register %s device", name));
                set_led_in_failed_mode(led_control);
                vTaskDelete(NULL);
            }
        }
    }

    LogInfo(("Initialise sensors"));
    auto sensor_factory = SensorFactory();
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
