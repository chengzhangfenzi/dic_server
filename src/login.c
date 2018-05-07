/*************************************************************************
	> File Name: login.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时20分48秒
 ************************************************************************/

#include<server.h>


/*检查用户密码回调函数*/
int cp_callback(void *s, int f_num, char **f_value, char **f_name){
    state_t *ps = (state_t *)s;
    *ps = RTL_RIGHTPASSWD;
    return 0;
}

/*用户密码是否正确*/
state_t check_passwd(struct para_t data)
{
    char *errmsg;
    char sql[SQL_LENE];
    sprintf(sql, "select * from user where username='%s' and passwd='%s'", data.msg->usrname, data.msg->data);
#ifdef _DEBUG
    printf("-%s-\n", sql);
#endif
    data.msg->cmd_type = RTL_WRONGPASSWD;
    if (sqlite3_exec(data.db, sql, cp_callback, (void *)&data.msg->cmd_type, &errmsg) != SQLITE_OK) {
        printf("passwd_error: %s\n", errmsg);
        //free(errmsg);
        return RTL_DBERR;
    }
    if (data.msg->cmd_type == RTL_RIGHTPASSWD){
        return RTL_RIGHTPASSWD;
    } else {
        return RTL_WRONGPASSWD;
    }
}

/*检查用户状态的回调函数*/
int cu_callback(void *s, int f_num, char **f_value, char **f_name){
    struct XProtocol *msg = (struct XProtocol *)s;
#ifdef _DEBUG
    printf("msg->state:%hd, server->state:%s\n",msg->state, f_value[2]);
    printf("msg->state:%d, server->state:%d\n",(int)msg->state, atoi(f_value[2]));
#endif 
    if ((int)msg->state == atoi(f_value[2])){
        msg->cmd_type = RTL_SUCCESS;
    } else if (atoi(f_value[2]) == 0) {
        msg->cmd_type = RTL_USEROFFLINE;
    } else {
        msg->cmd_type = RTL_USERONLINE;
    }
    return 0;
}

/*查询用户状态*/
state_t check_ustate(struct para_t data)
{
    char *errmsg;
    char sql[SQL_LENE];
    bzero(sql, SQL_LENE);

    /*初始化为用户不存在，查找到用户，更改为用户状态*/
    data.msg->cmd_type = RTL_NUSEREXIST;
    sprintf(sql, "select * from user where username = '%s'", data.msg->usrname);
    if (sqlite3_exec(data.db, sql, cu_callback, (void *)data.msg, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    
    /*返回用户状态*/
    return data.msg->cmd_type;
}

/*更新用户状态*/
state_t update_ustate(struct para_t data)
{
    char *errmsg;
    char sql[SQL_LENE];
    bzero(sql, SQL_LENE);

    /*用户不在线更新用户状态*/

    sprintf(sql, "update user set state = '%hd' where username = '%s'", data.msg->state, data.msg->usrname);
#ifdef _DEBUG
    printf("%p-%s-\n",data.db, sql);
#endif
    if (sqlite3_exec(data.db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("update_error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    return RTL_SUCCESS;
    
}

/*登陆*/
int flogin(struct para_t data)
{
    uint16_t rtl = check_ustate(data);
    /*检查用户名是否存在, 密码是否正确*/
    if (rtl == RTL_USEROFFLINE){
        if (check_passwd(data) == RTL_RIGHTPASSWD){/*密码正确*/
            if (update_ustate(data) == RTL_SUCCESS){/*更新用户状态*/
                data.msg->state = RTL_SUCCESS;
            } else {
                data.msg->state = RTL_DBERR;
            }
        } else {/*密码错误*/
                data.msg->state = RTL_WRONGPASSWD;
        }
    } else {/*其它*/
        data.msg->state = rtl;
    }
    /*old_update by0.30
     * 
    if (find_user(data) == RTL_DBERR){
        return -1;
    } else if (find_user(data) != RTL_NUSEREXIST){
        if (check_passwd(data) == RTL_RIGHTPASSWD){
            if (update_ustate(data) == RTL_SUCCESS){
                data.msg->state = RTL_SUCCESS;
            } else {
                data.msg->state = RTL_USERONLINE;
            }
        } else {
            data.msg->state = RTL_WRONGPASSWD;
        }
    } else {
        data.msg->state = RTL_NUSEREXIST;
    }
    */
    data.msg->cmd_type = RPL_LOGIN;
    bzero(data.msg->data,data.msg->data_len);
    data.msg->data_len = 0;
    send_msg(data.connfd, (unsigned char *)data.msg);
    return 0;
}
