//
// Created by toni on 2021-11-08.
//

#ifndef TEMPLATE2_SERVER_H
#define TEMPLATE2_SERVER_H

#include <stdlib.h>

void receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size);
void store_data(struct dc_posix_env *env, struct dc_error *err, char *data);
unsigned long words(const char *sentence);

/* use this code later to parse the data line by line then word by word
 * int count_lines(const char *str) {
    int lines = 0;
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == '\n') {
            lines++;
        }
    }

    return lines;
}

int main() {
    char *template = "HTTP/1.1 200 OK\n"
                     "Content-Encoding: gzip\n"
                     "Accept-Ranges: bytes\n"
                     "Age: 242818\n"
                     "Cache-Control: max-age=604800\n"
                     "Content-Type: text/html; charset=UTF-8\n"
                     "Date: Sun, 14 Nov 2021 19:07:18 GMT\n"
                     "Etag: \"3147526947\"\n"
                     "Expires: Sun, 21 Nov 2021 19:07:18 GMT\n"
                     "Last-Modified: Thu, 17 Oct 2019 07:18:26 GMT\n"
                     "Server: ECS (sec/96ED)\n"
                     "X-Cache: HIT\n"
                     "Content-Length: 648\n"
                     "\r\n";

    char *template2 = "HTTP/1.1 200 OK\n"
                     "Content-Encoding: gzip\n"
                      "\r\n";

    char *temp = malloc((strlen(template) + 1) * sizeof(char));
    strcpy(temp, template);
    temp[strlen(temp)] = '\0';
    int lines = count_lines(temp);
    char *token_array[lines];

    char *rest = NULL;
    char *token;
    int index = 0;

    for (token = strtok_r(temp, "\n", &rest);
         token != NULL;
         token = strtok_r(NULL, "\n", &rest)) {

        char *token_ed = malloc((strlen(token) + 1) * sizeof(char));
        strcpy(token_ed, token);
        token_ed[strlen(token_ed)] = '\0';

        if (token_ed[strlen(token_ed)] == '\0') {
            token_array[index] = token_ed;
            index++;
        }
    }



    for (int i = 0; i < lines; ++i) {
        unsigned long wordsCount = words(token_array[i]);
        char *aLine = calloc((strlen(token_array[i]) + 1),sizeof(char));
        strcpy(aLine, token_array[i]);
        aLine[strlen(aLine)] = '\0';
        char *words_array[wordsCount];
        char *rest2 = NULL;
        int index2 = 0;
        char *tok;
        if (strcmp(aLine, "\r") == 0) {
            printf("can stop now\n");
            break;
        } else {
            for (tok = strtok_r(aLine, " ", &rest2);
                 tok != NULL;
                 tok = strtok_r(NULL, " ", &rest2)) {
                char *token_ed2 = calloc((strlen(tok) + 1),sizeof(char));
                strcpy(token_ed2, tok);
                token_ed2[strlen(token_ed2)] = '\0';
                if (token_ed2[strlen(token_ed2)] == '\0') {
                    words_array[index2] = token_ed2;
                    index2++;
                }
            }

            for (int j = 0; j < wordsCount; j++) {
                printf("%s ", words_array[j]);
            }
            printf("\n");
        }
//        printf("Index2: %d\n", index2);
//        printf("%s\n", token_array[i]);
    }





    return 0;
}
 */
#endif //TEMPLATE2_SERVER_H
