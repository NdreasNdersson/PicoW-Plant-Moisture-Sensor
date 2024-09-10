#include "network/rest_api_command_handler.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <vector>

#include "bootloader_lib.h"
#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"
#include "utils/json_converter.h"
#include "utils/logging.h"

RestApiCommandHandler::RestApiCommandHandler(std::vector<Ads1115Adc> &sensors)
    : m_config_handler{},
      m_software_download{},
      m_sensors{std::move(sensors)} {}

auto RestApiCommandHandler::get_callback(const std::string &resource,
                                         std::string &payload) -> bool {
    auto status{false};
    if ("SENSORS" == resource) {
        nlohmann::json rest_api_data;
        auto save_config{false};
        std::vector<sensor_config_t> sensor_config(m_sensors.size());
        for (size_t i{0}; i < m_sensors.size(); i++) {
            std::string name{};
            m_sensors[i].get_name(name);
            if (m_sensors[i].read() == SensorReadStatus::Calibrating) {
                m_sensors[i].get_config(sensor_config[i]);
                save_config = true;
            }

            rest_api_data[name]["value"] = m_sensors[i].get_value();
            rest_api_data[name]["raw_value"] = m_sensors[i].get_raw_value();
        }
        payload = rest_api_data.dump();
        status = true;

        if (save_config) {
            m_config_handler.write_config(sensor_config);
        }
    } else if ("CONFIG" == resource) {
        std::vector<sensor_config_t> sensor_config(m_sensors.size());
        for (size_t i{0}; i < m_sensors.size(); i++) {
            m_sensors[i].get_config(sensor_config[i]);
        }
        payload = nlohmann::json{sensor_config}.dump();
        status = true;
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
            m_config_handler.write_config(config);
            LogDebug(("Sensor config stored, will reboot in 1s..."));
            uint32_t reboot_delay_ms{1000};
            m_software_download.reboot(reboot_delay_ms);
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
                    ("SWDL binary content must be less then (last packages) or "
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
