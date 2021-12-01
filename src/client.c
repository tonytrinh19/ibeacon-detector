#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/sys/dc_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "common.h"

void init_scr();
int mainBox();

void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size);


int main(void) {
    dc_error_reporter reporter;
    dc_posix_tracer tracer;
    struct dc_error err;
    struct dc_posix_env env;
    const char *host_name;
    struct addrinfo hints;
    struct addrinfo *result;

    reporter = error_reporter;
    tracer = trace_reporter;
    tracer = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);

    host_name = "192.168.1.119";
    dc_memset(&env, &hints, 0, sizeof(hints));
    hints.ai_family = PF_INET; // PF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    dc_getaddrinfo(&env, &err, host_name, NULL, &hints, &result);

    if (dc_error_has_no_error(&err)) {
        int socket_fd;

        socket_fd = dc_socket(&env, &err, result->ai_family, result->ai_socktype, result->ai_protocol);

        if (dc_error_has_no_error(&err)) {
            struct sockaddr *sockaddr;
            in_port_t port;
            in_port_t converted_port;
            socklen_t sockaddr_size;

            sockaddr = result->ai_addr;
            port = 8001;
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
                dc_connect(&env, &err, socket_fd, sockaddr, sockaddr_size);

                if (dc_error_has_no_error(&err)) {
                    struct sigaction old_action;

                    dc_sigaction(&env, &err, SIGINT, NULL, &old_action);

                    if (old_action.sa_handler != SIG_IGN) {
                        struct sigaction new_action;
                        char data[1024] = {0};

                        exit_flag = 0;
                        new_action.sa_handler = quit_handler;
                        sigemptyset(&new_action.sa_mask);
                        new_action.sa_flags = 0;
                        dc_sigaction(&env, &err, SIGINT, &new_action, NULL);

                        init_scr();

                        while (dc_read(&env, &err, STDIN_FILENO, data, 1024) > 0 && dc_error_has_no_error(&err)) {
                            data[strlen(data)] = '\0';
//                            printf("READ %s\n", data);

                            clear();
                            refresh();

                            int height, width, start_x, start_y;
                            height = 28;
                            width = 80;
                            start_x = 10;
                            start_y = 4;


                            /// create window for our input
                            WINDOW *menu = newwin(height, width, start_y, start_x);
                            wbkgd(menu, COLOR_PAIR(2));

                            refresh();


                            /// create box inside the window
                            box(menu, 0, 0);
                            mvwprintw(menu, 3, 21, "Welcome to i-beacon detection project");
                            mvwprintw(menu, 7, 10, "Choose one of the items in the box.");
                            mvwprintw(menu, 9, 10, "* To put the data to DB - \"PUT\"");
                            mvwprintw(menu, 11, 10, "* To get the data from DB - \"GET\"");
                            wrefresh(menu);

                            keypad(menu, true);

                            char *choices[] = {"PUT", "", "", "GET"};
                            int arrow;
                            int highlight = 0;

                            while (1) {

                                for (int i = 0; i < 4; i++) {
                                    if (i == highlight) {
                                        wattron(menu, A_REVERSE);
                                    }
                                    mvwprintw(menu, i + 18, 10, choices[i]);
                                    wattroff(menu, A_REVERSE);
                                }

                                arrow = wgetch(menu);

                                switch (arrow) {
                                    case KEY_UP:
                                        highlight -= 3;
                                        if (highlight <= 0) {
                                            highlight = 0;
                                        }
                                        break;
                                    case KEY_DOWN:
                                        highlight += 3;
                                        if (highlight >= 3) {
                                            highlight = 3;
                                        }
                                        break;
                                }
                                keypad(menu, false);

                                /// Enter
                                if (arrow == 10) {
                                    if (highlight == 0) {
                                        echo();
                                        char data_string[25];
                                        char put[40] = "PUT / HTTP/1.0\r\n\r\n";
                                        mvprintw(30, 39, "Your choice was: \"%s\"", choices[highlight]);
                                        move(22, 30);
                                        getstr(data_string);
                                        strcat(put, data_string);
                                        strcpy(data, put);
                                        fputs(data, stdin);
                                        dc_write(&env, &err, socket_fd, data, strlen(data) + 1);
                                        fflush(stdin);
                                        receive_data(&env, &err, socket_fd, 1024);
                                        break;
                                    }
                                    else if (highlight == 3) {
                                        char get[15] = "GET / HTTP/1.0";
                                        strcpy(data, get);
                                        mvprintw(30, 39, "Your choice was: \"%s\"", choices[highlight]);
                                        dc_write(&env, &err, socket_fd, data, strlen(data));
                                        fflush(stdin);
                                        receive_data(&env, &err, socket_fd, 1024);
                                        break;
                                    }
                                }
                            }

                            noecho();
                            memset(data, '\0', strlen(data) + 1);
                            delwin(menu);
                            refresh();
                            flash();
                        }
                    }
                }
            }
        }

        if (dc_error_has_no_error(&err)) {
            dc_close(&env, &err, socket_fd);
        }
    }
    endwin();
    return EXIT_SUCCESS;
}


