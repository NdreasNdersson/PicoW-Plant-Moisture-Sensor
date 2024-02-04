#include <stdarg.h>
#include <stdio.h>

void vLoggingPrintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
