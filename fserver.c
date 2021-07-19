#include "socketfd.h"

int daemon_server() {
    int daemonfd, clientfd;
    int ret;

    daemonfd  =  socketfd_create(1);
    if(daemonfd == -1) {
        printf("socketfd_create failed!\n");
        return -1;
    }

    ret = socketfd_start(daemonfd, SOCKET_PATH);
    if(ret == -1) {
        printf("socketfd_start failed!\n");
        return -1;
    }

    clientfd = socketfd_accept(daemonfd);
    if(clientfd < 0) {
        printf("socketfd_accept failed!\n");
        return -1;
    }
    
    g_fds[g_fds_num++] = daemonfd;
    g_fds[g_fds_num++] = clientfd;
    
    struct byte_message bt_msg;
    while(1) {
        message_recv(clientfd, &bt_msg);
    }
}

int main(int argc, char *argv[])
{
    int listenfd, clientfd;
    int ret;

    pthread_t tid;
	ret = pthread_create(&tid, NULL, (void *)daemon_server, NULL);
	if(ret != 0)
	{
		printf("pthread_create failed!");
	}

    listenfd  =  socketfd_create(1);
    if(listenfd == -1) {
        printf("socketfd_create failed!\n");
        return -1;
    }

    ret = socketfd_start(listenfd, UNIXSTR_PATH);
    if(ret == -1) {
        printf("socketfd_start failed!\n");
        return -1;
    }

    clientfd = socketfd_accept(listenfd);
    if(clientfd < 0) {
        printf("socketfd_accept failed!\n");
        return -1;
    }
    
    struct byte_message bt_msg;
    
    message_recv(clientfd, &bt_msg);

    message_recv(clientfd, &bt_msg);

    bt_msg.msg_type = DONE;
    message_send(clientfd, &bt_msg, 0);

    return 0 ;
}