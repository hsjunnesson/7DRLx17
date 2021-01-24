#pragma once

/**
 * @brief Logs an SDL error
 *
 * @param message The message to prepend the log with.
 */
void logSDLError(const char* message);

/**
 * @brief Quits with a format string.
 */
void quit_fmt(const char* fmt, ...);
