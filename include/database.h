//
// Created by toni on 2021-11-08.
//

#ifndef TEMPLATE2_DATABASE_H
#define TEMPLATE2_DATABASE_H

#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_ndbm.h>
#include <dc_posix/sys/dc_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int store(const struct dc_posix_env *env, const struct dc_error *err, DBM *db, const char *name, const char *phone_number, int type);
datum fetch(const struct dc_posix_env *env, const struct dc_error *err, DBM *db, const char *name);
void display(const char *name, datum *content);

#endif //TEMPLATE2_DATABASE_H
