#ifndef DEF_H_
#define DEF_H_


//枚举
enum{
    TCP_SERVER = 1,
    TCP_CLIENT = 2,
};

enum MSG_TYPE {
    MSG_NAME,
    MSG_STR,
} ;

#define SREVER_PORT 7777

//定义变量
static const int MSG_LEN = 1000;
static const int CHAT_LEN = 1024;


#endif //DEF_H_
