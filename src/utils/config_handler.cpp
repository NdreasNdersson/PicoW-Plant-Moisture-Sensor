#include "config_handler.h"

extern "C" {
#include "hardware/sync.h"
};

#include <cstring>

#include "FreeRTOS.h"
#include "logging.h"
#include "task.h"

ConfigHandler::ConfigHandler()
    : m_flash_target_contents{
          (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET)} {}

json ConfigHandler::read_json_from_flash() {
    page_t data;
    std::memcpy(data, m_flash_target_contents, sizeof(data));
    LogDebug(("Read from flash: %s", reinterpret_cast<char *>(data)));
    auto json_data = json::parse(reinterpret_cast<char *>(data));
    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from flash"));
    }

    return json_data;
}

bool ConfigHandler::write_json_to_flash(std::string json) {
    page_t data{};

    if (json.length() > sizeof(data)) {
        LogError(("Can't write more than %u!", sizeof(data)));
        return false;
    }
    LogDebug(("Write %u of data", json.length()));

    std::memcpy(data, json.c_str(), json.length());
    return write(data);
}

void ConfigHandler::read(page_t data) {}

bool ConfigHandler::write(page_t data) {
    bool mismatch = false;
    for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
        if (data[i] != m_flash_target_contents[i]) {
            mismatch = true;
            break;
        }
    }
    mismatch = true;

    if (mismatch) {
        LogDebug(("Erase region and program... "));

        vTaskSuspendAll();
        taskENTER_CRITICAL();
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        flash_range_program(FLASH_TARGET_OFFSET, data, FLASH_PAGE_SIZE);
        taskEXIT_CRITICAL();
        xTaskResumeAll();

        mismatch = false;
        for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
            if (data[i] != m_flash_target_contents[i]) {
                mismatch = true;
            }
        }
        if (mismatch) {
            LogError(("Flash programming failed!"));
        } else {
            LogDebug(("Flash programming successful!"));
        }
    } else {
        LogDebug(("No need to flash!"));
    }

    return !mismatch;
}

void ConfigHandler::print_buf(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", buf[i]);
        if (i % 16 == 15) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}
