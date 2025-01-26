#include "network/rest_api_command_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <memory>
#include <string>

#include "FreeRTOS.h"
#include "gmock/gmock.h"
#include "hardware/flash.h"
#include "linker_definitions.h"
#include "mocks/mock_config_handler.h"
#include "mocks/mock_freertos_interface.h"
#include "mocks/mock_sensor.h"
#include "mocks/mock_software_download.h"
#include "types.h"
#include "utils/config_handler/configs/sensor_config.h"

class RestApiCommandHandlerTest : public testing::Test {
   protected:
    void SetUp() override {
        mock_sensor_1_ = std::make_shared<testing::StrictMock<MockSensor>>();
        mock_sensor_2_ = std::make_shared<testing::StrictMock<MockSensor>>();
        EXPECT_CALL(*mock_sensor_1_, attach(testing::_));
        EXPECT_CALL(*mock_sensor_2_, attach(testing::_));
        std::vector<std::shared_ptr<Sensor>> sensors;
        sensors.push_back(mock_sensor_1_);
        sensors.push_back(mock_sensor_2_);

        uut_ = std::make_unique<RestApiCommandHandler>(
            mock_config_handler_, mock_software_download_,
            mock_freertos_interface_, sensors);
    }

    void TearDown() override {
        EXPECT_CALL(*mock_sensor_1_, detach(testing::_));
        EXPECT_CALL(*mock_sensor_2_, detach(testing::_));
    }

    testing::StrictMock<MockConfigHandler> mock_config_handler_{};
    testing::StrictMock<MockSoftwareDownload> mock_software_download_{};
    testing::NiceMock<MockFreertosInterface> mock_freertos_interface_{};

    std::shared_ptr<testing::StrictMock<MockSensor>> mock_sensor_1_;
    std::shared_ptr<testing::StrictMock<MockSensor>> mock_sensor_2_;

    std::unique_ptr<RestApiCommandHandler> uut_;
};

TEST_F(RestApiCommandHandlerTest, TestGetCallback) {
    std::string payload{};

    EXPECT_FALSE(uut_->get_callback("WRONG", payload));

    EXPECT_FALSE(uut_->get_callback("SENSORS", payload));

    EXPECT_CALL(mock_freertos_interface_,
                semaphore_take(testing::_, testing::_))
        .WillRepeatedly(testing::Return(pdTRUE));

    Measurement_t measurement{"test", 10, 1.0, {}};
    uut_->update(measurement);
    EXPECT_TRUE(uut_->get_callback("SENSORS", payload));
    EXPECT_EQ(payload, "{\"test\":{\"raw_value\":10,\"value\":1.0}}");

    EXPECT_FALSE(uut_->get_callback("CONFIG", payload));

    measurement.config.type = "temp";
    uut_->update(measurement);
    EXPECT_TRUE(uut_->get_callback("CONFIG", payload));
    EXPECT_EQ(payload,
              "{\"config\":[{\"calibrate_max_value\":false,\"calibrate_min_"
              "value\":false,\"inversed\":false,\"max\":0,\"min\":0,\"pin\":0,"
              "\"type\":\"temp\"}]}");
}

TEST_F(RestApiCommandHandlerTest, TestPostCallbackSensors) {
    std::string payload{
        "{\"config\": "
        "[{\"inversed\":true,\"max\":10,\"min\":1,\"pin\":1,\"type\":"
        "\"temp\"}]}"};

    EXPECT_FALSE(uut_->post_callback("WRONG", payload));

    EXPECT_CALL(
        mock_config_handler_,
        write_config(
            testing::Matcher<const std::vector<sensor_config_t>&>(testing::_)))
        .WillOnce(testing::Return(false));
    EXPECT_FALSE(uut_->post_callback("SENSORS", payload));

    EXPECT_CALL(
        mock_config_handler_,
        write_config(
            testing::Matcher<const std::vector<sensor_config_t>&>(testing::_)))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(mock_software_download_, reboot(3000));
    EXPECT_TRUE(uut_->post_callback("SENSORS", payload));
}

