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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#endif

#define MAXBUFF 1024
#define MAXLINK 128

#define ERR(is_faild)                                                 \
if (is_faild) {                                                       \
    fprintf(stderr, "[Error] (id: %d) %s\n", errno, strerror(errno)); \
    exit(0);                                                          \
}

struct SockInfo{
    int acceptfd;
    pthread_t thid;
    struct sockaddr_in addr;
};

void* working(void* arg);

int main(int argc, char *argv[]){
    if(argc != 2 || (argc == 2 && *(int16_t*)argv[1] == *(int16_t*)"-h")){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int listenfd = 0;
    ERR((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1);

    struct sockaddr_in st_ser;
    memset(&st_ser, 0, sizeof(struct sockaddr_in));
    st_ser.sin_port = htons(atoi(argv[1]));
    st_ser.sin_addr.s_addr = INADDR_ANY;
    ERR(bind(listenfd, (struct sockaddr*)&st_ser, sizeof(struct sockaddr_in)) == -1);
    ERR(listen(listenfd, MAXLINK) == -1);

    int reuseaddr = 1;
    ERR(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr))==-1);

    uint32_t len = sizeof(struct sockaddr);
    struct SockInfo infos[MAXLINK];
    memset(infos, 0, sizeof(struct SockInfo) * MAXLINK);
    for(int i = 0; i + 1 < MAXLINK; i += 2){
        infos[i].acceptfd = -1;
        infos[i].thid = -1;
        infos[i + 1].acceptfd = -1;
        infos[i + 1].thid = -1;
    }
    while(1)
    {
        // 创建子线程
        struct SockInfo* pinfo;
        for(int i=0; i < MAXLINK; ++i){
            if(infos[i].acceptfd == -1){
                pinfo = &infos[i];
                break;
            }
            if(i == MAXLINK - 1){
                sleep(1);
                --i;
            }
        }

        int connfd = accept(listenfd, (struct sockaddr*)&pinfo->addr, &len);
        printf("parent thread, connfd: %d\n", connfd);
        if(connfd == -1)
        {
            perror("accept");
            exit(0);
        }
        pinfo->acceptfd = connfd;
        pthread_create(&pinfo->thid, NULL, working, pinfo);
        pthread_detach(pinfo->thid);
    }

    // 释放资源
    close(listenfd);  // 监听

    return 0;
}

void* working(void* arg)
{
    time_t tp;
    while(1)
    {
        struct SockInfo* info = (struct SockInfo*)arg;
        // 接收数据
        char buf[MAXBUFF];
        int ret = read(info->acceptfd, buf, MAXBUFF);
        if(ret == 0){
            fprintf(stdout, "[Info(Thread id: %lu)] (fd: %d) Client has disconnnected.\n", info->thid, info->acceptfd);
            info->acceptfd = -1;
            break;
        }
        else if(ret == -1){
            fprintf(stderr, "[Error(Thread id: %lu)] (fd %d,id: %d): %s\n", info->thid, info->acceptfd, errno, strerror(errno));
            info->acceptfd = -1;
            break;
        }
        else{
            fprintf(stdout, "[Thread id: %lu] (fd: %d) RecvMsg: %s", info->thid, info->acceptfd, buf);
            time(&tp);
            memset(buf, 0, sizeof(buf));
            time(&tp);
            strcpy(buf, ctime(&tp));
            if(write(info->acceptfd, buf, MAXBUFF) == -1)
                fprintf(stderr, "[Error(Thread id: %lu)] (fd %d,id: %d): %s\n", info->thid, info->acceptfd, errno, strerror(errno));
        }
    }
    return NULL;
}
