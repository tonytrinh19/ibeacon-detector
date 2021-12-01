#include "server.h"
#define DEFAULT_ECHO_PORT 2007
#define INT_MIN 0
#define INT_MAX 10000
#define PATH_404 "../../404.html"
#define PATH_INDEX "../../index.html"
#define PATH_CREATED "../../saved.html"
#define database "DB"
#define ROOT "../.."
int main(int argc, char *argv[])
{
    dc_error_reporter reporter;
    dc_posix_tracer tracer;
    struct dc_posix_env env;
    struct dc_error err;
    struct dc_application_info *info;
    int ret_val;
    struct sigaction sa;

    reporter = error_reporter;
    tracer   = trace_reporter;
    tracer   = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);
    dc_memset(&env, &sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    dc_sigaction(&env, &err, SIGINT, &sa, NULL);
    dc_sigaction(&env, &err, SIGTERM, &sa, NULL);

    info = dc_application_info_create(&env, &err, "HTTP Server Application");
    ret_val = dc_application_run(&env, &err, info, create_settings, destroy_settings, run, dc_default_create_lifecycle,
                                 dc_default_destroy_lifecycle,
                                 "~/.dcecho.conf",
                                 argc, argv);
    dc_application_info_destroy(&env, &info);
    dc_error_reset(&err);

    return ret_val;
}


struct dc_application_settings *create_settings(const struct dc_posix_env *env, struct dc_error *err)
{
    static const bool default_verbose   = false;
    static const char *default_hostname = "localhost";
    static const char *default_ip       = "IPv4";
    static const uint16_t default_port  = DEFAULT_ECHO_PORT;
    static const bool default_reuse     = false;
    struct application_settings *settings;

    settings = dc_malloc(env, err, sizeof(struct application_settings));

    if (settings == NULL)
    {
        return NULL;
    }

    settings->opts.parent.config_path = dc_setting_path_create(env, err);
    settings->verbose = dc_setting_bool_create(env, err);
    settings->hostname = dc_setting_string_create(env, err);
    settings->ip_version = dc_setting_regex_create(env, err, "^IPv[4|6]");
    settings->port = dc_setting_uint16_create(env, err);
    settings->reuse_address = dc_setting_bool_create(env, err);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
    struct options opts[] =
            {
                    {(struct dc_setting *) settings->opts.parent.config_path, dc_options_set_path,   "config",  required_argument, 'c', "CONFIG",        dc_string_from_string, NULL,            dc_string_from_config, NULL},
                    {(struct dc_setting *) settings->verbose,                 dc_options_set_bool,   "verbose", no_argument,       'v', "VERBOSE",       dc_flag_from_string,   "verbose",       dc_flag_from_config,   &default_verbose},
                    {(struct dc_setting *) settings->hostname,                dc_options_set_string, "host",    required_argument, 'h', "HOST",          dc_string_from_string, "host",          dc_string_from_config, default_hostname},
                    {(struct dc_setting *) settings->ip_version,              dc_options_set_regex,  "ip",      required_argument, 'i', "IP",            dc_string_from_string, "ip",            dc_string_from_config, default_ip},
                    {(struct dc_setting *) settings->port,                    dc_options_set_uint16, "port",    required_argument, 'p', "PORT",          dc_uint16_from_string, "port",          dc_uint16_from_config, &default_port},
                    {(struct dc_setting *) settings->reuse_address,           dc_options_set_bool,   "force",   no_argument,       'f', "REUSE_ADDRESS", dc_flag_from_string,   "reuse_address", dc_flag_from_config,   &default_reuse},
            };
#pragma GCC diagnostic pop

    // note the trick here - we use calloc and add 1 to ensure the last line is all 0/NULL
    settings->opts.opts = dc_calloc(env, err, (sizeof(opts) / sizeof(struct options)) + 1, sizeof(struct options));
    dc_memcpy(env, settings->opts.opts, opts, sizeof(opts));
    settings->opts.flags = "c:vh:i:p:f";
    settings->opts.env_prefix = "DC_HTTP_";

    return (struct dc_application_settings *) settings;
}

int destroy_settings(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
                     struct dc_application_settings **psettings)
{
    struct application_settings *app_settings;

