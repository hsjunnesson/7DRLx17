#pragma once

enum LOG_SEVERITY {
    DEBUG,
    INFO,
    ERROR,
};

void internal_log(LOG_SEVERITY severity, const char* format, ...);

#define log_debug(format, ...) internal_log(LOG_SEVERITY::DEBUG, format, __VA_ARGS__);
#define log_info(format, ...) internal_log(LOG_SEVERITY::INFO, format, __VA_ARGS__);
#define log_error(format, ...) internal_log(LOG_SEVERITY::ERROR, format, __VA_ARGS__);
