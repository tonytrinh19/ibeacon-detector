//
// Created by toni on 2021-11-08.
//

#ifndef TEMPLATE2_SERVER_H
#define TEMPLATE2_SERVER_H

#include <stdlib.h>

void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size);

#endif //TEMPLATE2_SERVER_H