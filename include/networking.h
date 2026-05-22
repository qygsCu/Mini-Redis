#ifndef NETWORKING_H
#define NETWORKING_H

#include "dict.h"

#define PORT 6379
#define MAX_EVENTS 64

void set_nonblocking(int fd);
int init_server_socket(int port);
int init_epoll(int server_fd);
void init_client_manager(void);
void destroy_client_manager(void);
void accept_new_client(int server_fd, int epoll_fd);
void handle_client_readable(int active_fd, int epoll_fd, Dict *db);

#endif
