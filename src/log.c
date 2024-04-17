#include <stdio.h>
#include <stdarg.h>
#include "log.h"
#include "time.h"
#define LOG_FILE "log.txt"

// Log function
void log_message(int level, const char* format, ...) {
    time_t t = time(NULL);
  struct tm* tm = localtime(&t);
  char time_str[64];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm);
  // Get the log level string
  const char* level_str;
  switch (level) {
    case LOG_LEVEL_DEBUG: level_str = "DEBUG"; break;
    case LOG_LEVEL_INFO: level_str = "INFO"; break;
    case LOG_LEVEL_WARNING: level_str = "WARNING"; break;
    case LOG_LEVEL_ERROR: level_str = "ERROR"; break;
    default: level_str = "UNKNOWN"; break;
  }

  // Format the log message
  va_list args;
  va_start(args, format);
  char message[1024];
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  printf(LOG_FORMAT, time_str, level_str, message);
}