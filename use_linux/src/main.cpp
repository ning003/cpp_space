#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "tcpserver.h"
#include "tcpclient.h"

#define MAXLINE 4096

int main(int argc, char **argv)
{
    if (argc < 1)
    {
        printf("Inpupt Run model server  = 1,client = 2");
        return 0;
    }

    int type = atoi(argv[1]);
    if (type == 1)
    {
        TcpServer *server = new TcpServer();
        server->Run();
    }
    else
    {
        TcpClient *client = new TcpClient();
        client->Run();
    }

    return 0;
}