// Look at the code in the client, you could do the same thing
void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size) {
    // more efficient would be to allocate the buffer in the caller (main) so we don't have to keep
    // mallocing and freeing the same data over and over again.
    char *data;
    ssize_t count;

    data = dc_malloc(env, err, size);

    if (!(exit_flag) && (count = dc_read(env, err, fd, data, size)) > 0 && dc_error_has_no_error(err)) {
        //Receiving data from the server and write it on client's terminal
        dc_write(env, err, STDOUT_FILENO, data, (size_t) count);
//        dc_write(env, err, fd, data, strlen(data));
//        memset(data, '\0', strlen(data));
    }
    dc_free(env, data, size);
}


void init_scr() {
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);

    curs_set(2);
    noecho();
    keypad(stdscr, TRUE);
}


//int mainBox() {
//    /// get screen size
//    int height, width, start_x, start_y;
//    height  = 28;
//    width   = 80;
//    start_x = 10;
//    start_y = 4;
//
//
//    /// create window for our input
//    WINDOW *menu = newwin(height, width, start_y, start_x);
//    wbkgd(menu, COLOR_PAIR(2));
//
//    refresh();
//
//
//    /// create box inside the window
//    box(menu, 0, 0);
//    mvwprintw(menu, 3, 21, "Welcome to i-beacon detection project");
//    mvwprintw(menu, 7, 10, "Choose one of the items in the box.");
//    mvwprintw(menu, 9, 10, "* To put the data to DB - \"PUT\"");
//    mvwprintw(menu, 11, 10, "* To get the data from DB - \"GET\"");
//    mvwprintw(menu, 13, 10, "* Press 'ESC' to EXIT");
//    wrefresh(menu);
//
//    keypad(menu, true);
//
//    char* choices[] = {"PUT", "", "", "GET"};
//    int select;
//    int arrow;
//    int highlight = 0;
//
//    while(1) {
//        char data_string[20];
//
//        for (int i = 0; i < 4; i++) {
//            if (i == highlight) {
//                wattron(menu, A_REVERSE);
//            }
//            mvwprintw(menu, i + 18, 10, choices[i]);
//            wattroff(menu, A_REVERSE);
//        }
//
//        arrow = wgetch(menu);
//
//        switch (arrow) {
//            case KEY_UP:
//                highlight -= 3;
//                if (highlight <= 0) {
//                    highlight = 0;
//                }
//                select = 0;
//                break;
//            case KEY_DOWN:
//                highlight += 3;
//                if (highlight >= 3) {
//                    highlight = 3;
//                }
//                select = 1;
//                break;
//            default:
//                break;
//        }
//
//        /// Enter
//        if (arrow == 10) {
//            echo();
//
//            if (select == 0) {
//                char put[30] = "put ";
//                mvprintw(30, 39, "Your choice was: \"%s\"", choices[highlight]);
//                move(22, 30);
//                getstr(data_string);
//                strcat(put, data_string);
//                char *put_string = malloc(strlen(put) * sizeof(char));
//                strcpy(put_string, put);
//                mvprintw(7, 7, put_string);
//                fputs(put_string, stdin);
//                break;
//            }
//            else if (select == 1) {
//                char get[30] = "get ";
//                mvprintw(30, 39, "Your choice was: \"%s\"", choices[highlight]);
//                move(25, 30);
//                getstr(data_string);
//                strcat(get, data_string);
//                char *get_string = malloc(strlen(data_string) * sizeof(char));
//                strcpy(get_string, get);
//                fputs(get_string, stdin);
//                break;
//            }
//            touchwin(menu);
//            free(data_string);
//        }
//
//        /// ESC
//        if (arrow == 27) {
//            select = 27;
//            break;
//        }
//    }
//}