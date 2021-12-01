#ifndef MORSE_COMMON_H
#define MORSE_COMMON_H

#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/sys/dc_socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static volatile sig_atomic_t exit_flag;

void error_reporter(const struct dc_error *err);

void trace_reporter(__attribute__((unused)) const struct dc_posix_env *env,
                           const char *file_name,
                           const char *function_name,
                           size_t line_number);

void quit_handler(int sig_num);

#endif // MORSE_COMMON_H
