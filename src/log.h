#pragma once

enum class LOGGING_SEVERITY {
    DEBUG,
    INFO,
    ERR,
    FATAL,
};

void internal_log(LOGGING_SEVERITY severity, const char *format, ...);

#define log_debug(format, ...) internal_log(LOGGING_SEVERITY::DEBUG, format, __VA_ARGS__);
#define log_info(format, ...) internal_log(LOGGING_SEVERITY::INFO, format, __VA_ARGS__);
#define log_error(format, ...) internal_log(LOGGING_SEVERITY::ERR, format, __VA_ARGS__);
#define log_fatal(format, ...) internal_log(LOGGING_SEVERITY::FATAL, format, __VA_ARGS__);
