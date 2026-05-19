#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "../include/dict.h"
#include "../include/protocol.h"
#include "../include/common.h"

#define PORT 6379
#define BUFFER_SIZE 1024
#define MAX_EVENTS 64

void set_nonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0); //获取文件属性标志
    if (flag == -1) {
        perror("fcntl F_GETFL");
        return;
    }
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL O_NONBLOCK");
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    Dict *db = dictCreate(16);

    //创建Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //socket创建空socket，bind函数绑定信息
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    //进入监听状态
    if (listen(server_fd, 5) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Mini-Redis is listening on port %d...\n", PORT);

    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    ClientState* clients[1024];

    while(1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for(int i = 0; i < n; i++) {
            int active_fd = events[i].data.fd;
            if (active_fd == server_fd) {
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len); //阻塞等待客户端连接
                if (client_fd < 0) {
                    perror("Accept failed");
                    continue;
                }
                printf("New client connected!\n");
                set_nonblocking(client_fd);
                clients[client_fd] = malloc(sizeof(ClientState));
                clients[client_fd]->fd = client_fd;
                memset(clients[client_fd]->buffer, 0, READ_BUF_SIZE);
                clients[client_fd]->querylen = 0;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
            }
            else if (events[i].events & EPOLLIN) {
                ClientState *client = clients[active_fd];
                while(1) {
                    int num_read = read(active_fd, client->buffer + client->querylen, READ_BUF_SIZE - client->querylen);
                    if (num_read == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break; //文件描述符没数据，且为非阻塞时，抛出EAGAIN；如果是阻塞，此处不会抛出EAGAIN，read会一直等
                    }
                    else if (num_read > 0) {
                        client->querylen += num_read;
                    }
                }
                char *r_pos = find_crlf(client);
                if (r_pos == NULL) break;
                else {
                    parse_and_execute(db, client->buffer, client->fd);
                    int remaining_len = client->querylen - (r_pos - buffer + 2);
                    memmove(client->buffer, r_pos + 2, remaining_len);
                    client->querylen = remaining_len;
                }
            }
        }

        // printf("New client connected!\n");
        // memset(buffer, 0, BUFFER_SIZE);
        // int valread = read(client_fd, buffer, BUFFER_SIZE-1);
        // if (valread > 0) {
        //     buffer[valread] = '\0';
        //     printf("Received: %s", buffer);
        //     parse_and_execute(db, buffer, client_fd);
        // }
        // close(client_fd);
        
    }

    dictFree(db);
    return 0;


}

/*
epoll笔记：
1.epoll_create1() creates a new epoll instance and returns a file descriptor referring to that instance.
  建立红黑树记录监听节点，建立就绪列表
2.epoll_ctl(int epfd, int op, int fd, struct epoll_event *_Nullable event);
  向内核的 Epoll 监听实例（红黑树）中添加、修改或删除需要监控的文件描述符（Socket）及事件，O(logn)的增删改查
  epfd：epoll_create返回的文件描述符
  op：对socket的动作（添加到红黑树/修改/删除）
  fd：操作的socket的fd，内核需要用fd这个整数作为二分查找的 Key，来决定这个新节点应该插在树的左子树还是右子树。一旦插入成功，target_fd 在这一阶段的使命就完成了。
  event：结构体，存fd与事件。当用户程序调用 epoll_wait 收割事件时，内核只负责把就绪链表里的 event 结构体拷贝回你传入的events数组中，内核绝对不会去查红黑树，也根本不知道、不关心这个节点在红黑树上的 Key（即最初的fd）是多少
  重点：调用epoll_ctl(ADD)时，为该socket注册一个回调函数；当该socket收到数据时，内核自动触发回调函数，在红黑树里找到该socket（此处socket内维护有哈希表，能O(1)找到该节点位于红黑树中的位置），检查事件是否对的上，并将其拷贝到就绪链表中
3.epoll_wait()：fetching items from the ready list of the epoll instance
4.LT与ET：
  使用ET必须使用非阻塞的文件描述符。如set_nonblocking()函数所示
  使用ET模式读client_fd时不能只读一次，写一个while(1)一直读，直到系统抛出EAGAIN为止
*/