#if defined _POSIX_SOURCE || defined __MINGW32__ \
    || defined __CYGWIN__ || defined __unix__
#ifndef __cplusplus
#define _POSIX_C_SOURCE 200809L
#endif
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/select.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#endif

#define MAXBUFF 5

#define ERR(is_faild)                                                 \
if (is_faild) {                                                       \
    fprintf(stderr, "[Error] (id: %d) %s\n", errno, strerror(errno)); \
    exit(0);                                                          \
}
int main(int argc, char* argv[]){
    if(argc!=2 || (argc==2 && *(uint16_t*)&argv[1] == *(uint16_t*)"-h")){
        fprintf(stderr, "Usage: %s <server port>\n", argv[0]);
        return 1;
    }

    int listenfd = 0;
    ERR((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1);
    struct sockaddr_in st_ser;

    memset(&st_ser, 0, sizeof(struct sockaddr_in));
    st_ser.sin_port = htons(atoi(argv[1]));
    st_ser.sin_addr.s_addr = INADDR_ANY;
    ERR(bind(listenfd, (struct sockaddr*)&st_ser, sizeof(struct sockaddr_in)) == -1);
    ERR(listen(listenfd, 128) == -1);

    int reuseaddr = 1;
    ERR(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1);

    int maxfd = listenfd;
    fd_set rdset;
    fd_set rdtmp;
    FD_ZERO(&rdset);
    FD_SET(listenfd, &rdset);
    time_t tp;

    while(true){
        rdtmp = rdset;
        int num = select(maxfd + 1, &rdtmp, NULL, NULL, NULL);
        if(FD_ISSET(listenfd, &rdtmp)){
            struct sockaddr_in st_cli;
            uint32_t clilen = sizeof(st_cli);
            int acceptfd = accept(listenfd, (struct sockaddr*)&st_cli, &clilen);

            FD_SET(acceptfd, &rdset);
            maxfd = acceptfd > maxfd ? acceptfd : maxfd;
        }

        for(int i = 0; i < maxfd + 1; ++i){
            if( i != listenfd && FD_ISSET(i, &rdtmp)){
                char buf[MAXBUFF] = {0};
                int len = read(i, buf, sizeof(buf));

                if(len > 0){
                    fprintf(stdout, "[Select fd: %d] RecvMsg: %s\n", i, buf);
                    char sendbuf[20] = {0};
                    time(&tp);
                    strcpy(sendbuf, ctime(&tp));
                    if(write(i, sendbuf, sizeof(sendbuf)) == -1)
                        fprintf(stderr, "[Error(Select fd %d)] (id: %d): %s\n", i, errno, strerror(errno));
                }
                else if(len == 0){
                    fprintf(stdout, "[Info(Select fd %d)] Client has disconnnected.\n", i);
                    FD_CLR(i, &rdset);
                    close(i);
                }
                else
                    fprintf(stderr, "[Error(Select fd %d)] (id: %d): %s\n", i, errno, strerror(errno));
            }
        }
    }
}
