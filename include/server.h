//
// Created by toni on 2021-11-08.
//

#ifndef TEMPLATE2_SERVER_H
#define TEMPLATE2_SERVER_H
#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/sys/dc_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size);

#endif //TEMPLATE2_SERVER_H
