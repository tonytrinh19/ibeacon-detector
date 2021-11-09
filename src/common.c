#include "common.h"

void error_reporter(const struct dc_error *err) {
    fprintf(stderr, "Error: \"%s\" - %s : %s : %d @ %zu\n", err->message, err->file_name, err->function_name,
            err->errno_code, err->line_number);
}

void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number) {
    fprintf(stderr, "Entering: %s : %s @ %zu\n", file_name, function_name, line_number);
}

void quit_handler(int sig_num)
{
    exit_flag = 1;
}
