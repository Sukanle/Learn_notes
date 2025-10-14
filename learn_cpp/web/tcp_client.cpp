#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _POSIX_SOURCE
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#endif

#define TCP_HPACKE 1
#ifdef TCP_HPACKE
#include "tcp_package.h"
#endif

#ifndef __cplusplus
#define nullptr NULL
#endif

#define MAXRBUF 20
#define MAXSBUF 30

#define ERR(is_faild)                                                     \
    if (is_faild) {                                                       \
        fprintf(stderr, "[Error] (id: %d) %s\n", errno, strerror(errno)); \
        exit(0);                                                          \
    }

int i_sockfd = -1;

bool dencrypt_fn(const hpack_t* const hpt) {
    ptlv_t tmp = encrypt(Nd2N, hpt);
    if (tmp != hpt->version) {
        fprintf(stderr, "[Error] This package is vaild\n");
        return false;
    }
    else if(!hpt->flags)
        fprintf(stdout, "[Info] This package is not send from server\n");
    return true;
}
void SigCatch([[maybe_unused]] int sigNum)   // 信号捕捉函数(捕获Ctrl+C)
{
    if (i_sockfd != -1) { close(i_sockfd); }
    printf("Bye~! Will Exit...\n");
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3 || (argc == 2 && *(int16_t*)argv[1] == *(int16_t*)"-h")) {
        fprintf(stderr, "Using: %s <socket server ip> <socket server port>\n",
                argv[0]);
        return -1;
    }

    struct sockaddr_in st_clnsock;
    char msg[MAXRBUF] = {0};
    char recv[MAXSBUF] = {0};
    int nrecvSize = 0;

    signal(SIGINT, SigCatch);   // 注册信号捕获函数

    ERR((i_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)   // 建立套接字

    memset(&st_clnsock, 0, sizeof(st_clnsock));
    st_clnsock.sin_family = AF_INET;   // IPv4协议
    // IP地址转换(直接可以从物理字节序的点分十进制 转换成网络字节序)
    ERR(inet_pton(AF_INET, argv[1], &st_clnsock.sin_addr) != 1)
    st_clnsock.sin_port =
        htons(atoi(argv[2]));   // 端口转换(物理字节序到网络字节序)

    ERR(connect(i_sockfd, (struct sockaddr*)&st_clnsock, sizeof(st_clnsock)) ==
        -1)   // 主动向设置的IP和端口号的服务端发出连接

    printf("Connect to server, sent data...\n");

    hpack_t hpt;
    time_t timepoint;
    hpt.packet_magic = HPACK_MAGIC;
    while (1)   // 循环输入，向服务端发送数据并接受服务端返回的数据
    {
        time(&timepoint);
        memset(&msg, 0, MAXRBUF);
        memset(&recv, 0, MAXSBUF);
        fscanf(stdin, "%s", msg);
        hpt.timestamp = timepoint;
        ++hpt.packet_seq;
        set_hpack_pft(FT_NOR, 1, &hpt);
        hpt.packet_size = MAXSBUF;
        hpt.flags = 0;
        hpt.check = crc32(msg, MAXSBUF, 0);
        hpt.version = encrypt(Nd2N, &hpt);
        printf("Will send: %s\n", msg);
        if (hsend(i_sockfd, msg, &hpt) == -1)   // 发送数据
        {
            printf("Write Error: %s (errno: %d)\n", strerror(errno), errno);
            exit(0);
        }

        if ((nrecvSize = hrecv(i_sockfd, recv, &hpt, dencrypt_fn)) < 0) {   // 接受数据
            printf("Read Error: %s (errno: %d)\n", strerror(errno), errno);
        } else if (nrecvSize == 0)
            printf("Service Close!\n");
        else
            printf("Server return: %s\n", recv);
    }
    return 0;
}
