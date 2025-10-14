#include <unistd.h>
#include <wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

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

enum ConnStatus {
    None,
    Send,
    Close,
};

int main(int argc, char *argv[]){
    if(argc !=2 || (argc ==2 && *(int32_t*)argv[1] == *(int32_t*)"-h")){
        fprintf(stderr, "Usage: %s <server port>", argv[0]);
        return 1;
    }
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    ERR(lfd == -1);

    int reuseaddr = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    struct sockaddr_in st_ser;
    st_ser.sin_addr.s_addr = INADDR_ANY;
    st_ser.sin_family = AF_INET;
    st_ser.sin_port = htons(atoi(argv[1]));
    ERR(bind(lfd, (struct sockaddr*)&st_ser, sizeof(struct sockaddr)) == -1);
    ERR(listen(lfd, MAX_FDS) == -1);
    time_t tp;

    int epfd = epoll_create(MAX_FDS);
    ERR(epfd == -1);
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    ERR(epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev) == -1);

    struct epoll_event evs[MAX_FDS];
    char sendbuf[30] = {0};
    while(true){
        int num = epoll_wait(epfd, evs, MAX_FDS, -1);
        for(int i = 0; i < num; ++i){
            int curfd = evs[i].data.fd;
            if(curfd == lfd){
                // ET模式下，需要将读取操作设置为非阻塞模式来读取
                int cfd = accept(curfd, NULL, NULL);
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag);
                // 水平触发(LT)，内核会一直发送事件通知，直至相关缓冲区为空或写满
                // ev.events = EPOLLIN;
                // 边沿触发(ET)，内核只会发送一次事件通知，当相关缓冲区从满或空转变时发送一次
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                ERR(epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1);
            }
            else {
                char buf[MAXBUFF] = {0};
                enum ConnStatus status = ConnStatus::None;
                while(true) {
                    int len = recv(curfd, buf, sizeof(buf), MSG_WAITALL);
                    // 当fd读缓冲区>buf长度时，recv会直接将对应字节填入其中
                    // 会导致buf[5]不为终止符'\0'，需要手动填入
                    // 不然会将输出接受值+随机值，即跟解引用野指针一样
                    buf[5] = '\0';

                    if(len > 0){
                        fprintf(stdout, "[Epoll fd: %d] RecvMsg: %s\n", curfd, buf);
                        time(&tp);
                        strcpy(sendbuf, ctime(&tp));
                        if(status != ConnStatus::Send) {
                            status =  ConnStatus::Send;
                            if(send(curfd, sendbuf, sizeof(sendbuf), 0) == -1)
                                fprintf(stderr, "[Error(Epoll fd %d)] (id: %d): %s\n", curfd, errno, strerror(errno));
                        }
                    }
                    else if(len == 0 || errno == ECONNRESET) {
                        fprintf(stdout, "[Info(Epoll fd %d)] Client has disconnnected.\n", curfd);
                        status = ConnStatus::Close;
                        break;
                    }
                    else {
                        if(errno == EAGAIN)
                            fprintf(stdout, "[Info(Epoll fd %d)] Reading RecvMsg has be done.\n", curfd);
                        // else if (errno == ECONNRESET)
                        //     fprintf(stdout, "[Info(Epoll fd %d)] Connection reset by peer.\n", curfd);
                        else{
                            fprintf(stderr, "[Error(poll fd %d)] (id: %d): %s\n", curfd, errno, strerror(errno));
                            status = ConnStatus::Close;
                        }
                        break;
                    }
                }
                if(status == ConnStatus::Close) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                    close(curfd);
                }
            }
        }
    }

    return 0;
}
