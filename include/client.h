//
// Created by toni on 2021-11-14.
//

#ifndef TEMPLATE2_CLIENT_H
#define TEMPLATE2_CLIENT_H
void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size);

/** Use this code for ncurses interface
 * #include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

void input_box();


void init_scr()
{
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);

    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
}

void mainBox() {
    /// get screen size
    int height, width, start_x, start_y;
    height  = 28;
    width   = 80;
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
    mvwprintw(menu, 11, 10, "*1. To get the data from DB - \"GET\"");
    mvwprintw(menu, 13, 10, "* Press 'ESC' to EXIT");
    wrefresh(menu);
}


int menu_box() {
    WINDOW * menu = newwin(10, 70, 20, 15);
    wbkgd(menu, COLOR_PAIR(3));

    refresh();
    box(menu, 0, 0);
    wrefresh(menu);

    input_box();

    keypad(menu, true);

    char* choices[] = {"PUT", "", "", "GET"};
    int select;
    int arrow;
    int highlight = 0;

    while(1) {
        for (int i = 0; i < 4; i++) {
            if (i == highlight) {
                wattron(menu, A_REVERSE);
            }
            mvwprintw(menu, i + 3, 3, choices[i]);
            wattroff(menu, A_REVERSE);
        }

        arrow = wgetch(menu);

        switch (arrow) {
            case KEY_UP:
                highlight -= 3;
                if (highlight <= 0) {
                    highlight = 0;
                }
                select = 0;
                break;
            case KEY_DOWN:
                highlight += 3;
                if (highlight >= 3) {
                    highlight = 3;
                }
                select = 1;
                break;
            default:
                break;
        }

        /// Enter
        if (arrow == 10) {
            break;
        }

        /// ESC
        if (arrow == 27) {
            return select = 27;
        }
    }
    mvprintw(30, 39, "Your choice was: \"%s\"", choices[highlight]);
    return select;
}


void input_box() {
    WINDOW * put_input = newwin(3, 58, 22, 23);
    wbkgd(put_input, COLOR_PAIR(2));
    refresh();
    box(put_input, 0, 0);
    wrefresh(put_input);


    WINDOW * get_input = newwin(3, 58, 25, 23);
    wbkgd(get_input, COLOR_PAIR(2));
    refresh();
    box(get_input, 0, 0);
    wrefresh(get_input);
}

int main() {
    init_scr();
    mainBox();
    while (1) {
        int num = menu_box();
        if (num == 0) {

        }

        if (num == 27) {
            break;
        }
    }

    endwin();
    return 0;
}
 */
#endif //TEMPLATE2_CLIENT_H