    app_settings = (struct application_settings *) *psettings;
    dc_setting_bool_destroy(env, &app_settings->verbose);
    dc_setting_string_destroy(env, &app_settings->hostname);
    dc_setting_uint16_destroy(env, &app_settings->port);
    dc_free(env, app_settings->opts.opts, app_settings->opts.opts_size);
    dc_free(env, app_settings, sizeof(struct application_settings));

    if (env->null_free)
    {
        *psettings = NULL;
    }

    return 0;
}


static struct dc_server_lifecycle *create_server_lifecycle(const struct dc_posix_env *env, struct dc_error *err)
{
    struct dc_server_lifecycle *lifecycle;

    lifecycle = dc_server_lifecycle_create(env, err);
    dc_server_lifecycle_set_create_settings(env, lifecycle, do_create_settings);
    dc_server_lifecycle_set_create_socket(env, lifecycle, do_create_socket);
    dc_server_lifecycle_set_set_sockopts(env, lifecycle, do_set_sockopts);
    dc_server_lifecycle_set_bind(env, lifecycle, do_bind);
    dc_server_lifecycle_set_listen(env, lifecycle, do_listen);
    dc_server_lifecycle_set_setup(env, lifecycle, do_setup);
    dc_server_lifecycle_set_accept(env, lifecycle, do_accept);
    dc_server_lifecycle_set_shutdown(env, lifecycle, do_shutdown);
    dc_server_lifecycle_set_destroy_settings(env, lifecycle, do_destroy_settings);

    return lifecycle;
}

static void destroy_server_lifecycle(const struct dc_posix_env *env, struct dc_server_lifecycle **plifecycle)
{
    DC_TRACE(env);
    dc_server_lifecycle_destroy(env, plifecycle);
}


int run(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
        struct dc_application_settings *settings)
{
    int ret_val;
    struct dc_server_info *info;

    info = dc_server_info_create(env, err, "HTTP Server Application", NULL, settings);

    if (dc_error_has_no_error(err)) {
        dc_server_run(env, err, info, create_server_lifecycle, destroy_server_lifecycle);
        dc_server_info_destroy(env, &info);
    }

    if (dc_error_has_no_error(err)) {
        ret_val = 0;
    } else {
        ret_val = -1;
    }

    return ret_val;
}


void signal_handler(__attribute__ ((unused)) int signnum)
{
    printf("caught\n");
    exit_signal = 1;
}

void do_create_settings(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct application_settings *app_settings;
    const char *ip_version;
    int family;

    DC_TRACE(env);
    app_settings = arg;
    ip_version = dc_setting_regex_get(env, app_settings->ip_version);

    if (dc_strcmp(env, ip_version, "IPv4") == 0)
    {
        family = PF_INET;
    } else
    {
        if (dc_strcmp(env, ip_version, "IPv6") == 0)
        {
            family = PF_INET6;
        } else
        {
            DC_ERROR_RAISE_USER(err, "Invalid ip_version", -1);
            family = 0;
        }
    }

    if (dc_error_has_no_error(err))
    {
        const char *hostname;

        hostname = dc_setting_string_get(env, app_settings->hostname);
        dc_network_get_addresses(env, err, family, SOCK_STREAM, hostname, &app_settings->address);
    }
}

void do_create_socket(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct application_settings *app_settings;
    int socket_fd;

    DC_TRACE(env);
    app_settings = arg;
    socket_fd = dc_network_create_socket(env, err, app_settings->address);

    if (dc_error_has_no_error(err))
    {
        app_settings = arg;
        app_settings->server_socket_fd = socket_fd;
    } else
    {
        socket_fd = -1;
    }
}

void do_set_sockopts(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct application_settings *app_settings;
    bool reuse_address;

    DC_TRACE(env);
    app_settings = arg;
    reuse_address = dc_setting_bool_get(env, app_settings->reuse_address);
    dc_network_opt_ip_so_reuse_addr(env, err, app_settings->server_socket_fd, reuse_address);
}

void do_bind(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct application_settings *app_settings;
    uint16_t port;

    DC_TRACE(env);
    app_settings = arg;
    port = dc_setting_uint16_get(env, app_settings->port);

    dc_network_bind(env,
                    err,
                    app_settings->server_socket_fd,
                    app_settings->address->ai_addr,
                    port);
}

