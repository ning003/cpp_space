#pragma onece
#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include "def.h"

class TcpClient
{
private:
    int m_sockfd;

public:
    struct MsgData
    {
        int msg_type;
        char data[MSG_LEN];
    };

    TcpClient(/* args */);
    ~TcpClient();

    int SendMsg(MsgData msg,int msg_len);
    int Run();
};

#endif // TCP_CLIENT_H_
