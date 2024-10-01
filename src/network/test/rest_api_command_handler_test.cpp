#include "network/rest_api_command_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <string>

#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mock_config_handler.h"
#include "mocks/mock_pico_interface.h"

class RestApiCommandHandlerTest : public testing::Test {
   protected:
    RestApiCommandHandlerTest()
        : mock_config_handler_{}, uut_{mock_config_handler_} {}

    testing::StrictMock<MockConfigHandler> mock_config_handler_;
    RestApiCommandHandler uut_;
};

TEST_F(RestApiCommandHandlerTest, TestGetCallback) {}
