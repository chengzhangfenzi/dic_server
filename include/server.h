/*************************************************************************
	> File Name: server.h
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时12分46秒
 ************************************************************************/
#ifndef DICTIOARY_SERVER_H_
#define DICTIOARY_SERVER_H_

#include <dictionary.h>
#include <sqlite3.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define _DEBUG

/*数据库和单词文件地址地址*/
#define DB_ADDR "/home/chendezhi/farsight/item/dictionary/dic_server/data/my.db"
#define DIC_ADDR "/home/chendezhi/farsight/item/dictionary/dic_server/data/dict.txt"

/*客户端无操作登陆失效时间(分钟)*/
#define ACTIVE_TIME 30 

/*SQL语句长度*/
#define SQL_LENE  100

/*本机地址端口配置*/
#define MY_ADDR "192.168.2.78"
#define MY_PORT 8888


/*服务器端函数标志宏*/
#define FREGISTER   REQ_REGISTER
#define FLOGIN      REQ_LOGIN
#define FSEARCH     REQ_QUERYWORD
#define FLIST_HIS   REQ_HISTORY
#define FQUIT       REQ_EXIT

/*用于回调函数中标记查找状态*/
struct para_t
{
    sqlite3 *db;
    int connfd;
    struct XProtocol *msg;
};

/*初始化数据库*/
int init_database(sqlite3 **db);

/*初始化网络*/
int init_network();

/*接收数据,解码到本地数据中*/
int recv_msg(int connfd, unsigned char *lo_data);

/*发送数据，数据包由处理函数打包好*/
int send_msg(int connfd, unsigned char *lo_data);

/*选择功能*/
int sec_fun(struct para_t data);

/*检测用户状态*/
state_t check_ustate(struct para_t data);

/*注册*/
int fregister(struct para_t data);

/*更新用户状态*/
state_t update_ustate(struct para_t data);

/*登陆*/
int flogin(struct para_t data);

/*查询单词*/
int fsearch(struct para_t data);

/*查询记录*/
int flist_his(struct para_t data);

/*退出*/
void quit(struct para_t data);

#endif /*DICTIOARY_SERVER_H_*/
