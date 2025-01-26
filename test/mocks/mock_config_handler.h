#include <gmock/gmock.h>

#include <vector>

#include "utils/config_handler/config_handler.h"
#include "utils/config_handler/configs/mqtt_config.h"
#include "utils/config_handler/configs/sensor_config.h"
#include "utils/config_handler/configs/wifi_config.h"

class MockConfigHandler : public ConfigHandler {
   public:
    MOCK_METHOD(bool, init, (), (override));
    MOCK_METHOD(bool, write_config,
                (const std::vector<sensor_config_t> &config), (override));
    MOCK_METHOD(bool, write_config, (const wifi_config_t &config), (override));
    MOCK_METHOD(bool, write_config, (const mqtt_config_t &config), (override));
    MOCK_METHOD(bool, read_config, (std::vector<sensor_config_t> & config),
                (override));
    MOCK_METHOD(bool, read_config, (wifi_config_t & config), (override));
    MOCK_METHOD(bool, read_config, (mqtt_config_t & config), (override));
};
