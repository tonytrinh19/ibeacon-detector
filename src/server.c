#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/sys/dc_socket.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"
#include "common.h"
#include "database.h"

#define testdb "TESTDB"

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

    host_name = "localhost";
    dc_memset(&env, &hints, 0, sizeof(hints));
    hints.ai_family = PF_INET; // PF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    dc_getaddrinfo(&env, &err, host_name, NULL, &hints, &result);

    if (dc_error_has_no_error(&err)) {
        int server_socket_fd;

        server_socket_fd = dc_socket(&env, &err, result->ai_family, result->ai_socktype, result->ai_protocol);

        if (dc_error_has_no_error(&err)) {
            struct sockaddr *sockaddr;
            in_port_t port;
            in_port_t converted_port;
            socklen_t sockaddr_size;

            sockaddr = result->ai_addr;
            port = 1237;
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
                dc_bind(&env, &err, server_socket_fd, sockaddr, sockaddr_size);

                if (dc_error_has_no_error(&err)) {
                    int backlog;

                    backlog = 5;
                    dc_listen(&env, &err, server_socket_fd, backlog);

                    if (dc_error_has_no_error(&err)) {
                        struct sigaction old_action;

                        dc_sigaction(&env, &err, SIGINT, NULL, &old_action);

                        if (old_action.sa_handler != SIG_IGN) {
                            struct sigaction new_action;

                            exit_flag = 0;
                            new_action.sa_handler = quit_handler;
                            sigemptyset(&new_action.sa_mask);
                            new_action.sa_flags = 0;
                            dc_sigaction(&env, &err, SIGINT, &new_action, NULL);
                            while (!(exit_flag) && dc_error_has_no_error(&err)) {
                                int client_socket_fd;

                                client_socket_fd = dc_accept(&env, &err, server_socket_fd, NULL, NULL);

                                if (dc_error_has_no_error(&err)) {
                                    receive_data(&env, &err, client_socket_fd, 1024);
                                    dc_close(&env, &err, client_socket_fd);
                                } else {
                                    if (err.type == DC_ERROR_ERRNO && err.errno_code == EINTR) {
                                        dc_error_reset(&err);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (dc_error_has_no_error(&err)) {
            dc_close(&env, &err, server_socket_fd);
        }
    }

    return EXIT_SUCCESS;
}

// Look at the code in the client, you could do the same thing
void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size) {
    // more efficient would be to allocate the buffer in the caller (main) so we don't have to keep
    // mallocing and freeing the same data over and over again.
    char *data;
    ssize_t count;

    data = dc_malloc(env, err, size);

    while (!(exit_flag) && (count = dc_read(env, err, fd, data, size)) > 0 && dc_error_has_no_error(err)) {

        store_data(env, err, data);
        dc_write(env, err, STDOUT_FILENO, data, (size_t) count);
        memset(data, '\0', strlen(data));
    }
    dc_free(env, data, size);
}

void store_data(struct dc_posix_env *env, struct dc_error *err, char *data) {
    DBM *db = dc_dbm_open(env, err, testdb, DC_O_RDWR | DC_O_CREAT, 0600);
    if (dc_error_has_no_error(err)) {
        store(env, err, db, data, "value bro bro", DBM_REPLACE);
        if (dc_error_has_error(err)) {
            if (err->type == DC_ERROR_ERRNO && err->errno_code == EINTR) {
                dc_error_reset(err);
            }
        } else {
            datum content;
            content = fetch(env, err, db, data);
            //content = fetch(&env, &err, db, "Foo");
            display(data, &content);
        }
    }
    dc_dbm_close(env, err, db);
}

