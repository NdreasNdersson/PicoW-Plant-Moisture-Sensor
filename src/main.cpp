#include "FreeRTOS.h"
#include "network/rest_api.h"
#include "network/wifi_helper.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "task.h"
#include "utils/logging.h"
extern "C" {
#include "ads1115.h"
}

#include <stdio.h>

#include <atomic>
#include <string>

#define I2C_PORT i2c0
#define I2C_FREQ 400000
#define ADS1115_I2C_ADDR_GND 0x48
#define ADS1115_I2C_ADDR_VDD 0x49
const uint8_t SDA_PIN = 8;
const uint8_t SCL_PIN = 9;

// Check these definitions where added from the makefile
#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD not defined"
#endif

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

    LogDebug(("Connecting to WiFi... %s \n", WIFI_SSID));

    if (WifiHelper::join(WIFI_SSID, WIFI_PASSWORD)) {
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

    std::string device_name{"moisture"};
    int device_value{0};
    if (!rest_api.register_device(device_name, std::to_string(device_value))) {
        LogError(("Failed to register %s device", device_name));
        return;
    }

    LogDebug(("Initialise I2C"));
    struct ads1115_adc adc;
    // Initialise I2C
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    LogDebug(("Initialise ads1115"));
    // Initialise ADC
    ads1115_init(I2C_PORT, ADS1115_I2C_ADDR_VDD, &adc);

    LogDebug(("Set ads1115 config"));
    // Modify the default configuration as needed. In this example the
    // signal will be differential, measured between pins A0 and A3,
    // and the full-scale voltage range is set to +/- 4.096 V.
    ads1115_set_input_mux(ADS1115_MUX_SINGLE_0, &adc);
    ads1115_set_pga(ADS1115_PGA_4_096, &adc);
    ads1115_set_data_rate(ADS1115_RATE_475_SPS, &adc);

    LogDebug(("Write ads1115 config"));
    // Write the configuration for this to have an effect.
    ads1115_write_config(&adc);

    // Data containers
    float volts;
    uint16_t adc_value;
    while (true) {
        vTaskDelay(3000);
        ads1115_read_adc(&adc_value, &adc);
        volts = ads1115_raw_to_volts(adc_value, &adc);
        printf("ADC: %u  Voltage: %f\n", adc_value, volts);

        rest_api.set_data(device_name, std::to_string(volts));

        if (!WifiHelper::isJoined()) {
            LogError(("AP Link is down\n"));

            if (WifiHelper::join(WIFI_SSID, WIFI_PASSWORD)) {
                LogInfo(("Connect to Wifi\n"));
                wifi_connected = true;
            } else {
                LogError(("Failed to connect to Wifi \n"));
                wifi_connected = false;
            }
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
