#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _POSIX_SOURCE
#include <arpa/inet.h>
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

bool dencrypt_fn(const hpack_t* const hpt) {
    ptlv_t tmp = encrypt(Nd2N, hpt);
    if (tmp != hpt->version) {
        fprintf(stderr, "[Error] This package is vaild\n");
        return false;
    } else if (hpt->flags)
        fprintf(stdout, "[Info] This package is not send from client\n");
    return true;
}
int main(int argc, char* argv[]) {
    if (argc != 2 || (argc == 2 && *(int16_t*)argv[1] == *(int16_t*)"-h")) {
        fprintf(stderr, "Using: %s <port>\n", argv[0]);
        return -1;
    }
    time_t timepoint;

    int listenfd = 0, connfd = 0, recvsize = 0;
    struct sockaddr_in st_ser;
    char recvbuff[MAXRBUF] = {0};
    char sendbuff[MAXSBUF] = {0};

    ERR((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)

    int reuseaddr = 1;
    ERR(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                   sizeof(reuseaddr)) == -1);

    memset(&st_ser, 0, sizeof(struct sockaddr_in));
    st_ser.sin_addr.s_addr = INADDR_ANY;   // 0.0.0.0
    st_ser.sin_port = htons(atoi(argv[1]));
    st_ser.sin_family = AF_INET;   // IPv4

    ERR(bind(listenfd, (struct sockaddr*)&st_ser, sizeof(st_ser)) == -1);
    ERR(listen(listenfd, 0x10) == -1);

    fprintf(stdout, "[Info] Waiting for client request...\n");
    ERR((connfd = accept(listenfd, nullptr, nullptr)) == -1);
    fprintf(stdout, "[Info] Connect Client (id: %d).\n", connfd);

    hpack_t hpt;
    while (1) {
        memset(recvbuff, 0, MAXRBUF);
        if ((recvsize = hrecv(connfd, recvbuff, &hpt, dencrypt_fn)) == -1) {
            fprintf(stderr, "Error(id: %d): %s\n", errno, strerror(errno));
            continue;
        } else if (recvsize == 0) {
            fprintf(stdout, "[Info] Client has disconnnected.\n");
            close(connfd);
            break;
        } else {
            fprintf(stdout, "RecvMsg: %s\n", recvbuff);
            memset(sendbuff, 0, MAXSBUF);
            strcpy(sendbuff, ctime(&timepoint));
            hpt.timestamp = timepoint;
            ++hpt.packet_seq;
            set_hpack_pft(FT_NOR, 1, &hpt);
            hpt.packet_size = MAXSBUF;
            hpt.flags = 1;
            hpt.check = crc32(sendbuff, MAXSBUF, 0);
            hpt.version = encrypt(Nd2N, &hpt);
            if (hsend(connfd, sendbuff, &hpt) == -1)
                fprintf(stderr, "Error(id: %d): %s\n", errno, strerror(errno));
        }
    }

    return 0;
}
