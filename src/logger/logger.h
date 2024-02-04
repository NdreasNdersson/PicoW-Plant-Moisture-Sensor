#ifndef _LOGGER_LOGGER_
#define _LOGGER_LOGGER_

#include "logging_levels.h"

#include <stdarg.h>
#include <stdio.h>

#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_LEVEL LOG_DEBUG
#endif

#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME "PICO_MQTT"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

#define SdkLog( X ) logger X

void logger(const char *fmt, ...);

#endif
