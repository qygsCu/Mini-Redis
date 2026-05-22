// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>
#include "../include/dict.h"
#include "../include/networking.h"
#include "../include/common.h"

int main() {
    // 1. 初始化全局资源
    Dict *db = dictCreate(16);
    int server_fd = init_server_socket(PORT);
    int epoll_fd = init_epoll(server_fd);

    // 初始化客户端管理中心（这里可以由动态数组或全局 Hash 支撑）
    init_client_manager();

    struct epoll_event events[MAX_EVENTS];
    printf("InferCache Server engine started on port %d...\n", PORT);

    // 2. 纯粹的调度中心主循环
    while(1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue; // 处理系统中断信号
            perror("epoll_wait fatal error");
            break;
        }

        for(int i = 0; i < n; i++) {
            int active_fd = events[i].data.fd;

            if (active_fd == server_fd) {
                // 处理新连接（封装进 networking.c）
                accept_new_client(server_fd, epoll_fd);
            }
            else if (events[i].events & EPOLLIN) {
                // 处理非阻塞读取与业务路由（封装进 networking.c 和 protocol.c）
                handle_client_readable(active_fd, epoll_fd, db);
            }
        }
    }

    // 3. 释放资源
    destroy_client_manager();
    dictFree(db);
    return 0;
}