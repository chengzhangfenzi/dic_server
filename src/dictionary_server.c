/*************************************************************************
	> File Name: dirtionary_server.c
	> Author: 
	> Mail: 
	> Created Time: 2014��08��21�� ������ 18ʱ44��35��
 ************************************************************************/

#include <dictionary.h>
#include <sqlite3.h>
#include <time.h>

#define _DEBUG

/*���ݿ�͵����ļ���ַ��ַ*/
#define DB_ADDR "./data/my.db"
#define DIC_ADDR "./data/dict.txt"

/*�ͻ����޲�����½ʧЧʱ��*/
#define ACTIVE_TIME 30 

/*SQL��䳤��*/
#define SQL_LENE  100

/*������ַ�˿�����*/
#define MY_ADDR "192.168.2.78"
#define MY_PORT 8888


/*�������˺�����־��*/
#define FREGISTER   REQ_REGISTER
#define FLOGIN      REQ_LOGIN
#define FSEARCH     REQ_QUERYWORD
#define FLIST_HIS   REQ_HISTORY
#define FQUIT       REQ_EXIT

/*���ڻص������б�ǲ���״̬*/
state_t state;
struct para{
    sqlite3 *db;
    struct XProtocol *msg;
    int connfd;
};

/*��ʼ�����ݿ�*/
int init_database(sqlite3 **db);

/*��ʼ������*/
int init_network();

/*��������,���뵽����������*/
int recv_msg(int connfd, unsigned char *lo_data);

/*�������ݣ����ݰ��ɴ����������*/
int send_msg(int connfd, unsigned char *lo_data);

/*ѡ����*/
int sec_fun(sqlite3 *db, int connfd, unsigned char *lo_data);

/*����û����Ƿ����*/
state_t find_user(sqlite3 *db, char *uname);

/*������û����û���*/
int inst_usr(sqlite3 *db, char *uname, char *passwd);

/*ע��*/
int fregister(sqlite3 *db, int connfd, unsigned char *lo_data);


/*�û������Ƿ���ȷ*/
state_t check_passwd(sqlite3 *db, char *uname, char *passwd);

/*�����û�״̬*/
int update_ustate(sqlite3 *db, char *uname, char state);

/*��½*/
int flogin(sqlite3 *db, int connfd, unsigned char *lo_data);

/*��ѯ����*/
int fsearch(sqlite3 *db, int connfd, unsigned char *lo_data);

/*��ѯ��¼*/
int flist_his(sqlite3 *db, int connfd, unsigned char *lo_data);

/*�˳�*/
int quit(sqlite3 *db, int sockfd);


int main(int argc, char *argv[])
{
    sqlite3 *db;
    int listenfd, connfd, state;
    socklen_t clilen;
    struct sockaddr_in cliaddr;

    unsigned char lo_data[sizeof(struct XProtocol)];
    bzero(&lo_data, sizeof(struct XProtocol));
    
    /*��ʼ��*/
    init_database(&db);
    if ((listenfd = init_network()) < 0) {
        perror("fail to init_network");
        exit(-1);
    }

    while (1)
	{
        clilen = sizeof(cliaddr);
        /*�Ӽ���������ȡһ�����ӳ���,����ȡ���ӵĵ�ַ*/
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
		{
            perror("accept");
            exit(-1);
        }

        /*�����ӽ��̷��񲻶Ͻ��տͻ��˵�����,ֱ���ͻ��˹رչرռ���*/
        while (1)
		{
            state = recv_msg(connfd, lo_data);
            if (state < 0)
			{
                /*�ͻ����˳���δ֪�����˳��߳�*/
                perror("unknown cmd");
                break;
            } else if( state == 1)
			{
                /*�յ�����Ϣ���󣬲�����*/
                printf("�ͻ����˳�\n");
                continue;
            }
            sec_fun(db, connfd, lo_data);
        }
    }
    return 0;
}

