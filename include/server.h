//
// Created by toni on 2021-11-08.
//

#ifndef TEMPLATE2_SERVER_H
#define TEMPLATE2_SERVER_H

#include <dc_application/command_line.h>
#include <dc_application/config.h>
#include <dc_application/options.h>
#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/sys/dc_socket.h>
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_ndbm.h>
#include <dc_network/common.h>
#include <dc_network/options.h>
#include <dc_network/server.h>
#include <dc_util/streams.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include "common.h"
#include "database.h"

static volatile sig_atomic_t exit_signal = 0;

void getData(struct dc_posix_env *env,
             struct dc_error *err);

int receive_data(struct dc_posix_env *env,
                 struct dc_error *err,
                 int *contentLength,
                 int fd, size_t size);

void store_data(struct dc_posix_env *env,
                struct dc_error *err,
                char *majorMinor,
                char *location);

struct application_settings {
    struct dc_opt_settings opts;
    struct dc_setting_bool *verbose;
    struct dc_setting_string *hostname;
    struct dc_setting_regex *ip_version;
    struct dc_setting_uint16 *port;
    struct dc_setting_bool *reuse_address;
    struct addrinfo *address;
    int server_socket_fd;
};

struct dc_application_settings *create_settings(const struct dc_posix_env *env,
                                                struct dc_error *err);

int
destroy_settings(const struct dc_posix_env *env,
                 struct dc_error *err,
                 struct dc_application_settings **psettings);

int run(const struct dc_posix_env *env,
        struct dc_error *err,
        struct dc_application_settings *settings);

void signal_handler(int signnum);

void do_create_settings(const struct dc_posix_env *env,
                        struct dc_error *err,
                        void *arg);

void do_create_socket(const struct dc_posix_env *env,
                      struct dc_error *err,
                      void *arg);

void do_set_sockopts(const struct dc_posix_env *env,
                     struct dc_error *err,
                     void *arg);

void do_bind(const struct dc_posix_env *env,
             struct dc_error *err,
             void *arg);

void do_listen(const struct dc_posix_env *env,
               struct dc_error *err,
               void *arg);

void do_setup(const struct dc_posix_env *env,
              struct dc_error *err,
              void *arg);

bool do_accept(struct dc_posix_env *env,
               struct dc_error *err,
               int *client_socket_fd,
               void *arg);

void do_shutdown(const struct dc_posix_env *env,
                 struct dc_error *err,
                 void *arg);

void do_destroy_settings(const struct dc_posix_env *env,
                         struct dc_error *err,
                         void *arg);

#endif //TEMPLATE2_SERVER_H
