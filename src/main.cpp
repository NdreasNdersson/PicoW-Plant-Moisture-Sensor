#include "FreeRTOS.h"
#include "utils/logging.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "run_time_stats/run_time_stats.h"
#include "task.h"
#include "wifi_helper/WiFiHelper.h"

#include <atomic>
#include <stdio.h>

// Check these definitions where added from the makefile
#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD not defined"
#endif

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define STATUS_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

std::atomic<bool> wifi_connected{false};
std::atomic<bool> initialized{false};

void status_task(void *params) {

    LogDebug(("Started\n"));

    while(!initialized);
    while(true) {
        /* runTimeStats(); */
        if (wifi_connected) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(1000);
        }
        else
        {
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

    while (true) {

        vTaskDelay(3000);

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
    xTaskCreate(status_task, "StatusThread", 256, NULL, STATUS_TASK_PRIORITY, NULL);

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
