#include <ncurses.h>
#include "client.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
__attribute__ ((unused)) static volatile sig_atomic_t exit_signal = 0;


int main(int argc, char *argv[])
{
    dc_error_reporter reporter;
    dc_posix_tracer tracer;
    struct dc_error err;
    struct dc_posix_env env;
    struct dc_application_info *info;
    int ret_val;

    reporter = error_reporter;
    tracer = trace_reporter;
    tracer = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env,  tracer);
    info = dc_application_info_create(&env, &err, "HTTP Client Application");
    ret_val = dc_application_run(&env, &err, info, create_settings, destroy_settings, run, dc_default_create_lifecycle, dc_default_destroy_lifecycle,
                                 "~/.dcecho.conf",
                                 argc, argv);
    dc_application_info_destroy(&env, &info);
    dc_error_reset(&err);

    return ret_val;
}

struct dc_application_settings *create_settings(const struct dc_posix_env *env, struct dc_error *err)
{
    static const bool default_verbose = false;
    static const char *default_hostname = "localhost";
    static const char *default_ip = "IPv4";
    static const uint16_t default_port = DEFAULT_ECHO_PORT;
    static const char *default_message = "GET / HTTP/1.1\r\n\r\n";
    struct application_settings *settings;

    settings = dc_malloc(env, err, sizeof(struct application_settings));

    if(settings == NULL)
    {
        return NULL;
    }

    settings->opts.parent.config_path = dc_setting_path_create(env, err);
    settings->verbose = dc_setting_bool_create(env, err);
    settings->hostname = dc_setting_string_create(env, err);
    settings->ip_version = dc_setting_regex_create(env, err, "^IPv[4|6]");
    settings->port = dc_setting_uint16_create(env, err);
    settings->message = dc_setting_string_create(env, err);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
    struct options opts[] =
            {
                    {(struct dc_setting *)settings->opts.parent.config_path, dc_options_set_path,   "config",  required_argument, 'c', "CONFIG",  dc_string_from_string, NULL,      dc_string_from_config, NULL},
                    {(struct dc_setting *)settings->verbose,                 dc_options_set_bool,   "verbose", no_argument,       'v', "VERBOSE", dc_flag_from_string,   "verbose", dc_flag_from_config,   &default_verbose},
                    {(struct dc_setting *)settings->hostname,                dc_options_set_string, "host",    required_argument, 'h', "HOST",    dc_string_from_string, "host",    dc_string_from_config, default_hostname},
                    {(struct dc_setting *)settings->ip_version,              dc_options_set_regex,  "ip",      required_argument, 'i', "IP",      dc_string_from_string, "ip",      dc_string_from_config, default_ip},
                    {(struct dc_setting *)settings->port,                    dc_options_set_uint16, "port",    required_argument, 'p', "PORT",    dc_uint16_from_string, "port",    dc_uint16_from_config, &default_port},
                    {(struct dc_setting *)settings->message,                 dc_options_set_string, "message", required_argument, 'm', "MESSAGE", dc_string_from_string, "message", dc_string_from_config, default_message},
            };
#pragma GCC diagnostic pop

    // note the trick here - we use calloc and add 1 to ensure the last line is all 0/NULL
    settings->opts.opts = dc_calloc(env, err, (sizeof(opts) / sizeof(struct options)) + 1, sizeof(struct options));
    dc_memcpy(env, settings->opts.opts, opts, sizeof(opts));
    settings->opts.flags = "c:vh:i:p:m:";
    settings->opts.env_prefix = "DC_ECHO_";

    return (struct dc_application_settings *)settings;
}

int destroy_settings(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
                            struct dc_application_settings **psettings)
{
    struct application_settings *app_settings;

    app_settings = (struct application_settings *)*psettings;
    dc_setting_bool_destroy(env, &app_settings->verbose);
    dc_setting_string_destroy(env, &app_settings->hostname);
    dc_setting_uint16_destroy(env, &app_settings->port);
    dc_free(env, app_settings->opts.opts, app_settings->opts.opts_size);
    dc_free(env, app_settings, sizeof(struct application_settings));

    if(env->null_free)
    {
        *psettings = NULL;
    }

    return 0;
}

int run(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
               struct dc_application_settings *settings)
{
    struct application_settings *app_settings;
    const char *message;
    bool verbose;
    const char *hostname;
    const char *ip_version;
    in_port_t port;
    int ret_val;
    struct addrinfo hints;
    struct addrinfo *result;
    int family;
    int sock_fd;
    socklen_t size;
    size_t message_length;
    uint16_t converted_socket;

    app_settings = (struct application_settings *)settings;
    message = dc_setting_string_get(env, app_settings->message);

    if(message == NULL)
    {
        return -1;
    }

    verbose = dc_setting_bool_get(env, app_settings->verbose);
    hostname = dc_setting_string_get(env, app_settings->hostname);
    ip_version = dc_setting_regex_get(env, app_settings->ip_version);
    port = dc_setting_uint16_get(env, app_settings->port);
    ret_val = 0;

    if(verbose)
    {
        fprintf(stderr, "Connecting to %s @ %" PRIu16 " via %s\n", hostname, port, ip_version);
    }

    if(dc_strcmp(env, ip_version, "IPv4") == 0)
    {
        family = PF_INET;
    }
    else
    {
        if(dc_strcmp(env, ip_version, "IPv6") == 0)
        {
            family = PF_INET6;
        }
        else
        {
            assert("Can't get here" != NULL);
            family = 0;
        }
    }

    dc_memset(env, &hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    dc_getaddrinfo(env, err, hostname, NULL, &hints, &result);

    if(dc_error_has_error(err))
    {
        return -1;
    }

    sock_fd = dc_socket(env, err, result->ai_family, result->ai_socktype, result->ai_protocol);

    if(dc_error_has_error(err))
    {
        return -1;
    }

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    converted_socket = htons(port);

    if(dc_strcmp(env, ip_version, "IPv4") == 0)
    {
        struct sockaddr_in *sockaddr;

        sockaddr = (struct sockaddr_in *)result->ai_addr;
        sockaddr->sin_port = converted_socket;
        size = sizeof(struct sockaddr_in);
    }
    else
    {
        if(dc_strcmp(env, ip_version, "IPv6") == 0)
        {
            struct sockaddr_in6 *sockaddr;

            sockaddr = (struct sockaddr_in6 *)result->ai_addr;
            sockaddr->sin6_port = converted_socket;
            size = sizeof(struct sockaddr_in);
        }
        else
        {
            assert("Can't get here" != NULL);
            size = 0;
        }
    }

    dc_connect(env, err, sock_fd, result->ai_addr, size);

    if(dc_error_has_error(err))
    {
        return -1;
    }

    message_length = dc_strlen(env, message);
    int row;
    int col;
    // Initiate Ncurses window
    initscr();
    dc_write(env, err, sock_fd, message, message_length);
    char *buf = dc_malloc(env, err, BUFSIZ);
    // display data
    dc_read(env, err, sock_fd, buf, BUFSIZ);
    printw("(major,minor time:longitude,latitude)");
    printw("\n%s", buf);
    refresh();
    getch();
    endwin();

    dc_close(env, err, sock_fd);
    dc_freeaddrinfo(env, result);

    return ret_val;
}
