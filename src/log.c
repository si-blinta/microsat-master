#include <stdio.h>
#include <stdarg.h>
#include "log.h"
#define LOG_FILE "log.txt"

// Log function
void log_message(int level, const char* message) {
  const char* level_str;
  switch (level) {
    case LOG_LEVEL_DEBUG: level_str = "DEBUG"; break;
    case LOG_LEVEL_INFO: level_str = "INFO"; break;
    case LOG_LEVEL_WARNING: level_str = "WARNING"; break;
    case LOG_LEVEL_ERROR: level_str = "ERROR"; break;
    default: level_str = "UNKNOWN"; break;
  }
  printf(LOG_FORMAT,level_str, message);
}