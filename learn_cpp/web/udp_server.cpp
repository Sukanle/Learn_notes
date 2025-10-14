#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <wait.h>

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#endif

#define MAXBUF 1024
#define ERR(is_faild)                                                    \
    if (is_faild) {                                                      \
        fprintf(stderr, "[Error] (id %d) %s\n", errno, strerror(errno)); \
        exit(0);                                                         \
    }

int main(int argc, char* argv[]) {
    if (argc != 2 || (argc == 2 && !strcmp(argv[1], "-h"))) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 0;
    }

    int lfd = socket(AF_INET, SOCK_DGRAM, 0);
    ERR(lfd == -1);
    struct sockaddr_in st_ser;
    st_ser.sin_addr.s_addr = INADDR_ANY;
    st_ser.sin_port = htons(atoi(argv[1]));
    st_ser.sin_family = AF_INET;
    ERR(bind(lfd, (struct sockaddr*)&st_ser, sizeof(st_ser)) == -1);
    int reconn = 1;
    ERR(setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reconn, sizeof(reconn)) ==
        -1);

    char buf[MAXBUF];
    char sbuf[MAXBUF];
    char ipbuf[64];
    time_t tp;

    struct sockaddr_in cliaddr;
    uint32_t len = sizeof(cliaddr);

    // 3. 通信
    while (1) {
        // 接收数据
        memset(buf, 0, sizeof(buf));
        int rlen = recvfrom(lfd, buf, sizeof(buf), 0,
                            (struct sockaddr*)&cliaddr, &len);
        printf(
            "clinet IP addr: %s, port: %d\n",
            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ipbuf, sizeof(ipbuf)),
            ntohs(cliaddr.sin_port));
        fprintf(stdout, "RecvMsg: %s\n", buf);

        // 回复数据
        // 数据回复给了发送数据的客户端
        time(&tp);
        strcpy(sbuf, ctime(&tp));
        sendto(lfd, sbuf, rlen, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    }

    close(lfd);

    return 0;
}
