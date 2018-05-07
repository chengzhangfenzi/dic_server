/*************************************************************************
	> File Name: regisger.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时20分06秒
 ************************************************************************/

#include<server.h>


    /*检查用户状态的回调函数*/
int fu_callback(void *s, int f_num, char **f_value, char **f_name){
    state_t *ps = (state_t *)s;
    *ps = RTL_USEREXIST;
    return 0;
}

/*
 *此函数依然可用，但是功能已经被check_ustate包含
 * 检测用户名是否存在*/
state_t find_user(struct para_t data)
{
    char *errmsg;
    char sql[SQL_LENE];
    data.msg->cmd_type = RTL_NUSEREXIST;
    sprintf(sql, "select * from user where username = '%s'", data.msg->usrname);
    if (sqlite3_exec(data.db, sql, fu_callback, (void *)&data.msg->cmd_type, &errmsg) != SQLITE_OK) 
	{
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    if (data.msg->cmd_type == RTL_USEREXIST)
	{
        return RTL_USEREXIST;
    } else 
	{
        return RTL_NUSEREXIST;
    }
}


/*添加新用户到用户表*/
int inst_usr(struct para_t data)
{
    char *errmsg;
    char sql[SQL_LENE];
    sprintf(sql, "insert into user values('%s','%s','%c')", data.msg->usrname, data.msg->data,'N');
    if (sqlite3_exec(data.db, sql, NULL, NULL, &errmsg) != SQLITE_OK) 
	{
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_INSERTERR;
    }
    return RTL_SUCCESS;
}

/*注册*/
int fregister(struct para_t data)
{

    /*检测用户名是否存在*/
    if (find_user(data) == RTL_DBERR)
	{/*数据库有问题*/
        return -1;
    } else if (check_ustate(data) == RTL_NUSEREXIST)
	{
        /*用户不存在插入新用户*/
        if (inst_usr(data) == RTL_SUCCESS){
            data.msg->state = RTL_SUCCESS;
        } else {
            data.msg->state = RTL_INSERTERR;
            return -1;
        }
    } else 
	{
        /*用户已存在*/
        data.msg->state = RTL_USEREXIST;
        
    }
    data.msg->cmd_type = RPL_REGISTER;
    bzero(data.msg->data, data.msg->data_len);
    data.msg->data_len = 0;
    send_msg(data.connfd, (unsigned char *)data.msg);
    return 0;
}

