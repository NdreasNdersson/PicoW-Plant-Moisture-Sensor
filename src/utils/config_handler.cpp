#include "config_handler.h"

#include <cstring>

#include "logging.h"
#include "pico/flash.h"

ConfigHandler::ConfigHandler()
    : m_flash_target_contents{
          (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET)} {}

json ConfigHandler::read_json_from_flash() {
    uint8_t data[MAX_FLASH_SIZE];
    std::memcpy(data, m_flash_target_contents, sizeof(data));
    LogDebug(("Read from flash: %s", reinterpret_cast<char *>(data)));
    auto json_data =
        json::parse(reinterpret_cast<char *>(data), nullptr, false);

    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from flash"));
    }

    return json_data;
}

bool ConfigHandler::write_json_to_flash(std::string json) {
    flash_data_t data{};

    if (json.length() > MAX_FLASH_SIZE) {
        LogError(("Can't write more than %u!", MAX_FLASH_SIZE));
        return false;
    }
    LogDebug(("Write %u of data", json.length()));

    std::memcpy(data.data, json.c_str(), json.length());
    data.number_of_pages = json.length() / FLASH_PAGE_SIZE + 1U;
    return write(data);
}

bool ConfigHandler::write(flash_data_t &data) {
    bool mismatch = false;

    if (data.number_of_pages > (MAX_NUMBER_OF_PAGES)) {
        LogError(("Can't write more than %u number of pages, %u bytes!",
                  MAX_NUMBER_OF_PAGES, MAX_FLASH_SIZE));
        return false;
    }
    LogDebug(("Flash number of pages: %u", data.number_of_pages));
    for (int i = 0; i < (FLASH_PAGE_SIZE * data.number_of_pages); ++i) {
        if (data.data[i] != m_flash_target_contents[i]) {
            mismatch = true;
            break;
        }
    }
    mismatch = true;

    if (mismatch) {
        LogDebug(("Erase region and program... "));

        auto status{flash_safe_execute(&erase_and_program,
                                       static_cast<void *>(&data), 100U)};
        if (PICO_OK != status) {
            LogError(("Flash safe execute failed with code: %u!", status));
        }

        mismatch = false;
        for (int i = 0; i < (FLASH_PAGE_SIZE * data.number_of_pages); ++i) {
            if (data.data[i] != m_flash_target_contents[i]) {
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

void ConfigHandler::erase_and_program(void *data) {
    auto flash_data{static_cast<flash_data_t *>(data)};
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET,
                        static_cast<uint8_t *>(flash_data->data),
                        FLASH_PAGE_SIZE * flash_data->number_of_pages);
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
