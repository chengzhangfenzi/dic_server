/*************************************************************************
	> File Name: search.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时22分29秒
 ************************************************************************/

#include<server.h>

/*插入单词查询记录*/
state_t insert_his(struct para_t data)
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
    sprintf(sql, "insert into searchhis values('%s','%s','%s')", data.msg->usrname, timestamp, data.msg->data);
#ifdef _DEBUG
    printf("%s\n", sql);
#endif
    if (sqlite3_exec(data.db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("error: %s\n", errmsg);
        //free(errmsg);
        return RTL_DBERR;
    }
    return RTL_SUCCESS;
}
    
/*查询单词*/
int fsearch(struct para_t data)
{
    char buff[MAXLEN_DATA];
    FILE *fp;
    char *buf = buff;
    int i= 0;
    char word1[35]="\0", word2[35] = "\0";

    /*检测用户状态*/
    if (check_ustate(data) != RTL_SUCCESS){ 
        data.msg->state = RTL_USEROFFLINE;
        data.msg->cmd_type = RPL_QUERYWORD;
        data.msg->data_len = 0;
        bzero(data.msg, MAXLEN_DATA);
        send_msg(data.connfd, (unsigned char *)data.msg);
    }

    /*查找单词*/
    if ((fp = fopen(DIC_ADDR, "r")) == NULL){
        perror("fail to open");
        return -1;
    }

#ifdef _DEBUG
    printf("msg->data = %s\n",data.msg->data);
#endif
    strncpy(word1, data.msg->data, data.msg->data_len);
    while (word1[i] != '\0'){//转为小写
        if (word1[i] >= 'A' && word1[i] <= 'Z'){
            word1[i] += 32;
        }
        i++;
    }
    /*末尾补空格*/
    word1[i] = ' ';
    word1[i+1] = '\0';
    i = 0;
    while (word1[i] != '\0'){
        word2[i] = word1[i]-32;//大写
        i++;
    }
    word2[i] = '\0';
#ifdef _DEBUG
    printf("word1 = %s, word2 = %s\n", word1, word2);
#endif
    while (fgets(buff, MAXLEN_DATA, fp) != NULL){
        if (!strncmp(buff, word1, i-1) ||  !strncmp(buff, word2, i-1))
            break;
    }

    if (!strncmp(buff, word1,i-1) || !strncmp(buff, word2, i-1))
	{
#ifdef _DEBUG
        printf("%s", buff);
#endif
		//找到单词
        data.msg->state = RTL_SUCCESS;
        while (*(++buf) != ' '){
            continue;//跳过第一个单词
        }
        while (*(++buf) == ' '){
            continue;//跳过单词后面的空格
        }
        insert_his(data);
    } else 
	{//没找到
        data.msg->state = RTL_NOFINDWORD;
        bzero(buf, MAXLEN_DATA);
    }
    data.msg->cmd_type = RPL_QUERYWORD;
    data.msg->data_len = strlen(buf);
    strncpy(data.msg->data, buf, data.msg->data_len);
    send_msg(data.connfd, (unsigned char *)data.msg);
    return 0;
}


