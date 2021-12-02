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
            port = 1232;
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

        temp = malloc((dc_strlen(env, data) + 1) * sizeof(char));
        strcpy(temp, data);
        temp[dc_strlen(env, temp)] = '\0';
        dc_write(env, err, STDOUT_FILENO, temp, dc_strlen(env, temp));

        requestType = dc_strtok(env, temp, " ");
        path        = dc_strtok(env, NULL, " ");


        if (dc_strcmp(env, requestType, "GET") == 0)
        {
            // Web browser, looks for file
            if (dc_strlen(env, path) > 1)
            {
                // Allocates memory for the path, dc_strlen env, of path + 2 for ".."(ROOT)
                filePath = calloc((dc_strlen(env, path) + 2),sizeof(char));
                dc_strcat(env, filePath, ROOT);
                dc_strcat(env, filePath, path);
                filePath[dc_strlen(env, filePath)] = '\0';
                fileD = open(filePath, DC_O_RDONLY, 0);
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
            else if (dc_strlen(env, path) == 1)
            {
                fileD = open(PATH_INDEX, DC_O_RDONLY, 0);
                openPagePath(env, err, fileD, response, INDEX);
            }
            getData(env, err);
        } else if (strcmp(requestType, "PUT") == 0)
        {
            char *body = dc_strstr(env, data, "\r\n\r\n");
            if (body != NULL)
            {
                char *modBody = body + 4;
                char *majorMinor = dc_strtok(env, modBody, " ");
                char *gpsLocation = dc_strtok(env, NULL, " ");

                majorMinor[dc_strlen(env, majorMinor)] = '\0';
                gpsLocation[dc_strlen(env, gpsLocation)] = '\0';

        for (int i = 0; i < num_of_tokens; i++) {
            printf("Token: %s\n", token_array[i]);
        }
        free(temp);
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
    content[dc_strlen(env, content)] = '\0';

    numOfDigits   = getNumberOfDigits((int) nread);
    contentLengthString   = calloc(numOfDigits, sizeof(char));

    sprintf(contentLengthString, "%d", (int) nread);

    dc_strcat(env, response, headerError);
    dc_strcat(env, response, contentLengthString);
    dc_strcat(env, response, headerErrorRest);
    dc_strcat(env, response, content);
    dc_strcat(env, response, "\r\n\r\n");
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

    nread                    = dc_read(env, err, fd, fileContent, BUFSIZ);
    content                  = malloc(nread * sizeof(char));
    strncpy(content, fileContent, nread);
    content[dc_strlen(env, content)] = '\0';

    numOfDigits              = getNumberOfDigits((int) nread);
    contentLengthString      = calloc(numOfDigits, sizeof(char));
    sprintf(contentLengthString, "%d", (int) nread);

    if(type == CREATED)
    {
        dc_strcat(env, response, headerCREATED);
        dc_strcat(env, response, contentLengthString);
        dc_strcat(env, response, headerCREATEDRest);
    }
    else
    {
        dc_strcat(env, response, headerOK);
        dc_strcat(env, response, contentLengthString);
        dc_strcat(env, response, headerOKRest);
    }

    dc_strcat(env, response, content);
    dc_strcat(env, response, "\r\n\r\n");

    free(contentLengthString);
    dc_close(env, err, fd);
}
