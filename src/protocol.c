#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../include/dict.h"

void parse_and_execute(Dict *db, char *buffer, int client_fd) {
    char cmd[16] = {0};
    char key[64] = {0};
    char value[256] = {0};
    char response[512] = {0};

    buffer[strcspn(buffer, "\r\n")] = 0;
    int matched = sscanf(buffer, "%15s %63s %255s", cmd, key, value);
    if (matched == 2 && strcmp(cmd, "GET") == 0) {
        char *res = dictGet(db, key);
        if (res) snprintf(response, sizeof(response), "$%zu\r\n%s", strlen(res), res);
        else sprintf(response, "$-1\r\n");
    }
    else if (matched == 3 && strcmp(cmd, "SET") == 0) {
        dictSet(db, key, value);
        sprintf(response, "+OK\r\n");
    }
    else if (matched == 2 && strcmp(cmd, "DELETE") == 0) {
        int res = dictDelete(db, key);
        sprintf(response, ":Delete %s\r\n", (res == 1 ? "Successful" : "Failed"));
    }
    else {
        sprintf(response, "-ERR unknown command or wrong arguments\r\n");
    }
    write(client_fd, response, strlen(response));
}