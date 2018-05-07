/*************************************************************************
	> File Name: communication.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时16分34秒
 ************************************************************************/

#include<server.h>

/*接收数据*/
int recv_msg(int connfd, unsigned char *lo_data)
{
    int state;
    struct XProtocol *msg = (struct XProtocol *)lo_data;
    struct XProtocol buffer;
    bzero(&buffer, sizeof(struct XProtocol));

    /*接收命令*/
    state = recv(connfd, &buffer, sizeof(struct XProtocol), 0);
    if (state == -1) 
	{
        /*接收出错，返回一条错误消息,这里不能关闭连接*/
        perror("recv");
        send_msg(connfd, (unsigned char *)&buffer);
        return 1;
    } else if (state == 0)
	{
        /*客户端关闭连接*/
        //close(connfd);
        //exit(0);
        return -1;
    } else
	{
        /*数据解码*/
        bzero(lo_data, sizeof(struct XProtocol));
        strcpy(msg->usrname, buffer.usrname);
        msg->cmd_type = ntohs(buffer.cmd_type);
        msg->state = ntohs(buffer.state);
        msg->data_len = ntohl(buffer.data_len);
        strncpy(msg->data, buffer.data, msg->data_len);
    }
    

#ifdef _DEBUG
    printf("-----------recv_msg-------------");
    coutmsg(lo_data);
#endif
    return 0;
}

/*发送数据*/
int send_msg(int connfd, unsigned char *lo_data)
{
    int state;
    struct XProtocol *msg = (struct XProtocol *)lo_data;
    struct XProtocol buffer;
    bzero(&buffer, sizeof(struct XProtocol));

    /*数据编码*/
    strcpy(buffer.usrname, msg->usrname);
    buffer.cmd_type = htons(msg->cmd_type);
    buffer.state = htons(msg->state);
    buffer.data_len = htonl(msg->data_len);
    strncpy(buffer.data, msg->data, msg->data_len);

    send(connfd, (unsigned char *)&buffer, sizeof(struct XProtocol), 0);
    //alarm(10);
#ifdef _DEBUG
    printf("-----------send_msg-------------");
    coutmsg(lo_data);
#endif
    return 0;
}
