//
// Created by toni on 2021-11-08.
//

#include "database.h"

int store(const struct dc_posix_env *env,
      const struct dc_error *err,
      DBM *db, const char *majorMinor,
      const char *location,
      int type)
{
    int ret_val;
    datum key   = {(void *) majorMinor, (int) strlen(majorMinor) + 1};
    datum value = {(void *) location, (int) strlen(location) + 1};
    dc_dbm_store(env, err, db, key, value, type);

    return ret_val;
}


datum fetch(const struct dc_posix_env *env,
            const struct dc_error *err, DBM *db,
            const char *majorMinor)
{
    datum key = {(void *) majorMinor, (int) strlen(majorMinor) + 1};
    datum content;

    content = dc_dbm_fetch(env, err, db, key);

    return content;

}


void display(const char *majorMinor,
             datum *content)
{
    if (content->dsize > 0)
    {
        printf("%s, %s\n", majorMinor, (char *) content->dptr);
    } else
    {
        printf("%s: Not found\n");
    }
}