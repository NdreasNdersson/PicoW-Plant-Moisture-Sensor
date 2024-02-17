#include "json_handler.h"

#include "logging.h"

JsonHandler::JsonHandler() : m_pool{}, m_parent{} {}

bool JsonHandler::parse_json(char *str) {
    m_parent = json_create(str, m_pool, MAX_FIELDS);
    if (m_parent == NULL) {
        LogError(("Could not parse json\n"));
        return false;
    }
    return true;
}

char const *JsonHandler::get_value(const char *field_name) {
    json_t const *namefield = json_getProperty(m_parent, field_name);
    if (namefield == NULL) {
        LogError(("Could not fetch field %s\n", field_name));
        return NULL;
    }
    if (json_getType(namefield) != JSON_TEXT) {
        LogError(("Field %s not string\n", field_name));
        return NULL;
    }

    return json_getValue(namefield);
}
