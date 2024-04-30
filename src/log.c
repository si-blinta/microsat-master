#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"
#include <stdlib.h>
// Log function
void log_message(int level, const char* format, ...) {
  const char* level_str;
  switch (level) {
    case LOG_LEVEL_DEBUG: level_str = ANSI_COLOR_CYAN "DEBUG" ANSI_COLOR_RESET; break;
    case LOG_LEVEL_INFO: level_str = ANSI_COLOR_GREEN "INFO" ANSI_COLOR_RESET; break;
    case LOG_LEVEL_WARNING: level_str = ANSI_COLOR_YELLOW "WARNING" ANSI_COLOR_RESET; break;
    case LOG_LEVEL_ERROR: level_str = ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET; break;
    default: level_str = ANSI_COLOR_MAGENTA "UNKNOWN" ANSI_COLOR_RESET; break;
  }

  // Allocate memory for the message
  int len = snprintf(NULL, 0, format, "");
  char* message = malloc(len + 1);

  // Write the formatted message to the buffer
  va_list args;
  va_start(args, format);
  vsprintf(message, format, args);
  va_end(args);

  // Get the current time
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  // Print the message with the current time and log level
  printf("[%02d:%02d:%02d] [%s] %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, level_str, message);

  // Free the memory allocated for the message
  free(message);
}
