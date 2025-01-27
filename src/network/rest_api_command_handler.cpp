#include "network/rest_api_command_handler.h"

#include <cctype>
#include <charconv>
#include <memory>
#include <vector>

#include "types.h"
#include "utils/config_handler/config_handler.h"
#include "utils/config_handler/configs/mqtt_config.h"
#include "utils/config_handler/configs/sensor_config.h"
#include "utils/json_converter.h"
#include "utils/logging.h"

RestApiCommandHandler::RestApiCommandHandler(
    ConfigHandler &config_handler,
    PicoBootloader::SoftwareDownloadApi &software_download,
    FreertosInterface &freertos_interface,
    std::vector<std::shared_ptr<Sensor>> sensors)
    : config_handler_{config_handler},
      software_download_{software_download},
      freertos_interface_{freertos_interface},
      sensors_{std::move(sensors)},
      rest_api_data_{},
      semaphore_{freertos_interface_.semaphore_create_mutex()} {
    for (const auto &sensor : sensors_) {
        sensor->attach(this);
    }
}

RestApiCommandHandler::~RestApiCommandHandler() {
    for (const auto &sensor : sensors_) {
        sensor->detach(this);
    }
}

auto RestApiCommandHandler::get_callback(const std::string &resource,
                                         std::string &payload) -> bool {
    auto status{false};
    if ("SENSORS" == resource) {
        if (rest_api_data_.contains("sensors")) {
            if (freertos_interface_.semaphore_take(semaphore_, 10) == pdTRUE) {
                payload = rest_api_data_["sensors"].dump();
                freertos_interface_.semaphore_give(semaphore_);

                status = true;
            }
        } else {
            LogError(("No config data available"));
        }

    } else if ("CONFIG" == resource) {
        if (rest_api_data_.contains("config")) {
            std::vector<sensor_config_t> sensor_config{};
            if (freertos_interface_.semaphore_take(semaphore_, 10) == pdTRUE) {
                for (auto &sensor : rest_api_data_["config"].items()) {
                    sensor_config.push_back(sensor.value());
                }
                freertos_interface_.semaphore_give(semaphore_);
                nlohmann::json json{};
                json["config"] = sensor_config;
                payload = json.dump();
                status = true;
            }
        } else {
            LogError(("No config data available"));
        }
    } else {
        LogError(("'GET /%s' is not implemented", resource.c_str()));
    }
    return status;
}
auto RestApiCommandHandler::post_callback(const std::string &resource,
                                          const std::string &payload) -> bool {
    auto json_data = nlohmann::json::parse(payload, nullptr, false);
    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from received data"));
        return false;
    }

    auto status{true};
    if (resource == "SENSORS") {
        status &= validate_sensors_content(json_data);
        if (status) {
            std::vector<sensor_config_t> config =
                json_data["config"].get<std::vector<sensor_config_t>>();
            if (config_handler_.write_config(config)) {
                LogInfo(("Sensor config stored, will reboot in 3s..."));
                uint32_t reboot_delay_ms{3000};
                software_download_.reboot(reboot_delay_ms);
            } else {
                status = false;
                LogError(("Sensor config could not be stored, json_data: \n%s",
                          json_data.dump().c_str()));
            }
        }
    } else if (resource == "MQTT") {
        status &= validate_mqtt_content(json_data);
        if (status) {
            auto config{json_data.get<mqtt_config_t>()};
            if (config_handler_.write_config(config)) {
                LogInfo(("MQTT config stored, will reboot in 3s..."));
                uint32_t reboot_delay_ms{3000};
                software_download_.reboot(reboot_delay_ms);
            } else {
                status = false;
                LogError(("MQTT config could not be stored, json_data: \n%s",
                          json_data.dump().c_str()));
            }
        }
    } else if (resource == "SWDL") {
        status &= validate_swdl_content(json_data);
        if (status && json_data.contains("size")) {
            uint32_t size{json_data["size"]};
            LogDebug(("Store app size %u", size));
            status &= software_download_.init_download(size);
        }
        if (status && json_data.contains("hash")) {
            unsigned char temp[PicoBootloader::SHA256_DIGEST_SIZE]{};
            std::string hash{json_data["hash"]};
            auto hash_c_str{hash.c_str()};
            for (size_t i{0}; i < hash.size(); i += 2) {
                std::from_chars(hash_c_str + i, hash_c_str + i + 2, temp[i / 2],
                                16);
            }
            LogDebug(("Store app hash %s", hash_c_str));
            status &= software_download_.set_hash(temp);
        }
        if (status && json_data.contains("binary")) {
            std::string data{json_data["binary"]};
            auto data_c_str{data.c_str()};
            unsigned char download_block[PicoBootloader::DOWNLOAD_BLOCK_SIZE]{};
            for (size_t i{0}; i < data.size(); i += 2) {
                std::from_chars(data_c_str + i, data_c_str + i + 2,
                                download_block[i / 2], 16);
            }
            if (!software_download_.write_app(download_block)) {
                status = false;
                LogError(("SWDL write to swap failed"));
            }
        }
        if (status && json_data.contains("complete")) {
            LogInfo(("SWDL complete"));
            status &= software_download_.download_complete();
        }
    } else {
        status = false;
        LogError(("'POST /%s' is not implemented", resource.c_str()));
    }

    return status;
}

// Subscriber
void RestApiCommandHandler::update(const Measurement_t &measurement) {
    if (freertos_interface_.semaphore_take(semaphore_, 10) == pdTRUE) {
        rest_api_data_["sensors"][measurement.name]["value"] =
            measurement.value;
        rest_api_data_["sensors"][measurement.name]["raw_value"] =
            measurement.raw_value;
        if (measurement.config.type != "") {
            rest_api_data_["config"][measurement.name] = measurement.config;
        }
        freertos_interface_.semaphore_give(semaphore_);
    }
}

auto RestApiCommandHandler::validate_sensors_content(
    const nlohmann::json &json_data) -> bool {
    return json_data.contains("config") && json_data["config"].is_array();
}

auto RestApiCommandHandler::validate_mqtt_content(
    const nlohmann::json &json_data) -> bool {
    auto status{true};
    status &= (json_data.contains("user") && json_data["user"].is_string());
    status &=
        (json_data.contains("password") && json_data["password"].is_string());
    status &= (json_data.contains("ip") && json_data["ip"].is_string());
    status &=
        (json_data.contains("port") && json_data["port"].is_number_integer());
    status &= (json_data.contains("client_name") &&
               json_data["client_name"].is_string());
    return status;
}

auto RestApiCommandHandler::validate_swdl_content(
    const nlohmann::json &json_data) -> bool {
    auto status{true};

    if (json_data.contains("size") && !json_data["size"].is_number_integer()) {
        status = false;
        LogError(("SWDL size must be a number"));
    }

    if (json_data.contains("hash") && !json_data["hash"].is_string()) {
        status = false;
        LogError(("SWDL hash must be a string"));
    }

    if (json_data.contains("hash") && json_data["hash"].is_string() &&
        (std::string{json_data["hash"]}.size() !=
         (PicoBootloader::SHA256_DIGEST_SIZE * 2))) {
        status = false;
        LogError(("SWDL hash was too large"));
    }

    if (json_data.contains("binary") && !json_data["binary"].is_string()) {
        status = false;
        LogError(("SWDL binary must be a string"));
    }

    if (json_data.contains("binary") && json_data["binary"].is_string() &&
        (std::string{json_data["binary"]}.size() >
         (PicoBootloader::DOWNLOAD_BLOCK_SIZE * 2))) {
        status = false;
        LogError(("SWDL binary was too large"));
    }

    return status;
}
