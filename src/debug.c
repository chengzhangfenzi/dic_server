/*************************************************************************
	> File Name: debug.c
	> Author: 
	> Mail: 
	> Created Time: 2014年08月24日 星期日 16时11分04秒
 ************************************************************************/

#include<server.h>

char *discmdtype(uint16_t cmd_type)
{
    switch(cmd_type){
        case REQ_REGISTER:
            return "REQ_REGISTER";
            break;
        case REQ_LOGIN:
            return "REQ_LOGIN";
            break;
        case REQ_QUERYWORD:
            return "REQ_QUERYWORD";
            break;
        case REQ_HISTORY:
            return "REQ_REGISTER";
            break;
        case REQ_EXIT:
            return "REQ_EXIT";
            break;
        case RPL_REGISTER:
            return "RPL_REGISTER";
            break;
        case RPL_LOGIN:
            return "RPL_LOGIN";
            break;
        case RPL_QUERYWORD:
            return "RPL_QUERYWORD";
            break;
        case RPL_HISTORY:
            return "RPL_HISTORY";
            break;
        default :
            return NULL;
    }
}

char *disstate(uint16_t state)
{
    switch(state){
         
        case RTL_SUCCESS:
        return "成功";
        break;
        case RTL_USEREXIST:
        return "用户已存在";
        break;
        case RTL_NUSEREXIST: 
        return "用户不存在";
        break;
        case RTL_INSERTERR:
        return "插入用户失败";
        break;
        case RTL_WRONGPASSWD:
        return "用户密码错误";
        break;
        case RTL_RIGHTPASSWD: 
        return "用户密码正确";
        break;
        case RTL_USERONLINE: 
        return "用户已在线";
        break;
        case RTL_NOFINDWORD:  
        return "未找到单词";
        break;
        case RTL_NOHISTORY:  
        return "查找用户历史失败"; 
        break;
        case RTL_DBERR:      
        return "查找数据库查询出错";
        break;
    default:
        printf("%hd", state);
        return " ";
    }
    
}


void coutmsg(unsigned char * data){
    struct XProtocol *msg = (struct XProtocol *)data;
    printf("\ncmd_type = %s\n", discmdtype(msg->cmd_type));
    printf("state= %s\n", disstate(msg->state));
    printf("username = %s\n", msg->usrname);
    printf("data = %s\n", msg->data);
    printf("data_len = %d\n", msg->data_len);
}
