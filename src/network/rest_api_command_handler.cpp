#include "network/rest_api_command_handler.h"

#include <cctype>
#include <charconv>
#include <vector>

#include "bootloader_lib.h"
#include "sensors/sensor_config.h"
#include "utils/json_converter.h"
#include "utils/logging.h"

RestApiCommandHandler::RestApiCommandHandler(
    std::vector<std::shared_ptr<Sensor>> sensors)
    : m_config_handler{},
      m_software_download{},
      m_sensors{std::move(sensors)},
      m_rest_api_data{},
      m_bin_sem{xSemaphoreCreateMutex()} {
    for (const auto &sensor : m_sensors) {
        sensor->attach(this);
    }
}

RestApiCommandHandler::~RestApiCommandHandler() {
    for (const auto &sensor : m_sensors) {
        sensor->detach(this);
    }
}

auto RestApiCommandHandler::get_callback(const std::string &resource,
                                         std::string &payload) -> bool {
    auto status{false};
    if ("SENSORS" == resource) {
        if (m_rest_api_data.contains("sensors")) {
            if (xSemaphoreTake(m_bin_sem, 10) == pdTRUE) {
                payload = m_rest_api_data["sensors"].dump();
                xSemaphoreGive(m_bin_sem);

                status = true;
            }
        } else {
            LogError(("No config data available"));
        }

    } else if ("CONFIG" == resource) {
        if (m_rest_api_data.contains("config")) {
            std::vector<sensor_config_t> sensor_config{};
            if (xSemaphoreTake(m_bin_sem, 10) == pdTRUE) {
                for (auto &sensor : m_rest_api_data["config"].items()) {
                    sensor_config.push_back(sensor.value());
                }
                xSemaphoreGive(m_bin_sem);
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
        if (json_data.contains("config") && json_data["config"].is_array()) {
            std::vector<sensor_config_t> config =
                json_data["config"].get<std::vector<sensor_config_t>>();
            if (m_config_handler.write_config(config)) {
                LogInfo(("Sensor config stored, will reboot in 3s..."));
                uint32_t reboot_delay_ms{3000};
                m_software_download.reboot(reboot_delay_ms);
            } else {
                LogError(("Sensor config could not be stored, json_data: \n%s",
                          json_data.dump().c_str()));
            }
        } else {
            status = false;
        }
    } else if (resource == "SWDL") {
        if (json_data.contains("size")) {
            uint32_t size{json_data["size"]};
            LogDebug(("Store app size %u", size));
            m_software_download.init_download(size);
        }
        if (json_data.contains("hash")) {
            unsigned char temp[SHA256_DIGEST_SIZE]{};
            std::string hash{json_data["hash"]};
            auto hash_c_str{hash.c_str()};
            if (hash.size() == (SHA256_DIGEST_SIZE * 2)) {
                for (size_t i{0}; i < hash.size(); i += 2) {
                    std::from_chars(hash_c_str + i, hash_c_str + i + 2,
                                    temp[i / 2], 16);
                }
                LogDebug(("Store app hash %s", hash_c_str));
                m_software_download.set_hash(temp);
            } else {
                status = false;
                LogError(("Received hash has wrong length"));
            }
        }
        if (json_data.contains("binary")) {
            std::string data{json_data["binary"]};
            auto data_c_str{data.c_str()};
            if ((data.size() / 2) <= DOWNLOAD_BLOCK_SIZE) {
                unsigned char download_block[DOWNLOAD_BLOCK_SIZE]{};
                for (size_t i{0}; i < data.size(); i += 2) {
                    std::from_chars(data_c_str + i, data_c_str + i + 2,
                                    download_block[i / 2], 16);
                }
                if (!m_software_download.write_app(download_block)) {
                    status = false;
                    LogError(("SWDL write to swap failed"));
                }
            } else {
                status = false;
                LogError(
                    ("SWDL binary content must be less then (last "
                     "packages) or "
                     "equal size %d",
                     DOWNLOAD_BLOCK_SIZE));
            }
        }
        if (json_data.contains("complete")) {
            LogInfo(("SWDL complete"));
            m_software_download.download_complete();
        }
    } else {
        status = false;
        LogError(("'POST /%s' is not implemented", resource.c_str()));
    }

    return status;
}

// Subscriber
void RestApiCommandHandler::update(const Measurement_t &measurement) {
    if (xSemaphoreTake(m_bin_sem, 10) == pdTRUE) {
        m_rest_api_data["sensors"][measurement.name]["value"] =
            measurement.value;
        m_rest_api_data["sensors"][measurement.name]["raw_value"] =
            measurement.raw_value;
        if (measurement.config.type != "") {
            m_rest_api_data["config"][measurement.name] = measurement.config;
        }
        xSemaphoreGive(m_bin_sem);
    }
}
