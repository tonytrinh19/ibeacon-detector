#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/sys/dc_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

int main(void) {
    dc_error_reporter reporter;
    dc_posix_tracer tracer;
    struct dc_error err;
    struct dc_posix_env env;
    const char *host_name;
    struct addrinfo hints;
    struct addrinfo *result;

    reporter = error_reporter;
    tracer = trace_reporter;
    tracer = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);

    host_name = "10.0.0.168";
    dc_memset(&env, &hints, 0, sizeof(hints));
    hints.ai_family = PF_INET; // PF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    dc_getaddrinfo(&env, &err, host_name, NULL, &hints, &result);

    if (dc_error_has_no_error(&err)) {
        int socket_fd;

        socket_fd = dc_socket(&env, &err, result->ai_family, result->ai_socktype, result->ai_protocol);

        if (dc_error_has_no_error(&err)) {
            struct sockaddr *sockaddr;
            in_port_t port;
            in_port_t converted_port;
            socklen_t sockaddr_size;

            sockaddr = result->ai_addr;
            port = 1235;
            converted_port = htons(port);

            if (sockaddr->sa_family == AF_INET) {
                struct sockaddr_in *addr_in;

                addr_in = (struct sockaddr_in *) sockaddr;
                addr_in->sin_port = converted_port;
                sockaddr_size = sizeof(struct sockaddr_in);
            } else {
                if (sockaddr->sa_family == AF_INET6) {
                    struct sockaddr_in6 *addr_in;

                    addr_in = (struct sockaddr_in6 *) sockaddr;
                    addr_in->sin6_port = converted_port;
                    sockaddr_size = sizeof(struct sockaddr_in6);
                } else {
                    DC_ERROR_RAISE_USER(&err, "sockaddr->sa_family is invalid", -1);
                    sockaddr_size = 0;
                }
            }

            if (dc_error_has_no_error(&err)) {
                dc_connect(&env, &err, socket_fd, sockaddr, sockaddr_size);

                if (dc_error_has_no_error(&err)) {
                    struct sigaction old_action;

                    dc_sigaction(&env, &err, SIGINT, NULL, &old_action);

                    if (old_action.sa_handler != SIG_IGN) {
                        struct sigaction new_action;
                        char data[1024] = {0};

                        exit_flag = 0;
                        new_action.sa_handler = quit_handler;
                        sigemptyset(&new_action.sa_mask);
                        new_action.sa_flags = 0;
                        dc_sigaction(&env, &err, SIGINT, &new_action, NULL);

                        while (dc_read(&env, &err, STDIN_FILENO, data, 1024) > 0 && dc_error_has_no_error(&err)) {
                            data[strlen(data)] = '\0';
                            printf("READ %s\n", data);
                            dc_write(&env, &err, socket_fd, data, strlen(data) + 1);
                            memset(data, '\0', strlen(data) + 1);
                        }
                    }
                }
            }
        }

        if (dc_error_has_no_error(&err)) {
            dc_close(&env, &err, socket_fd);
        }
    }

    return EXIT_SUCCESS;
}
