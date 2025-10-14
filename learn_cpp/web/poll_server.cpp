#include <unistd.h>
#include <arpa/inet.h>
#include <sys/poll.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#endif

#define MAX_FDS 1024
#define MAXBUFF 5

#define ERR(is_faild)                                                 \
if(is_faild) {                                                        \
    fprintf(stderr, "[Error] (id: %d) %s\n", errno, strerror(errno)); \
    exit(0);                                                          \
}

int main(int argc, char *argv[]){
    if(argc !=2 || (argc == 2 && *(uint16_t*)argv[1] == *(uint16_t*)"-h")){
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
    time_t tp;
    int reuseaddr = 1;
    ERR(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1);

    struct pollfd fds[MAX_FDS];
    for(int i = 0; i + 3 < MAX_FDS; i+=4){
        fds[i].fd = fds[i + 1].fd = fds[i + 2].fd = fds[i + 3].fd = fds[i + 4].fd = -1;
        fds[i].events = fds[i + 1].events = fds[i + 2].events = fds[i + 3].events = 
            fds[i + 4].events = POLLIN;
    }
    fds[0].fd = listenfd;
    int maxfd = 0;

    while(1){
        ERR(poll(fds, maxfd + 1, -1) == -1);
        if(fds[0].revents & POLLIN){
            struct sockaddr_in st_cli;
            uint32_t len = sizeof(struct sockaddr_in);
            fprintf(stdout, "[Info] Waiting for client requesrt...\n");
            int acceptfd = accept(listenfd, (struct sockaddr*)&st_cli, &len);
            fprintf(stdout, "[Info] Connect Client (id: %d).\n", acceptfd);
            int index = 0;
            for(; index < MAX_FDS; ++index)
                if(fds[index].fd == -1){
                    fds[index].fd = acceptfd;
                    break;
            }
            maxfd = index > maxfd ? index : maxfd;
        }
        for(int i = 1; i <= maxfd; ++i){
            if(fds[i].revents & POLLIN){
                char buf[MAXBUFF] = {0};
                int len = read(fds[i].fd, buf, sizeof(buf));

                if(len > 0){
                    fprintf(stdout, "[Poll fd: %d] RecvMsg: %s\n", fds[i].fd, buf);
                    char sendbuf[20] = {0};
                    time(&tp);
                    strcpy(sendbuf, ctime(&tp));
                    if(write(fds[i].fd, sendbuf, sizeof(sendbuf)) == -1)
                        fprintf(stderr, "[Error(Poll fd %d)] (id: %d): %s\n", fds[i].fd, errno, strerror(errno));
                }
                else if(len == 0){
                    fprintf(stdout, "[Info(Poll fd %d)] Client has disconnnected.\n", fds[i].fd);
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                else
                    fprintf(stderr, "[Error(Poll fd %d)] (id: %d): %s\n", fds[i].fd, errno, strerror(errno));
            }
        }
    }
}
