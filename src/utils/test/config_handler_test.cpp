#include "config_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <string>

#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mocks/mock_pico_interface.h"
#include "nlohmann/json_fwd.hpp"

class ConfigHandlerTest : public testing::Test {
   protected:
    ConfigHandlerTest() : mock_pico_interface_{}, uut_{mock_pico_interface_} {}

    testing::StrictMock<MockPicoInterface> mock_pico_interface_;
    ConfigHandler uut_;
};

TEST_F(ConfigHandlerTest, TestInit) {
    std::string valid_json{"{\"valid\": \"json\"}"};
    {
        std::memcpy(g_app_storage, valid_json.c_str(), valid_json.length());
        EXPECT_TRUE(uut_.init());
    }

    {
        std::memset(g_app_storage, 0, FLASH_SECTOR_SIZE);

        EXPECT_CALL(
            mock_pico_interface_,
            erase_flash(ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS),
                        FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.init());
    }

    {
        EXPECT_CALL(
            mock_pico_interface_,
            erase_flash(ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS),
                        FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(
            mock_pico_interface_,
            store_to_flash(ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS),
                           testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.init());
    }

    {
        nlohmann::json actual_data;
        auto copy_write_data =
            [&actual_data](uint32_t arg0, const uint8_t *arg1, size_t arg2) {
                actual_data = nlohmann::json::parse(arg1);
            };
        EXPECT_CALL(
            mock_pico_interface_,
            erase_flash(ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS),
                        FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(
            mock_pico_interface_,
            store_to_flash(ADDR_WITH_XIP_OFFSET_AS_U32(APP_STORAGE_ADDRESS),
                           testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::DoAll(testing::Invoke(copy_write_data),
                                     testing::Return(true)));
        EXPECT_TRUE(uut_.init());
        EXPECT_EQ(
            actual_data.dump(),
            "{\"sensors\":[],\"wifi\":{\"password\":\"\",\"ssid\":\"\"}}");
    }
}

TEST_F(ConfigHandlerTest, TestWriteConfig) {}
