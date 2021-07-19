#include "socketfd.h"


int g_fds[1024] = {0};
int g_fds_num = 0;

/*
 *  create unix socket
 */
int socketfd_create(int reuse) {
    int socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(socketfd < 0) {
        printf ( "socket failed.\n" ) ;
        return  -1;
    }

    if (reuse) {
        int opt = 1;
        setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    }
    return socketfd;
} 
int socketfd_start(int socketfd, char *path) {
    int ret;
    struct sockaddr_un servaddr;

    bzero (&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy( servaddr.sun_path, path) ;

    unlink(path);
    ret  =  bind(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0) {
        printf ( "bind failed. errno = %d.\n" ,  errno ) ;
        close(socketfd);
        return  - 1 ;
    }
    listen(socketfd, 5);
    return 0;
}

int socketfd_connect(int socketfd, char *path) {
    struct sockaddr_un servaddr;
    int ret;

    bzero (&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, path);

    ret = connect(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0) {
        printf ( "connect failed.\n" ) ;
        return -1;
    }
    return 0;
}

int socketfd_accept(int listen_fd) {
    struct sockaddr_un client_addr;
    socklen_t client_len;
    int client_fd;

    client_len = sizeof(client_addr);
    client_fd = accept( listen_fd, (struct sockaddr*)&client_addr , &client_len);
    if ( client_fd < 0 ) {
        printf ( "accept failed. errno: %d.\n", errno) ;
        return -1;
    }
    return client_fd;
}

int message_recv(int socketfd, struct byte_message *bt_msg) {
    int ret = recv(socketfd , bt_msg, sizeof(struct byte_message), 0);
    if( ret <= 0 ) {
        return ret;
    }

    message_handler(socketfd, bt_msg);
    return 0;
}

int message_send(int socketfd, struct byte_message *bt_msg, int flags) {
    int ret = send(socketfd , bt_msg, sizeof(struct byte_message), flags);
    if( ret <= 0 ) {
        return ret;
    }

    return 0;
}


int fd_send(int socketfd, struct byte_message *bt_msg) {
    struct cmsghdr  * pcmsg;
    int ret;
    struct msghdr msg;
    struct iovec iov[1];

    union {  //对齐
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }  control_un;

    msg.msg_name  = NULL;
    msg.msg_namelen  = 0;
    //设置数据缓冲区
    iov[0].iov_base = bt_msg;;
    iov[0].iov_len = sizeof(struct byte_message);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    //设置辅助数据缓冲区和长度
    msg.msg_control = control_un.control;
    msg.msg_controllen  =  sizeof(control_un.control) ;

    //直接通过CMSG_FIRSTHDR取得附属数据
    pcmsg = CMSG_FIRSTHDR(&msg);
    pcmsg->cmsg_len = CMSG_LEN(sizeof(int)*bt_msg->fds_num);
    pcmsg->cmsg_level = SOL_SOCKET;
    pcmsg->cmsg_type = SCM_RIGHTS;  //指明发送的是描述符
    // *((int*)CMSG_DATA(pcmsg)) = bt_msg->fd;  //把描述符写入辅助数据
    memcpy((int*)CMSG_DATA(pcmsg), bt_msg->fds, sizeof(int)*bt_msg->fds_num);
    
    ret = sendmsg(socketfd, &msg, 0);  //send filedescriptor
    if (ret <= 0) {
        return ret;
    }
    return 0;
}

int fd_recv(int socketfd, struct byte_message *bt_msg) {
    struct cmsghdr  * pcmsg;
    int ret;
    struct msghdr msg;
    struct iovec iov[1];

    union {  //对齐
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }  control_un;

    msg.msg_name  = NULL;
    msg.msg_namelen  = 0;
    //设置数据缓冲区
    iov[0].iov_base = bt_msg;;
    iov[0].iov_len = sizeof(struct byte_message);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    //设置辅助数据缓冲区和长度
    msg.msg_control = control_un.control;
    msg.msg_controllen  =  sizeof(control_un.control) ;
    
    ret = recvmsg(socketfd, &msg, 0);  //send filedescriptor
    if (ret <= 0) {
        return ret;
    }

    if((pcmsg = CMSG_FIRSTHDR(&msg) ) != NULL && ( pcmsg->cmsg_len == CMSG_LEN(sizeof(int)*bt_msg->fds_num))) {
        if ( pcmsg->cmsg_level  !=  SOL_SOCKET ) {
            printf("cmsg_leval is not SOL_SOCKET\n");
            return -1;
        }

        if ( pcmsg->cmsg_type != SCM_RIGHTS ) {
            printf ( "cmsg_type is not SCM_RIGHTS" );
            return -1;
        }
        //这就是我们接收的描述符
        // bt_msg->fd = *((int*)CMSG_DATA(pcmsg));
        printf("bt_msg->fds_num: %d\n", bt_msg->fds_num);
        memcpy(bt_msg->fds, (int*)CMSG_DATA(pcmsg), sizeof(int)*bt_msg->fds_num);
    }

    message_handler(socketfd, bt_msg);
    return 0;
}

int message_handler(int socketfd, struct byte_message *bt_msg) {
    switch (bt_msg->msg_type) {
    case MESSAGE:
        printf("recv MESSAGE\n");
        printf("bt_msg->msg_buf: %s", bt_msg->msg_buf);
        break;
    case FD_REQUEST:
        printf("recv FD_REQUEST\n");

        memcpy(bt_msg->fds, g_fds, sizeof(int)*g_fds_num);
        bt_msg->fds_num = g_fds_num;
        
        bt_msg->msg_type = FD_RESPONSE;

        fd_send(socketfd, bt_msg);

        break;
    case FD_RESPONSE:
        printf("recv FD_RESPONSE\n");
        //检查是否收到了辅助数据，以及长度
        
        break;
    case SUCCESS:
        printf("recv SUCCESS\n");
        close(g_fds[0]);
        for(int i=1;i<g_fds_num;i++) {
            close(g_fds[i]);
        }

        bt_msg->msg_type = DONE;
        message_send(socketfd, bt_msg, 0);
        break;
    case DONE:
        printf("recv DONE\n");
        break;
    default:
        break;
    }
    return 0;
}