/*��ʼ�����ݿ�*/
int init_database(sqlite3 **db)
{
    int retval = 0;
    char sql[50];
    
    /*��ָ�������ݿ��ļ�,��������ڽ�����һ��ͬ�������ݿ��ļ�*/
    retval = sqlite3_open(DB_ADDR, db);
    if (retval != SQLITE_OK)
	{
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

/*��ʼ����������˿�*/
int init_network()
{
    int listenfd;
    struct sockaddr_in myaddr;

    /*��������������*/
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("fail to socket");
        return -1;
    }

    /*�����շ���ַ����ṹ����*/
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = inet_addr(MY_ADDR);
    //myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(MY_PORT);

    if (bind(listenfd,(struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) 
	{
        perror("fail to bind");
        return -1;
    }
    listen(listenfd, 5);

#ifdef _DEBUG
    printf("Initializing Network...OK\n");
#endif
    return listenfd;
}

/*��������*/
int recv_msg(int connfd, unsigned char *lo_data)
{
    int state;
    struct XProtocol *msg = (struct XProtocol *)lo_data;
    struct XProtocol buffer;
    bzero(&buffer, sizeof(struct XProtocol));

    /*��������*/
    state = recv(connfd, &buffer, sizeof(struct XProtocol), 0);
    if (state == -1) 
	{
        /*���ճ�������һ��������Ϣ,���ﲻ�ܹر�����*/
        perror("recv");
        send_msg(connfd, (unsigned char *)&buffer);
        return 1;
    } else if (state == 0)
	{
        /*�ͻ��˹ر�����*/
        return -1;
    }
    
    /*���ݽ���*/
    strcpy(msg->usrname, buffer.usrname);
    msg->cmd_type = ntohs(buffer.cmd_type);
    msg->state = ntohs(buffer.state);
    msg->data_len = ntohl(buffer.data_len);
    strncpy(msg->data, buffer.data, msg->data_len);

#ifdef _DEBUG
    coutmsg(lo_data);
#endif
    return 0;
}

/*��������*/
int send_msg(int connfd, unsigned char *lo_data)
{
    int state;
    struct XProtocol *msg = (struct XProtocol *)lo_data;
    struct XProtocol buffer;
    bzero(&buffer, sizeof(struct XProtocol));

    /*���ݱ���*/
    strcpy(buffer.usrname, msg->usrname);
    buffer.cmd_type = htons(msg->cmd_type);
    buffer.state = htons(msg->state);
    buffer.data_len = htonl(msg->data_len);
    strncpy(buffer.data, msg->data, msg->data_len);

    send(connfd, (unsigned char *)&buffer, sizeof(struct XProtocol), 0);
#ifdef _DEBUG
    printf("send_msg OK!\n");
#endif
#ifdef _DEBUG
    coutmsg(lo_data);
#endif
    return 0;
}

/*ѡ����*/
int sec_fun(sqlite3 *db, int connfd, unsigned char *lo_data)
{
    int retval;
    struct XProtocol *msg = (struct XProtocol*) lo_data;

    switch (msg->cmd_type)
	{
        case FREGISTER:
            retval = fregister(db, connfd, lo_data);
            break;
        case FLOGIN:
            retval = flogin(db, connfd, lo_data);
            break;
        case FSEARCH:
            retval = fsearch(db, connfd, lo_data);
            break;
        case FLIST_HIS:
            retval = flist_his(db, connfd, lo_data);
            break;
        case REQ_EXIT:
            return 1;
        default:
            retval = -1;
            break;
    }

    return retval;
}

/*����û�״̬�Ļص�����*/
int fu_callback(void *para, int f_num, char **f_value, char **f_name)
{
    state = RTL_USEREXIST;
    return 0;
}

/*����û����Ƿ����*/
state_t find_user(sqlite3 *db, char *uname)
{
    char *errmsg;
    char sql[SQL_LENE];
    state = RTL_NUSEREXIST;
    sprintf(sql, "select * from user where username = '%s'",uname);
    if (sqlite3_exec(db, sql, fu_callback, NULL, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    if (state == RTL_USEREXIST){
        return RTL_USEREXIST;
    } else {
        return RTL_NUSEREXIST;
    }
}


/*������û����û���*/
int inst_usr(sqlite3 *db, char *uname, char *passwd)
{
    char *errmsg;
    char sql[SQL_LENE];
    sprintf(sql, "insert into user values('%s','%s','%c')", uname, passwd,'A');
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_INSERTERR;
    }
    return RTL_SUCCESS;
}

/*ע��*/
int fregister(sqlite3 *db, int connfd, unsigned char *lo_data)
{
    struct XProtocol *msg = (struct XProtocol*)lo_data;

    /*����û����Ƿ����*/
    if (find_user(db, msg->usrname) == RTL_USEREXIST){
        msg->state = RTL_USEREXIST;
    } else {
        /*�������û�*/
        if (inst_usr(db, msg->usrname, msg->data) == RTL_SUCCESS){
            msg->state = RTL_SUCCESS;
        } else {
            msg->state = RTL_INSERTERR;
        }
    }
    msg->cmd_type = RPL_REGISTER;
    bzero(msg->data,msg->data_len);
    msg->data_len = 0;
    send_msg(connfd, lo_data);


    return 0;
}

/*����û�����ص�����*/
int cp_callback(void *para, int f_num, char **f_value, char **f_name)
{
    state = RTL_RIGHTPASSWD;
    return 0;
}

/*�û������Ƿ���ȷ*/
state_t check_passwd(sqlite3 *db, char *uname, char *passwd)
{
    char *errmsg;
    char sql[SQL_LENE];
    sprintf(sql, "select * from user where username='%s' and passwd='%s'", uname, passwd);
    if (sqlite3_exec(db, sql, cp_callback, NULL, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    if (state == RTL_RIGHTPASSWD){
        return RTL_RIGHTPASSWD;
    } else {
        return RTL_WRONGPASSWD;
    }
}

/*�����û�״̬*/
int update_ustate(sqlite3 *db, char *uname, char state)
{
    char *errmsg;
    char sql[SQL_LENE];

    /*����û�״̬�Ƿ�Ϊ����*/
    sprintf(sql, "update user set state='%c' where username='%s'", state, uname);
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    return RTL_SUCCESS;
    
}

/*��½*/
int flogin(sqlite3 *db, int connfd, unsigned char * lo_data)
{
    struct XProtocol *msg = (struct XProtocol*)lo_data;

    /*����û����Ƿ����, �����Ƿ���ȷ*/
    if (find_user(db, msg->usrname) == RTL_USEREXIST)
	{
        if (check_passwd(db, msg->usrname, msg->data) == RTL_RIGHTPASSWD)
		{
            if (update_ustate(db, msg->usrname, 'A') == RTL_SUCCESS)
			{
                msg->state = RTL_SUCCESS;
            } else 
			{
                msg->state = RTL_USERONLINE;
            }
        } else 
		{
            msg->state = RTL_WRONGPASSWD;
        }
    } else 
	{
        msg->state = RTL_NUSEREXIST;
    }
	
    msg->cmd_type = RPL_LOGIN;
    bzero(msg->data,msg->data_len);
    msg->data_len = 0;
    send_msg(connfd, lo_data);
    return 0;
}

/*���뵥�ʲ�ѯ��¼*/
state_t insert_his(sqlite3 *db, char *uname, char *word)
{
    time_t rawtime;
    struct tm * timeinfo;
    char *errmsg;
    char sql[MAXLEN_DATA];
    char timestamp[20];

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    sprintf(timestamp, "%4d-%02d-%02d %02d:%02d:%02d",1900+timeinfo->tm_year, 1+timeinfo->tm_mon,
            timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec); 
#ifdef _DEBUG
    printf("%s\n", timestamp);
#endif
    sprintf(sql, "insert into searchhis values('%s','%s','%s')", uname, timestamp, word);
#ifdef _DEBUG
    printf("%s\n", sql);
#endif
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        //free(errmsg);
        return RTL_DBERR;
    }
    return RTL_SUCCESS;
}
    
/*��ѯ����*/
int fsearch(sqlite3 *db, int connfd, unsigned char * lo_data)
{
    struct XProtocol *msg = (struct XProtocol*)lo_data;
    char buff[MAXLEN_DATA] = "reply queryword";
    FILE *fp;
    char *buf = buff;

    /*���ҵ���*/
    if ((fp = fopen(DIC_ADDR, "r")) == NULL){
        perror("fail to open");
        return -1;
    }

#ifdef _DEBUG
    printf("word = %s\n",msg->data);
#endif
    msg->data[msg->data_len] = ' ';
    msg->data_len++;
    msg->data[msg->data_len] = '\0';
#ifdef _DEBUG
    printf("word = %s\n",msg->data);
#endif
    while (fgets(buff, MAXLEN_DATA, fp) != NULL){
        if (strncmp(buff, msg->data, msg->data_len) == 0)
            break;
    }

    if (strncmp(buff, msg->data, msg->data_len) == 0){
        msg->state = RTL_SUCCESS;
        while (*(++buf) != ' '){
            continue;
        }
        while (*(++buf) == ' '){
            continue;
        }
        insert_his(db, msg->usrname, msg->data);
    } else {
        msg->state = RTL_NOFINDWORD;
        bzero(buf, MAXLEN_DATA);
    }
    msg->cmd_type = RPL_QUERYWORD;
    msg->data_len = strlen(buf);
    strncpy(msg->data, buf, msg->data_len);
    send_msg(connfd, lo_data);
    return 0;
}

/*��ѯ��¼�Ļص�����*/
int lh_callback(void *pa, int f_num, char **f_value, char **f_name){
    struct para *p = (struct para *)pa;
    char buff[MAXLEN_DATA];

    sprintf(buff, "%s %s", f_value[2], f_value[1]);
    state = RTL_SUCCESS;
    p->msg->state = RTL_SUCCESS;
    p->msg->cmd_type = RPL_HISTORY;
    p->msg->data_len = strlen(buff);
    strncpy(p->msg->data, buff, p->msg->data_len);
    send_msg(p->connfd,(char *) p->msg);
    return 0;
}

/*��ѯ��¼*/
int flist_his(sqlite3 *db, int connfd, unsigned char * lo_data)
{
    struct XProtocol *msg = (struct XProtocol*)lo_data;
    struct para pa;
    pa.msg = msg;
    pa.db = db;
    pa.connfd = connfd;
    char *errmsg;
    char sql[SQL_LENE];
    sprintf(sql, "select * from searchhis where username = '%s'",msg->usrname);
    if (sqlite3_exec(db, sql, lh_callback, (void *)&pa, &errmsg) != SQLITE_OK) {
        
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    //if (msg->state = RTL_SUCCESS){
        msg->cmd_type = RPL_HISTORY;
        bzero(msg->data, MAXLEN_DATA);
        msg->data_len = 0;
        send_msg(connfd, lo_data);//������־
    //}
    return 0;
}

/*�˳�*/
int quit(sqlite3 *db ,int sockfd)
{
    close(sockfd);
    sqlite3_close(db);
    return 0;
}


