#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/dict.h"
#include "../include/protocol.h"

#define PORT 6379
#define BUFFER_SIZE 1024

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

    while(1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len); //阻塞等待客户端连接
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New client connected!\n");
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(client_fd, buffer, BUFFER_SIZE-1);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Received: %s", buffer);
            parse_and_execute(db, buffer, client_fd);
        }
        close(client_fd);
        
    }

    dictFree(db);
    return 0;


}