#include "config_handler.h"

#include <cstdint>
#include <cstring>

#include "linker_definitions.h"
#include "logging.h"
#include "utils/json_converter.h"

ConfigHandler::ConfigHandler(PicoInterface &pico_interface)
    : pico_interface_{pico_interface} {}

auto ConfigHandler::init() -> bool {
    auto status{true};
    nlohmann::json json;
    if (!read_json_from_flash(json)) {
        LogInfo(("No previously saved config data, write default values"));
        std::vector<sensor_config_t> sensor_config{};
        wifi_config_t wifi_config{};
        nlohmann::json json_new;

        json_new["sensors"] = nlohmann::json(sensor_config);
        json_new["wifi"] = nlohmann::json(wifi_config);
        status &= write_json_to_flash(json_new);
    }

    return status;
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
        for (const auto &sensor : json["sensors"]) {
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
    auto json_string{json_data.dump()};
    auto status{true};

    if (json_string.length() > MAX_FLASH_STORAGE_SIZE) {
        LogError(("Can't write more than %u!", MAX_FLASH_STORAGE_SIZE));
        return false;
    }
    LogDebug(("Write %u of data", json_string.length()));

    auto number_of_pages =
        json_string.length() / FLASH_PAGE_SIZE + static_cast<uint8_t>(1U);

    status &= pico_interface_.erase_flash(
        ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS), FLASH_SECTOR_SIZE);

    if (status) {
        uint8_t data[FLASH_SECTOR_SIZE]{};
        memcpy(data, json_string.c_str(), json_string.length());
        status &= pico_interface_.store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS), data,
            FLASH_PAGE_SIZE * number_of_pages);
    }

    return status;
}

auto ConfigHandler::read_json_from_flash(nlohmann::json &json_data) -> bool {
    uint8_t data[MAX_FLASH_STORAGE_SIZE];
    auto status{true};
    std::memcpy(data, g_app_storage, sizeof(data));
    json_data =
        nlohmann::json::parse(reinterpret_cast<char *>(data), nullptr, false);

    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from flash"));
        status = false;
    }

    return status;
}
