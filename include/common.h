#ifndef COMMON_H
#define COMMON_H

#define MAX_CLIENTS 1024
#define READ_BUF_SIZE 1024

typedef struct ClientState {
    int fd;
    char buffer[READ_BUF_SIZE];
    int querylen;
} ClientState;

extern ClientState* clients[1024];

#endif

