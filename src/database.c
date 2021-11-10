//
// Created by toni on 2021-11-08.
//

#include "database.h"

int
store(const struct dc_posix_env *env,
      const struct dc_error *err,
      DBM *db, const char *name,
      const char *phone_number,
      int type) {
    int ret_val;

    datum key = {(void *) name, strlen(name) + 1};
    datum value = {(void *) phone_number, strlen(phone_number) + 1};
    dc_dbm_store(env, err, db, key, value, type);

    return ret_val;
}


datum fetch(const struct dc_posix_env *env,
            const struct dc_error *err, DBM *db,
            const char *name) {

    datum key = {(void *) name, strlen(name) + 1};
    datum content;

    content = dc_dbm_fetch(env, err, db, key);

    return content;

}


void display(const char *name,
             datum *content) {

    if (content->dsize > 0) {
        // printf("%s, %p, %zu\n", name, content->dptr, content->dsize);
        printf("%s, %s\n", name, (char *) content->dptr);
    } else {
        printf("%s: Not found\n");
    }
}
