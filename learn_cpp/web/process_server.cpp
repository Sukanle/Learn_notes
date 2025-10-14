#if defined _POSIX_SOURCE || defined __MINGW32__ \
    || defined __CYGWIN__ || defined __unix__
#ifndef __cplusplus
#define _POSIX_C_SOURCE 200809L
#endif
#include <unistd.h>
#include <signal.h>
#include <wait.h>
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

#define ERR(is_faild)                                                 \
if (is_faild) {                                                       \
    fprintf(stderr, "[Error] (id: %d) %s\n", errno, strerror(errno)); \
    exit(0);                                                          \
}

void callback(){
    while(true){
        pid_t pid= waitpid(-1, NULL, WNOHANG);
        if(pid == -1){
            fputs("[Warning] The child process is still running, or the child process has been fully cleaned up.", stdout);
            break;
        }
        fprintf(stdout, "[Info] child process die, pid = %d\n", pid);
    }
}

int childWork(int fd);

int main(int argc, char *argv[]){
    if(argc != 2 || (argc == 2 && *(int16_t*)argv[1] == *(int16_t*)"-h")){
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int listenfd = 0;
    struct sockaddr_in st_fath;
    memset(&st_fath, 0, sizeof(st_fath));
    ERR((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1);
    st_fath.sin_port = htons(atoi(argv[1]));
    st_fath.sin_family = AF_INET;
    st_fath.sin_addr.s_addr = INADDR_ANY;
    ERR(bind(listenfd, (struct sockaddr*)&st_fath, sizeof(struct sockaddr_in)) == -1);
    ERR(listen(listenfd, 128) == -1);

    struct sigaction act;
    act.sa_flags = 0;
    act.sa_restorer = callback;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);

    int reuseaddr = 1;
    ERR(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr))==-1);

    while(true){
        struct sockaddr_in st_chld;
        int acceptfd = 0;
        uint32_t chldlen = sizeof(struct sockaddr_in);
        if((acceptfd = accept(listenfd, (struct sockaddr *)&st_chld, &chldlen)) == -1){
            if(errno == EINTR)
                continue;
            fprintf(stderr, "[Error] (id: %d) %s\n", errno, strerror(errno));
            exit(0);
        }
        char ip[24] = {0};
        fprintf(stdout, "Clinet IP address: %s, Port: %d\n",
            inet_ntop(AF_INET, &st_chld.sin_addr.s_addr, ip,sizeof(ip)),
            ntohs(st_chld.sin_port));
        pid_t pid = fork();
        if (pid == 0){
            close(listenfd);
            while(childWork(acceptfd) > 0);
            close(acceptfd);
            exit(0);
        }
        else if (pid >0)
            close(acceptfd);
    }

    return 0;
}

int childWork(int fd){
    char buff[MAXBUFF];
    memset(buff, 0, sizeof(buff));
    int len = read(fd, buff, sizeof(buff));
    time_t tp;

    if(len > 0){
        fprintf(stdout, "[Process id: %d] (fd: %d) RecvMsg: %s\n", getpid(), fd, buff);
        memset(buff, 0, sizeof(buff));
        time(&tp);
        strcpy(buff, ctime(&tp));
        if(write(fd, buff, MAXBUFF) == -1)
            fprintf(stderr, "[Error(Process id %d)] (fd: %d, id: %d): %s\n", getpid(), fd, errno, strerror(errno));
    }
    else if(len == 0)
        fprintf(stdout, "[Info(Process id %d)] (fd: %d)Client has disconnnected.\n", getpid(), fd);
    else
        fprintf(stderr, "[Error(Process id %d)] (fd: %d, id: %d): %s\n", getpid(), fd, errno, strerror(errno));

    return len;
}