TEST_F(RestApiCommandHandlerTest, TestPostCallbackSwdl) {
    // Size
    {
        std::string payload{"{\"size\": \"9999\"}"};

        EXPECT_FALSE(uut_->post_callback("SWDL", payload));

        payload = "{\"size\": 9999}";
        EXPECT_CALL(mock_software_download_, init_download(9999))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_->post_callback("SWDL", payload));

        EXPECT_CALL(mock_software_download_, init_download(9999))
            .WillOnce(testing::Return(true));
        EXPECT_TRUE(uut_->post_callback("SWDL", payload));
    }

    // Hash
    {
        std::string payload =
            "{\"hash\": "
            "\"9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08"
            "\"}";
        unsigned char expected_hash[PicoBootloader::SHA256_DIGEST_SIZE]{
            0x9f, 0x86, 0xd0, 0x81, 0x88, 0x4c, 0x7d, 0x65, 0x9a, 0x2f, 0xea,
            0xa0, 0xc5, 0x5a, 0xd0, 0x15, 0xa3, 0xbf, 0x4f, 0x1b, 0x2b, 0x0b,
            0x82, 0x2c, 0xd1, 0x5d, 0x6c, 0x15, 0xb0, 0xf0, 0x0a, 0x08};
        unsigned char actual_hash[PicoBootloader::SHA256_DIGEST_SIZE]{};
        auto copy_hash = [&actual_hash](auto arg0) {
            std::memcpy(actual_hash, arg0, PicoBootloader::SHA256_DIGEST_SIZE);
        };
        EXPECT_CALL(mock_software_download_, set_hash(testing::_))
            .WillOnce(testing::DoAll(testing::Invoke(copy_hash),
                                     testing::Return(true)));
        EXPECT_TRUE(uut_->post_callback("SWDL", payload));
        for (int i = 0; i < PicoBootloader::SHA256_DIGEST_SIZE; ++i) {
            EXPECT_EQ(actual_hash[i], expected_hash[i])
                << "Hash differ at index " << i;
            break;
        }

        EXPECT_CALL(mock_software_download_, set_hash(testing::_))
            .WillOnce(testing::Return(false));
        EXPECT_FALSE(uut_->post_callback("SWDL", payload));

        payload =
            "{\"hash\": "
            "\"9f86d081884c7d659a2feaa0c55ad\"}";
        EXPECT_FALSE(uut_->post_callback("SWDL", payload));

        payload = "{\"hash\": 986081884}";
        EXPECT_FALSE(uut_->post_callback("SWDL", payload));
    }

    // Binary
    std::string payload =
        "{\"binary\": "
        "\"9f86d081884c7d659a2feaa0c55ad0\"}";
    unsigned char expected_binary[PicoBootloader::DOWNLOAD_BLOCK_SIZE]{
        0x9f, 0x86, 0xd0, 0x81, 0x88, 0x4c, 0x7d, 0x65,
        0x9a, 0x2f, 0xea, 0xa0, 0xc5, 0x5a, 0xd0};
    unsigned char actual_binary[PicoBootloader::DOWNLOAD_BLOCK_SIZE]{};
    auto copy_binary = [&actual_binary](auto arg0) {
        std::memcpy(actual_binary, arg0, PicoBootloader::DOWNLOAD_BLOCK_SIZE);
    };
    EXPECT_CALL(mock_software_download_, write_app(testing::_))
        .WillOnce(testing::DoAll(testing::Invoke(copy_binary),
                                 testing::Return(true)));
    EXPECT_TRUE(uut_->post_callback("SWDL", payload));
    for (int i = 0; i < PicoBootloader::SHA256_DIGEST_SIZE; ++i) {
        EXPECT_EQ(actual_binary[i], expected_binary[i])
            << "Binary differ at index " << i;
        break;
    }

    EXPECT_CALL(mock_software_download_, write_app(testing::_))
        .WillOnce(testing::Return(false));
    EXPECT_FALSE(uut_->post_callback("SWDL", payload));

    payload =
        "{\"binary\": "
        "\"eea8215e98a222f99f818dc61b06c2c89ee87cfe538e21fb25ae2d14eaf5b6974f39"
        "0cd1707e9c63c3c058c182d062b0f6a523e2ac0ed40d9aa9682998dd0f513c95b5a197"
        "5b43153d78f9b131fdf5f9bb46ac1e2e5d90a1fca6b658d2d7dbdb4af92a2af6de8596"
        "945c0175d00724cf4845e23136daf7535cee7121bab869f3f4cba66bb48c27d2ce2aa2"
        "4e6c799b5c3db9fbfaf79e3c922ecb5b99a422e569bd0dabc75706aefcc977b2b3ac3d"
        "ed6adac368c319d952bf0113812a353d758e3cc5f635fcd71c009d86d9737f14befc25"
        "c92ede8974a0636317740b8f70774e73aec9c34ec00ab4463a357f43fa9ec185fdb071"
        "0ab792eddece60fc1e87993c "
        "351d2cf20fe64d27ec0d4b3c633d7885c411afd3042512d78de5477e54b988daf74a25"
        "33d38888df2543c9bc515de889b00f6b52759759d414d844fcde5ed153ccd2f6a65943"
        "3db852bac8c54c2eb280d78446a7643f834c6c608a53676170c1645363cff81a0bf9e0"
        "248e5d7d9e9254578f4408ec0092796b74ffa9c34663fbcca4c78b62bbe54a15a95ea0"
        "53fd7204e3835266c1a842c87920735dd75a791ec66c1419e8c1da9c9e202d5bf263fe"
        "3fa2c681beea9a9ab6a3b7f05c4a75b528a5e1bdb9783467ed6481f8dff7db94d16a72"
        "dd77117f7baaa826db87c6a0d81d2f7536b0fcef0a2dcaea0ce1d0ece73c044fb44f55"
        "f9cbd6396a3d0125b0ada9 "
        "4bbfe44c6e0396633c882c867ebfbe6d65cfdf1f0c65311f9b80c8988df81506788701"
        "d6a6dae3d9203dc805b8eead2efa125242e9c4670bedf91945cc31352b1726acf64abd"
        "ae34bcd3dd3de0d8c1122e82e515ed6e53769d16a9874bda14320cb43d4cb401349ee8"
        "a5bcb2d93462503c579ba0425046a6b9ff746b6d5afb5aa13c192f718fcdd5017f81c7"
        "a6a67bdab94628cb58082bf002e1a126695d4b22b58465e600df3667a87dfe3b87d429"
        "58806e54ce43f237234909b1fff6a0a3c58167306a8517fdd00c168eed544fa0932412"
        "24f7495af909cc21a761c32ab65c1a230de24f727443963df99dcdd7d4c46997ee86a5"
        "bc904d58ecc489aa37105b "
        "41e14d67b4cbbd5bd592648a327b3c9611adac0174485b00a3a9205a8627d1eb4d5085"
        "ce0b273a15edf9cb705127c3e50f1d981b4c493635143c076fadf3f266768168657e17"
        "5267c9bb9e9752f98eb622ed376817979a40c0a10cc8b896050a6e24c49550e15a215e"
        "21618c5c105bd6c0fb99394849cd019fd4b5c1c1aeecbc8e8c50184ac1f13b85459d1f"
        "bdfa029eb2fa1b30226713d4c1f6a4f22963436cf1c6163d823383549e9b9ed310b66d"
        "b1b42ac31736a83a06a5a616af2e24f06af7a692f977f1d641d8a581c1a524641124fc"
        "06923ce3885980008d6b4cd67421efc20c54ef365c14f730524b5de2fe7841c1b11183"
        "6189c03dc12da8220cfa88 "
        "d807413cb77873e84ca89602e8cae3ec7b273f237e6ab457fe38622f88a1da85aab7ae"
        "e28a56e1a16f57570e3d54f8f91200cc2b6649947af8affe0b2df311210d10d32e4d84"
        "5cdb15516b67171aa355a03bbc289718d39abe0bf17837733dba0d8031025281a4907d"
        "ff03a67041bccd35d5aeda0c558954bef1dc423e0859ccc8d7a7101fa7c7eee3633f1a"
        "f2ef96dc4c3541a26d0df45ab688de2aca871009991b4ec8d9f374b2f3fd25bcf421a1"
        "dea888f7c23b0ad948d7d882344beb571aeea13329de43e9b7aa92b34d4f20c7a4de4b"
        "f536551d720e27d7ed541ced7f40c811e2f2c555b6ccddc57de54acaa855b626dff8e3"
        "8b7ac1f89543b371613074aa\"}";
    EXPECT_FALSE(uut_->post_callback("SWDL", payload));
}