void do_listen(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    struct application_settings *app_settings;
    int backlog;

    DC_TRACE(env);
    app_settings = arg;
    backlog = 5;
    dc_network_listen(env, err, app_settings->server_socket_fd, backlog);
}

void do_setup(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
              __attribute__ ((unused)) void *arg)
{
    DC_TRACE(env);
}

bool do_accept(struct dc_posix_env *env, struct dc_error *err, int *client_socket_fd, void *arg)
{
    struct application_settings *app_settings;
    bool ret_val;
    DC_TRACE(env);

    app_settings      = arg;
    ret_val           = false;
    *client_socket_fd = dc_network_accept(env, err, app_settings->server_socket_fd);


    if (dc_error_has_error(err))
    {
        if (exit_signal == true && dc_error_is_errno(err, EINTR))
        {
            ret_val = true;
        }
    } else
    {
        char response[BUFSIZ] = {0};
        receive_data(env, err, response, *client_socket_fd, BUFSIZ);
        // GET
        dc_send(env, err, *client_socket_fd, response, strlen(response), 0);
        dc_close(env, err, *client_socket_fd);
        exit_flag = 0;
    }

    return ret_val;
}

void do_shutdown(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
                 __attribute__ ((unused)) void *arg)
{
    DC_TRACE(env);
}

void
do_destroy_settings(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err, void *arg)
{
    struct application_settings *app_settings;

    DC_TRACE(env);
    app_settings = arg;
    dc_freeaddrinfo(env, app_settings->address);
}

__attribute__ ((unused)) static void
trace(__attribute__ ((unused)) const struct dc_posix_env *env, const char *file_name, const char *function_name,
      size_t line_number)
{
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}

// Look at the code in the client, you could do the same thing
void receive_data(struct dc_posix_env *env, struct dc_error *err, char *response, int fd, size_t size)
{
    // more efficient would be to allocate the buffer in the caller (main) so we don't have to keep
    // mallocing and freeing the same data over and over again.
    char *data;
    ssize_t count;
    data = dc_malloc(env, err, size);
    while (!(exit_flag) && (count = dc_read(env, err, fd, data, size)) > 0 && dc_error_has_no_error(err))
    {
        char *requestType;
        char *path;
        char *temp;
        char *content = NULL;
        char *filePath;
        int fileD;

        temp = malloc((strlen(data) + 1) * sizeof(char));
        strcpy(temp, data);
        temp[strlen(temp)] = '\0';
        dc_write(env, err, STDOUT_FILENO, temp, strlen(temp));

        requestType = strtok(temp, " ");
        path        = strtok(NULL, " ");


        if (dc_strcmp(env, requestType, "GET") == 0)
        {
            // Web browser, looks for file
            if (strlen(path) > 1)
            {
                // Allocates memory for the path, strlen of path + 2 for ".."(ROOT)
                filePath = calloc((strlen(path) + 2),sizeof(char));
                strcat(filePath, ROOT);
                strcat(filePath, path);
                filePath[strlen(filePath)] = '\0';
                fileD = open(filePath, DC_O_RDONLY, 0);
                printf("\nFD:%d\n", fileD);
                // Found a file.
                if(fileD != -1)
                {
                    openPagePath(env, err, fileD, response, CUSTOM);
                }
                // Couldn't locate file, returns 404.
                else
                {
                    open404Page(env, err, response);
                }
                free(content);
                free(filePath);
            }
            // Ncurses
            else if (strlen(path) == 1)
            {
                fileD = open(PATH_INDEX, DC_O_RDONLY, 0);
                openPagePath(env, err, fileD, response, INDEX);
            }
            getData(env, err);
        } else if (strcmp(requestType, "PUT") == 0)
        {
            char *body = strstr(data, "\r\n\r\n");
            if (body != NULL)
            {
                char *modBody = body + 4;
                char *majorMinor = strtok(modBody, " ");
                char *gpsLocation = strtok(NULL, " ");

                majorMinor[strlen(majorMinor)] = '\0';
                gpsLocation[strlen(gpsLocation)] = '\0';

                store_data(env, err, majorMinor, gpsLocation);
                if(dc_error_has_no_error(err))
                {
                    fileD = open(PATH_CREATED, DC_O_RDONLY, 0);
                    openPagePath(env, err, fileD, response, CREATED);
                }
            }

        }

        memset(data, '\0', strlen(data) + 1);
        free(temp);
        exit_flag = 1;
    }
    dc_free(env, data, size);
}

