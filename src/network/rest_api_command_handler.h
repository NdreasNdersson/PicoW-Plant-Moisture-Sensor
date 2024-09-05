#ifndef __NETWORK__REST_API_COMMAND_HANDLER__
#define __NETWORK__REST_API_COMMAND_HANDLER__

#include <string>

#include "bootloader_lib.h"
#include "utils/config_handler.h"

class RestApiCommandHandler {
   public:
    auto get_callback(const std::string &resource, const std::string &payload)
        -> bool;
    auto post_callback(const std::string &resource, const std::string &payload)
        -> bool;

   private:
    ConfigHandler m_config_handler{};
    SoftwareDownload m_software_download{};
    unsigned char m_download_block[DOWNLOAD_BLOCK_SIZE]{};
    uint16_t m_download_block_iterator{};
};

#endif
