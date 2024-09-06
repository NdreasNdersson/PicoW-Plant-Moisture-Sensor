#include "config_handler.h"

#include <cstring>

#include "logging.h"
#include "pico/flash.h"
#include "utils/json_converter.h"

ConfigHandler::ConfigHandler()
    : m_flash_target_contents{reinterpret_cast<const uint8_t *>(
          ADDR_AS_U32(__APP_STORAGE_ADDRESS))} {
    nlohmann::json json;
    if (!read_json_from_flash(json)) {
        LogInfo(("No previously saved config data, write default values"));
        std::vector<sensor_config_t> sensor_config{};
        wifi_config_t wifi_config{};
        nlohmann::json json_new;

        json_new["sensors"] = nlohmann::json(sensor_config);
        json_new["wifi"] = nlohmann::json(wifi_config);
        write_json_to_flash(json_new);
    }
}

auto ConfigHandler::write_config(const std::vector<sensor_config_t> &config)
    -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        json["sensors"] = config;
        status = write_json_to_flash(json);
    } else {
        nlohmann::json json_new;
        LogInfo(("No previously saved data, write sensor config"));
        json_new["sensors"] = nlohmann::json(config);
        status = write_json_to_flash(json_new);
    }

    return status;
}

auto ConfigHandler::write_config(const wifi_config_t &config) -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        json["wifi"] = nlohmann::json(config);
        status = write_json_to_flash(json);
    } else {
        nlohmann::json json_new;
        LogInfo(("No previously saved data, write wifi config"));
        json_new["wifi"] = nlohmann::json(config);
        status = write_json_to_flash(json_new);
    }

    return status;
}

auto ConfigHandler::read_config(std::vector<sensor_config_t> &config) -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        for (auto sensor : json["sensors"]) {
            config.emplace_back(sensor.get<sensor_config_t>());
        }
        status = true;
    }

    return status;
}

auto ConfigHandler::read_config(wifi_config_t &config) -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        config = json["wifi"].get<wifi_config_t>();
        status = true;
    }

    return status;
}

auto ConfigHandler::write_json_to_flash(const nlohmann::json &json_data)
    -> bool {
    flash_data_t data{};
    auto json_string{json_data.dump()};

    if (json_string.length() > MAX_FLASH_SIZE) {
        LogError(("Can't write more than %u!", MAX_FLASH_SIZE));
        return false;
    }
    LogDebug(("Write %u of data", json_string.length()));

    std::memcpy(data.data, json_string.c_str(), json_string.length());
    data.number_of_pages = json_string.length() / FLASH_PAGE_SIZE + 1U;
    return write(data);
}

auto ConfigHandler::read_json_from_flash(nlohmann::json &json_data) -> bool {
    uint8_t data[MAX_FLASH_SIZE];
    auto status{true};
    std::memcpy(data, m_flash_target_contents, sizeof(data));
    json_data =
        nlohmann::json::parse(reinterpret_cast<char *>(data), nullptr, false);

    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from flash"));
        status = false;
    }

    return status;
}

auto ConfigHandler::write(flash_data_t &data) -> bool {
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
    flash_range_erase(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_STORAGE_ADDRESS),
                      FLASH_SECTOR_SIZE);
    flash_range_program(ADDR_WITH_XIP_OFFSET_AS_U32(__APP_STORAGE_ADDRESS),
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
