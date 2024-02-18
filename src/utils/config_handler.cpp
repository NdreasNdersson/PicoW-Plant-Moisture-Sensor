#include "config_handler.h"

extern "C" {
#include "hardware/sync.h"
};

#include <cstring>

#include "logging.h"

ConfigHandler::ConfigHandler(JsonHandler &json_handler)
    : m_json_handler{json_handler},
      m_flash_target_contents{
          (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET)} {}

bool ConfigHandler::get_config_value(char const *config_name,
                                     return_value_t return_value) {
    bool return_status{false};
    page_t data;
    std::memcpy(data, m_flash_target_contents, sizeof(data));

    char const *value{NULL};
    if (m_json_handler.parse_json(reinterpret_cast<char *>(data))) {
        value = m_json_handler.get_value(config_name);
        if (value != NULL) {
            LogDebug(("Get %s value %s", config_name, value));
            return_status = true;
            std::memcpy(return_value, value, sizeof(return_value_t));
        }
    } else {
        LogError(("Failed to fetch %s from flash", config_name));
    }

    return return_status;
}

bool ConfigHandler::write_json_structure(char *json, size_t data_length) {
    page_t data{};
    if (data_length > sizeof(data)) {
        LogError(("Can't write more than %u!", sizeof(data)));
        return false;
    }
    LogDebug(("Write %u of data", data_length));
    print_buf(reinterpret_cast<uint8_t *>(json), data_length);

    std::memcpy(data, json, data_length);
    return write(data);
}

void ConfigHandler::read(page_t data) {}

bool ConfigHandler::write(page_t data) {
    bool mismatch = false;
    for (int i = 0; i < FLASH_PAGE_SIZE; ++i) {
        if (data[i] != m_flash_target_contents[i]) {
            mismatch = true;
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
