#ifndef SOCKETFD_H
#define SOCKETFD_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <pthread.h>

#define UNIXSTR_PATH "/var/tmp/threefds.msg"
#define SOCKET_PATH  "/var/tmp/threefds.sock"

enum type {
    MESSAGE,        //message, without socketfd
    FD_REQUEST,     //request to get fds
    FD_RESPONSE,    //return fds
    SUCCESS,        //hot update success
    DONE        //free resource done
};

struct byte_message {
    enum type msg_type;
    char msg_buf[32];
    int fds[1024];
    int fds_num;
};

extern int g_fds[1024];
extern int g_fds_num;

int socketfd_create(int reuse);
int socketfd_start(int socketfd, char *path);
int socketfd_connect(int socketfd, char *path);
int socketfd_accept(int listen_fd);

int fd_send(int socketfd, struct byte_message *bt_msg);
int fd_recv(int socketfd, struct byte_message *bt_msg);

int message_send(int socketfd, struct byte_message *bt_msg, int flags);
int message_recv(int socketfd, struct byte_message *bt_msg);

int message_handler(int socketfd, struct byte_message *bt_msg);

#endif