#include "config_handler.h"

extern "C" {
#include "hardware/sync.h"
};

#include <cstring>
#include <string>

#include "logging.h"

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

bool ConfigHandler::write_json_to_flash(char *json, size_t data_length) {
    page_t data{};
    if (data_length > sizeof(data)) {
        LogError(("Can't write more than %u!", sizeof(data)));
        return false;
    }
    LogDebug(("Write %u of data", data_length));

    std::memcpy(data, json, data_length);
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

    if (mismatch) {
        LogDebug(("Erase region ... "));
        uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
        restore_interrupts(ints);

        LogDebug(("Program region ... "));
        ints = save_and_disable_interrupts();
        flash_range_program(FLASH_TARGET_OFFSET, data, FLASH_PAGE_SIZE);
        restore_interrupts(ints);

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
