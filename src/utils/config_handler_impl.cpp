#include "config_handler_impl.h"

#include <cstdint>
#include <cstring>

#include "linker_definitions.h"
#include "logging.h"
#include "nlohmann/json.hpp"
#include "utils/config_handler/configs/mqtt_config.h"
#include "utils/json_converter.h"

ConfigHandlerImpl::ConfigHandlerImpl(PicoInterface &pico_interface)
    : pico_interface_{pico_interface} {}

auto ConfigHandlerImpl::init() -> bool {
    auto status{true};
    nlohmann::json json;
    if (!read_json_from_flash(json)) {
        LogInfo(("No previously saved config data, write empty json"));
        nlohmann::json json_new;
        status &= write_json_to_flash(json_new);
    }

    return status;
}

auto ConfigHandlerImpl::write_config(const std::vector<sensor_config_t> &config)
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

auto ConfigHandlerImpl::write_config(const wifi_config_t &config) -> bool {
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

auto ConfigHandlerImpl::write_config(const mqtt_config_t &config) -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        json["mqtt"] = nlohmann::json(config);
        status = write_json_to_flash(json);
    } else {
        nlohmann::json json_new;
        LogInfo(("No previously saved data, write wifi config"));
        json_new["mqtt"] = nlohmann::json(config);
        status = write_json_to_flash(json_new);
    }

    return status;
}

auto ConfigHandlerImpl::read_config(std::vector<sensor_config_t> &config)
    -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        if (json.contains("sensors")) {
            for (const auto &sensor : json["sensors"]) {
                config.emplace_back(sensor.get<sensor_config_t>());
            }
            status = true;
        }
    }

    return status;
}

auto ConfigHandlerImpl::read_config(wifi_config_t &config) -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        if (json.contains("wifi")) {
            config = json["wifi"].get<wifi_config_t>();
            status = true;
        }
    }

    return status;
}

auto ConfigHandlerImpl::read_config(mqtt_config_t &config) -> bool {
    auto status{false};
    nlohmann::json json;
    if (read_json_from_flash(json)) {
        if (json.contains("mqtt")) {
            config = json["mqtt"].get<mqtt_config_t>();
            status = true;
        }
    }

    return status;
}

auto ConfigHandlerImpl::write_json_to_flash(const nlohmann::json &json_data)
    -> bool {
    auto json_string{json_data.dump()};
    auto status{true};

    if (json_string.length() > MAX_FLASH_STORAGE_SIZE) {
        LogError(("Can't write more than %u!", MAX_FLASH_STORAGE_SIZE));
        return false;
    }

    auto number_of_pages =
        (json_string.length() + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    LogDebug(("Write %u bytes of data, %u pages", json_string.length(),
              number_of_pages));

    status &= pico_interface_.erase_flash(
        ADDR_WITH_XIP_OFFSET_AS_U32(PicoBootloader::APP_STORAGE_ADDRESS),
        FLASH_SECTOR_SIZE);

    if (status) {
        uint8_t data[FLASH_SECTOR_SIZE]{};
        std::memcpy(data, json_string.c_str(), json_string.length());
        status &= pico_interface_.store_to_flash(
            ADDR_WITH_XIP_OFFSET_AS_U32(PicoBootloader::APP_STORAGE_ADDRESS),
            data, static_cast<size_t>(FLASH_PAGE_SIZE * number_of_pages));
    }

    return status;
}

auto ConfigHandlerImpl::read_json_from_flash(nlohmann::json &json_data)
    -> bool {
    uint8_t data[MAX_FLASH_STORAGE_SIZE];
    auto status{true};
    std::memcpy(data, PicoBootloader::g_app_storage, sizeof(data));
    json_data =
        nlohmann::json::parse(reinterpret_cast<char *>(data), nullptr, false);

    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from flash"));
        status = false;
    }

    return status;
}
