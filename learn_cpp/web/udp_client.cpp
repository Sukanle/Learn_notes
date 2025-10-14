#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#endif

#include <arpa/inet.h>
#include <unistd.h>
#include <wait.h>

#define MAXSIZE 1024
#define ERR(is_faild)                                                    \
    if (is_faild) {                                                      \
        fprintf(stderr, "[Error] (id %d) %s\n", errno, strerror(errno)); \
        exit(0);                                                         \
    }

int main(int argc, char* argv[]) {
    if (argc != 3 || (argc == 2 && !strcmp(argv[1], "-h"))) {
        fprintf(stderr, "Usage: %s <ipaddr> <port>\n", argv[0]);
        return 1;
    }

    // 1. 创建通信的套接字
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    ERR(fd == -1);

    // 初始化服务器地址信息
    struct sockaddr_in seraddr;
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(atoi(argv[2]));   // 大端
    inet_pton(AF_INET, argv[1], &seraddr.sin_addr.s_addr);

    char buf[1024];
    char ipbuf[64];
    struct sockaddr_in cliaddr;
    int len = sizeof(cliaddr);
    int num = 0;
    // 2. 通信
    while (1) {
        sprintf(buf, "hello, udp %d....\n", num++);
        // 发送数据, 数据发送给了服务器
        sendto(fd, buf, strlen(buf) + 1, 0, (struct sockaddr*)&seraddr,
               sizeof(seraddr));

        // 接收数据
        memset(buf, 0, sizeof(buf));
        recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
        printf("Recv from server: %s\n", buf);
        sleep(1);
    }

    close(fd);

    return 0;
}
