//
// Created by toni on 2021-11-14.
//

#ifndef TEMPLATE2_CLIENT_H
#define TEMPLATE2_CLIENT_H

#include <assert.h>
#include <dc_application/command_line.h>
#include <dc_application/config.h>
#include <dc_application/options.h>
#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/sys/dc_socket.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

#define DEFAULT_ECHO_PORT 2007

int run(const struct dc_posix_env *env,
        struct dc_error *err,
        struct dc_application_settings *settings);

struct application_settings {
    struct dc_opt_settings opts;
    struct dc_setting_bool *verbose;
    struct dc_setting_string *hostname;
    struct dc_setting_regex *ip_version;
    struct dc_setting_string *message;
    struct dc_setting_uint16 *port;
};


struct dc_application_settings *create_settings(const struct dc_posix_env *env,
                                                struct dc_error *err);

int
destroy_settings(const struct dc_posix_env *env,
                 struct dc_error *err,
                 struct dc_application_settings **psettings);

#endif //TEMPLATE2_CLIENT_H
