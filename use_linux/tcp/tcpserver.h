#pragma onece
#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#define SREVER_PORT 8888

class TcpServer
{
private:
    /* data */
public:
    TcpServer(/* args */);
    ~TcpServer();

    int Run();
};

#endif //TCP_SERVER_H_
