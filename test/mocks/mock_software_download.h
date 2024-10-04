#include <gmock/gmock.h>

#include <cstdint>

#include "software_download_api.h"

class MockSoftwareDownload : public PicoBootloader::SoftwareDownloadApi {
   public:
    MOCK_METHOD(bool, init_download, (const uint32_t &size), ());
    MOCK_METHOD(
        bool, set_hash,
        (const unsigned char app_hash[PicoBootloader::SHA256_DIGEST_SIZE]),
        (const));
    MOCK_METHOD(bool, write_app,
                (const unsigned char binary_block[FLASH_PAGE_SIZE]), ());
    MOCK_METHOD(bool, download_complete, (), (const));
    MOCK_METHOD(void, reboot, (uint32_t delay_ms), (const));
    MOCK_METHOD(bool, restore, (uint32_t delay_ms), (const));
};
