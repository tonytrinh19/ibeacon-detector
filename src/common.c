#include "common.h"

void error_reporter(const struct dc_error *err)
{
    if (err->type == DC_ERROR_ERRNO) {
        fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number,
                err->errno_code);
    } else {
        fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number,
                err->err_code);
    }

    fprintf(stderr, "ERROR: %s\n", err->message);
}

void trace_reporter(__attribute__((unused)) const struct dc_posix_env *env,
                    const char *file_name,
                    const char *function_name,
                    size_t line_number)
{
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}

void quit_handler(int sig_num)
{
    exit_flag = 1;
}
