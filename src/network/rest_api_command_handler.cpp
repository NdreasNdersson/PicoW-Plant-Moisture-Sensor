#include "network/rest_api_command_handler.h"

#include <charconv>
#include <vector>

#include "nlohmann/json.hpp"
#include "sensors/sensor_config.h"
#include "utils/json_converter.h"
#include "utils/logging.h"

auto RestApiCommandHandler::get_callback(const std::string &resource,
                                         const std::string &payload) -> bool {
    return true;
}
auto RestApiCommandHandler::post_callback(const std::string &resource,
                                          const std::string &payload) -> bool {
    auto json_data = nlohmann::json::parse(payload, nullptr, false);
    if (json_data.is_discarded()) {
        LogError(("Failed to parse json from received data"));
        return false;
    }

    auto status{false};
    // ToDo make case insensitive
    if (resource == "sensors") {
        if (json_data.contains("sensors") && json_data["sensors"].is_array()) {
            std::vector<sensor_config_t> config =
                json_data["sensors"].get<std::vector<sensor_config_t>>();
            m_config_handler.write_config(config);
            status = true;
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
                for (uint8_t i{0}; i < hash.size(); i += 2) {
                    std::from_chars(hash_c_str + i, hash_c_str + i + 2,
                                    temp[i / 2], 16);
                }
                LogDebug(("Store app hash %s", hash_c_str));
                m_software_download.set_hash(temp);
            } else {
                LogError(("Received hash has wrong length"));
            }
        }
        if (json_data.contains("binary")) {
            std::string data{json_data["binary"]};
            auto data_c_str{data.c_str()};
            if ((data.size() / 2) <= DOWNLOAD_BLOCK_SIZE) {
                unsigned char download_block[DOWNLOAD_BLOCK_SIZE]{};
                for (uint16_t i{0}; i < data.size(); i += 2) {
                    std::from_chars(data_c_str + i, data_c_str + i + 2,
                                    download_block[i / 2], 16);
                }
                if (!m_software_download.write_app(download_block)) {
                    LogError(("SWDL write to swap failed"));
                }
            } else {
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
        LogError(("Resource %s is not implemented", resource.c_str()));
    }

    return true;
}
