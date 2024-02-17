#ifndef __UTILS__JSON_HANDLER__
#define __UTILS__JSON_HANDLER__

extern "C" {
#include "tiny-json.h"
};

#define MAX_FIELDS 8

class JsonHandler {
   public:
    JsonHandler();
    ~JsonHandler() = default;

    bool parse_json(char *str);
    char const *get_value(const char *field_name);

   private:
    json_t m_pool[MAX_FIELDS];
    json_t const *m_parent;
};

#endif
