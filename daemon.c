#include "socketfd.h"

int main(int argc, char *argv[])
{
    int clientfd;
    int ret;

    clientfd  =  socketfd_create(0);
    if(clientfd < 0) {
        printf("socketfd_create failed!\n");
        return -1;
    }

    ret = socketfd_connect(clientfd, SOCKET_PATH);
    if(ret < 0) {
        printf("socketfd_start failed!\n");
        return -1;
    }

    struct byte_message bt_msg;

    while(1) {
        char send_msg[1024];
        printf("type message content to send:");
        // scanf("%s", send_msg);
        fgets(send_msg, 1024, stdin);

        bt_msg.msg_type = MESSAGE;
        memset(bt_msg.msg_buf, 0, sizeof(bt_msg.msg_buf));
        memcpy(bt_msg.msg_buf, send_msg, sizeof(send_msg));

        message_send(clientfd, &bt_msg, 0);  
    }

    return 0 ;
}