/*************************************************************************
	> File Name: main.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时15分53秒
 ************************************************************************/

#include<server.h>

void reaper(int signo);

void offline_alluser(int sign_no);

void change_ustate(int sign_o);

int main(int argc, char *argv[])
{
    int listenfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr;
    struct XProtocol buf;
    struct para_t data;

    bzero(&buf, sizeof(struct XProtocol));
    data.msg = &buf;
    
    /*初始化*/
    signal(SIGCHLD,reaper);
    signal(SIGINT, offline_alluser);
    signal(SIGTSTP,offline_alluser);
    signal(SIGQUIT,offline_alluser);
    if ((listenfd = init_network()) < 0) {
        perror("fail to init_network");
        exit(-1);
    }

    while (1)
	{
        clilen = sizeof(cliaddr);
        /*从监听队列中取一个连接出来,并获取连接的地址*/
        if ((data.connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0){
            perror("accept");
            exit(-1);
        }
        
        switch(fork())
		{
			case -1:
				exit(-1);
			 case 0:
				close(listenfd); /*关闭子进程的监听*/
				init_database(&data.db);
				/*开启子进程服务不断接收客户端的请求,直到客户端关闭关闭监听*/
				while (1)
				{
					if (recv_msg(data.connfd, (unsigned char *)data.msg) == 0)
					{
						sec_fun(data);
					} else 
					{
						/*客户端退出或未知错误，退出线程*/
						printf("客户端关闭连接\n");
						//perror("unknown error");
						quit(data);
					}
				}
			default:
				/*父进程*/
				close(data.connfd);
        }
    }
    return 0;
}

/*初始化数据库*/
int init_database(sqlite3 **db)
{
    int retval = 0;
    
    printf("%s-\n",DB_ADDR);
    /*打开指定的数据库文件,如果不存在将创建一个同名的数据库文件*/
    retval = sqlite3_open(DB_ADDR, db);
    if (retval != SQLITE_OK){
        perror("Can't open database");
        sqlite3_close(*db);
        exit(1);
    } else {
        printf("You have opened a sqlite3 database named my.db successfully!\n");
        printf("Congratulations! Have fun ! ^-^ \n");
    }

#ifdef _DEBUG
    printf("Initializing database...OK\n");
#endif
    return retval;
}

/*初始化监听服务端口*/
int init_network()
{
    int listenfd;
    struct sockaddr_in myaddr;

    /*创建监听描述符*/
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("fail to socket");
        return -1;
    }

    /*将接收方地址传入结构体内*/
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = inet_addr(MY_ADDR);
    //myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(MY_PORT);

    if (bind(listenfd,(struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("fail to bind");
        return -1;
    }
    listen(listenfd, 5);

#ifdef _DEBUG
    printf("Initializing Network...OK\n");
#endif
    return listenfd;
}

/*选择功能*/
int sec_fun(struct para_t data)
{
    int retval;

    switch (data.msg->cmd_type)
	{
        case FREGISTER:
            retval = fregister(data);
            break;
        case FLOGIN:
            retval = flogin(data);
            break;
        case FSEARCH:
            retval = fsearch(data);
            break;
        case FLIST_HIS:
            retval = flist_his(data);
            break;
        case FQUIT:
            quit(data);
            return 1;
        default:
            retval = -1;
            break;
    }

    return retval;
}

/*退出*/
void quit(struct para_t data)
{
    close(data.connfd);
    
    /*下现当前用户*/
    update_ustate(data);
    sqlite3_close(data.db);
    printf("子进程正常结束\n");
    exit(0);
}

void reaper(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0){
        continue;
    }
}
/*下线所有用户*/
void offline_alluser(int sign_no){
    char *errmsg;
    sqlite3 *db;
    if (sqlite3_open(DB_ADDR, &db) != SQLITE_OK){
        perror("Can't open database");
        exit(1);
    }
    if (sqlite3_exec(db, "update user set state = '0' where state != '0'", NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("update_error: %s\n", errmsg);
        free(errmsg);
    }
    exit(0);
}
void change_ustate(int sign_o){

}
