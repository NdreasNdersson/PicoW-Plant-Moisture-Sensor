#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <string>

#include "config_handler_impl.h"
#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mocks/mock_pico_interface.h"
#include "nlohmann/json_fwd.hpp"

class ConfigHandlerTest : public testing::Test {
   protected:
    ConfigHandlerTest() : mock_pico_interface_{}, uut_{mock_pico_interface_} {}

    testing::StrictMock<MockPicoInterface> mock_pico_interface_;
    ConfigHandlerImpl uut_;
};

TEST_F(ConfigHandlerTest, TestInit) {
    std::string valid_json{"{\"valid\": \"json\"}"};
    {
        std::memcpy(PicoBootloader::g_app_storage, valid_json.c_str(),
                    valid_json.length());
        EXPECT_TRUE(uut_.init());
    }

    {
        std::memset(PicoBootloader::g_app_storage, 0, FLASH_SECTOR_SIZE);

        EXPECT_CALL(mock_pico_interface_,
                    erase_flash(ADDR_WITH_XIP_OFFSET_AS_U32(
                                    PicoBootloader::APP_STORAGE_ADDRESS),
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.init());
    }

    {
        EXPECT_CALL(mock_pico_interface_,
                    erase_flash(ADDR_WITH_XIP_OFFSET_AS_U32(
                                    PicoBootloader::APP_STORAGE_ADDRESS),
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface_,
                    store_to_flash(ADDR_WITH_XIP_OFFSET_AS_U32(
                                       PicoBootloader::APP_STORAGE_ADDRESS),
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_.init());
    }

    {
        EXPECT_CALL(mock_pico_interface_,
                    erase_flash(ADDR_WITH_XIP_OFFSET_AS_U32(
                                    PicoBootloader::APP_STORAGE_ADDRESS),
                                FLASH_SECTOR_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_CALL(mock_pico_interface_,
                    store_to_flash(ADDR_WITH_XIP_OFFSET_AS_U32(
                                       PicoBootloader::APP_STORAGE_ADDRESS),
                                   testing::_, FLASH_PAGE_SIZE))
            .WillOnce(testing::Return(true));
        EXPECT_TRUE(uut_.init());
    }
}
