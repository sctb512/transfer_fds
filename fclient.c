#include "socketfd.h"

int daemon_server(void *pfd) {
    struct byte_message bt_msg;
    int fd = *(int *)pfd;
    while(1) {
        message_recv(fd, &bt_msg);
    }
    return 0;
}

int daemon_server_start(void *pfd) {
    int clientfd, ret;
    int listenfd = *(int *)pfd;

    while(1) { 
        clientfd = socketfd_accept(listenfd);
        if(clientfd < 0) {
            printf("socketfd_accept failed!\n");
            return -1;
        }

        pthread_t tid;
        ret = pthread_create(&tid, NULL, (void *)daemon_server, &clientfd);
        if(ret != 0)
        {
            printf("pthread_create failed!");
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int clientfd;
    int ret;

    clientfd  =  socketfd_create(0);
    if(clientfd < 0) {
        printf("socketfd_create failed!\n");
        return -1;
    }

    ret = socketfd_connect(clientfd, UNIXSTR_PATH);
    if(ret < 0) {
        printf("socketfd_start failed!\n");
        return -1;
    }

    struct byte_message bt_msg;

    bt_msg.msg_type = FD_REQUEST;
    message_send(clientfd, &bt_msg, 0);

    fd_recv(clientfd, &bt_msg);

    printf ( "bt_msg.fds[0]: %d\n", bt_msg.fds[0]);
    pthread_t listen_tid;
    pthread_create(&listen_tid, NULL, (void *)daemon_server_start, &bt_msg.fds[0]);

    pthread_t tids[bt_msg.fds_num];
    for(int i=1;i<bt_msg.fds_num;i++) {
        printf ( "bt_msg.fds[%d]: %d\n",i , bt_msg.fds[i]);

        ret = pthread_create(&tids[i], NULL, (void *)daemon_server, &bt_msg.fds[i]);
        if(ret != 0)
        {
            printf("pthread_create failed!");
        }
    }

    bt_msg.msg_type = SUCCESS;
    message_send(clientfd, &bt_msg, 0);

    message_recv(clientfd, &bt_msg);

    while(1) {};

    return 0 ;
}