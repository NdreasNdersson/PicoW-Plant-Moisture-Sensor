#ifndef __UTILS__CONFIG_HANDLER__
#define __UTILS__CONFIG_HANDLER__

#include <string>

extern "C" {
#include <hardware/flash.h>
};

class ConfigHandler {
    using page_t = uint8_t[FLASH_PAGE_SIZE];

   public:
    ConfigHandler();
    ~ConfigHandler() = default;

    std::string get_config_value(std::string config_name);

   private:
    void read(page_t data);
    void write(page_t data);
};

#endif