void getData(struct dc_posix_env *env, struct dc_error *err)
{
    DBM *db = dc_dbm_open(env, err, database, DC_O_RDWR | DC_O_CREAT, 0600);
    for (datum key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db))
    {
        datum data = dc_dbm_fetch(env, err, db, key);
        dc_write(env, err, STDOUT_FILENO, data.dptr, (size_t) data.dsize - 1);
        dc_write(env, err, STDOUT_FILENO, "\n", 1);
    }

    dc_dbm_close(env, err, db);
}

void store_data(struct dc_posix_env *env, struct dc_error *err, char *majorMinor, char *location)
{
    DBM *db = dc_dbm_open(env, err, database, DC_O_RDWR | DC_O_CREAT, 0600);
    if (dc_error_has_no_error(err))
    {
        store(env, err, db, majorMinor, location, DBM_REPLACE);

        if (dc_error_has_error(err))
        {
            if (err->type == DC_ERROR_ERRNO && err->errno_code == EINTR)
            {
                dc_error_reset(err);
            }
        }
    }
    dc_dbm_close(env, err, db);
}

unsigned long getNumberOfDigits (int n) {
    if (n < 0) return getNumberOfDigits ((n == INT_MIN) ? INT_MAX: -n);
    if (n < 10) return 1;
    return 1 + getNumberOfDigits (n / 10);
}

void open404Page(struct dc_posix_env *env, struct dc_error *err, char* response) {
    ssize_t nread;
    char *content;
    char fileContent[BUFSIZ] = {0};
    unsigned long numOfDigits;
    char headerError[]     = "HTTP/1.0 404 Not Found\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: ";
    char headerErrorRest[] = "\r\n"
                             "\r\n";
    char *contentLengthString;
    int errorFileDescriptor     = open(PATH_404, DC_O_RDONLY, 0);
    nread                       = dc_read(env, err, errorFileDescriptor, fileContent, BUFSIZ);
    content                     = malloc(nread * sizeof(char));
    strncpy(content, fileContent, nread);
    content[strlen(content)] = '\0';

    numOfDigits   = getNumberOfDigits((int) nread);
    contentLengthString   = calloc(numOfDigits, sizeof(char));

    sprintf(contentLengthString, "%d", (int) nread);

    strcat(response, headerError);
    strcat(response, contentLengthString);
    strcat(response, headerErrorRest);
    strcat(response, content);
    strcat(response, "\r\n\r\n");
    free(contentLengthString);
    dc_close(env, err, errorFileDescriptor);
}

void openPagePath(struct dc_posix_env *env, struct dc_error *err, int fd, char* response, enum file type) {
    ssize_t nread;
    char *content;
    char fileContent[BUFSIZ] = {0};
    unsigned long numOfDigits;
    char *contentLengthString;
    char headerCREATED[]     = "HTTP/1.0 201 Created\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: ";
    char headerCREATEDRest[] = "\r\n"
                               "\r\n";

    char headerOK[]          = "HTTP/1.0 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: ";
    char headerOKRest[]      = "\r\n"
                               "\r\n";

    nread                       = dc_read(env, err, fd, fileContent, BUFSIZ);
    content                     = malloc(nread * sizeof(char));
    strncpy(content, fileContent, nread);
    content[strlen(content)] = '\0';

    numOfDigits   = getNumberOfDigits((int) nread);
    contentLengthString   = calloc(numOfDigits, sizeof(char));
    sprintf(contentLengthString, "%d", (int) nread);
    
    if(type == CREATED)
    {
        strcat(response, headerCREATED);
        strcat(response, contentLengthString);
        strcat(response, headerCREATEDRest);
    }
    else
    {
        strcat(response, headerOK);
        strcat(response, contentLengthString);
        strcat(response, headerOKRest);
    }

    strcat(response, content);
    strcat(response, "\r\n\r\n");

    free(contentLengthString);
    dc_close(env, err, fd);
}
