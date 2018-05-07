/*************************************************************************
	> File Name: dictionary.h
	> Author: 
	> Mail: 
	> Created Time: 2014年08月21日 星期四 17时29分51秒
 ************************************************************************/
#ifndef DICTIONARY_H_
#define DICTIONARY_H_


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <signal.h>

#define MAXLEN_USERNAME 8
#define MAXLEN_DATA     304

/*数据包类型*/
#define REQ_REGISTER    (unsigned short)0x0001 /*客户端发起的用户注册请求*/
#define RPL_REGISTER    (unsigned short)0x8001 /*服务器对用户注册请求的响应*/
#define REQ_LOGIN       (unsigned short)0x0002 /*客户端发起的用户登陆请求*/
#define RPL_LOGIN       (unsigned short)0x8002 /*服务器对用户登陆请求的响应*/
#define REQ_QUERYWORD   (unsigned short)0x0003 /*客户端发情的单词查询请求*/
#define RPL_QUERYWORD   (unsigned short)0x8003 /*服务器对单词查询请求的响应*/
#define REQ_HISTORY     (unsigned short)0x0004 /*客户端发起的历史记录查询请求*/
#define RPL_HISTORY     (unsigned short)0x8004 /*服务器对历史查询请求的响应*/
#define REQ_EXIT        (unsigned short)0x0005 /*客户端发起的退出登陆请求*/

/*返回状态说明*/
#define RTL_SUCCESS     (unsigned short)0x0000 /*成功*/
#define RTL_USEREXIST   (unsigned short)0x0001 /*用户已存在*/
#define RTL_NUSEREXIST  (unsigned short)0x0002 /*用户不存在*/
#define RTL_INSERTERR   (unsigned short)0x0004 /*插入用户失败*/
#define RTL_WRONGPASSWD (unsigned short)0x0005 /*用户密码错误*/
#define RTL_RIGHTPASSWD (unsigned short)0x0006 /*用户密码正确*/
#define RTL_USERONLINE  (unsigned short)0x0007 /*用户已在线*/
#define RTL_USEROFFLINE (unsigned short)0x0008 /*用户不在线*/
#define RTL_NOFINDWORD  (unsigned short)0x0009 /*未找到单词*/
#define RTL_NOHISTORY   (unsigned short)0x0010 /*查找用户历史失败*/
#define RTL_DBERR       (unsigned short)0x0011 /*查找数据库查询出错*/

typedef uint16_t state_t;

struct XProtocol 
{
    char    usrname[MAXLEN_USERNAME];
    uint16_t cmd_type;  /*note: in network byte order*/
    uint16_t state;     /*note: in network byte order*/
    uint32_t data_len;  /*note: in network byte order*/
    unsigned char data[MAXLEN_DATA];
}__attribute__((aligned(4)));

/*打印消息内容*/
void coutmsg(unsigned char * data);

#endif /*DICTIONARY_H_*/
