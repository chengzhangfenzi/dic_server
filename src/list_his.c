/*************************************************************************
	> File Name: list_his.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时23分12秒
 ************************************************************************/

#include<server.h>


/*查询记录的回调函数*/
int lh_callback(void *pdata, int f_num, char **f_value, char **f_name){
    struct para_t *p = (struct para_t *)pdata;
    char buff[MAXLEN_DATA];

    sprintf(buff, "%-15s %s", f_value[2], f_value[1]);
    p->msg->state = RTL_SUCCESS;
    p->msg->cmd_type = RPL_HISTORY;
    p->msg->data_len = strlen(buff);
    strncpy(p->msg->data, buff, p->msg->data_len);
    send_msg(p->connfd,(char *) p->msg);
    return 0;
}

/*查询记录*/
int flist_his(struct para_t data)
{
    char *errmsg;
    char sql[SQL_LENE];

    /*检测用户状态*/
    if (check_ustate(data) != RTL_SUCCESS){ 
        data.msg->state = RTL_USEROFFLINE;
        data.msg->cmd_type = RPL_QUERYWORD;
        data.msg->data_len = 0;
        bzero(data.msg, MAXLEN_DATA);
        send_msg(data.connfd, (unsigned char *)data.msg);
    }

    sprintf(sql, "select * from searchhis where username = '%s'",data.msg->usrname);
    if (sqlite3_exec(data.db, sql, lh_callback, (void *)&data, &errmsg) != SQLITE_OK) {
        
        printf("error: %s\n", errmsg);
        free(errmsg);
        return RTL_DBERR;
    }
    //if (msg->state = RTL_SUCCESS){
        data.msg->cmd_type = RPL_HISTORY;
        bzero(data.msg->data, MAXLEN_DATA);
        data.msg->data_len = 0;
        send_msg(data.connfd, (unsigned char *)data.msg);
    //}
    return 0;
}
