#ifndef LOG_H
#define LOG_H
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define D "[DPU]"
#define H "[HOST]"
void log_error(char *error);
void log_info(char *info);
void log_debug(char *debug_msg);
void log_result(char *result_msg);

#endif // LOG_H