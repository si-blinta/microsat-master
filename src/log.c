#include "log.h"
#include <stdio.h>
void log_error(char *error)
{
    printf(ANSI_COLOR_RED"[error] %s\n" ANSI_COLOR_RESET, error);
}

void log_info(char *info)
{
    printf("[info] %s\n", info);
}
void log_debug(char *debug_msg)
{
    printf(ANSI_COLOR_YELLOW "[debug] %s\n" ANSI_COLOR_RESET, debug_msg);
}

void log_result(char *result_msg)
{
    printf(ANSI_COLOR_GREEN "[result] %s\n" ANSI_COLOR_RESET, result_msg);
}
