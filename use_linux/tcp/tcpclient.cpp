#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include<arpa/inet.h>
#include <pthread.h>
#include <limits>

#include "tcpclient.h"
#include "def.h"

using namespace std;

#define MAXLINE 4096
#define BUFFER_LEN 1024

TcpClient::TcpClient(/* args */)
{
}

TcpClient::~TcpClient()
{
}

void *handle_recv(void *data)
{
    int pipe = *(int *)data;

    // message buffer
    string message_buffer;
    int message_len = 0;

    // one transfer buffer
    char buffer[BUFFER_LEN + 1];
    int buffer_len = 0;

    // receive
    while ((buffer_len = recv(pipe, buffer, BUFFER_LEN, 0)) > 0)
    {
        // to find '\n' as the end of the message
        for (int i = 0; i < buffer_len; i++)
        {
            if (message_len == 0)
                message_buffer = buffer[i];
            else
                message_buffer += buffer[i];

            message_len++;

            if (buffer[i] == '\n')
            {
                // print out the message
                cout <<"<<<"<< message_buffer << endl;

                // new message start
                message_len = 0;
                message_buffer.clear();
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }
    // because the recv() function is blocking, so when the while() loop break, it means the server is offline
    printf("The Server has been shutdown!\n");
    return NULL;

        // // 接收信息
        // int n = recv(sockfd, rec_buf, MAXLINE, 0);
        // rec_buf[n] = '\0';
        // printf("接收消息: %s\n", rec_buf);
}

int TcpClient::Run()
{
    int sockfd, n;
    char recvline[4096], send_buf[4096];
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);

    char port[20] = "127.0.0.1";

    if (inet_pton(AF_INET, port, &servaddr.sin_addr) <= 0)
    {
        return 0;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf(" Run Client =========\n");
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, handle_recv, (void *)&sockfd);
    while (1)
    {
        printf("输入消息：\n");
        char message[BUFFER_LEN + 1];
        cin.get(message, BUFFER_LEN);
        int n = strlen(message);
        if (cin.eof())
        {
            // reset
            cin.clear();
            clearerr(stdin);
            continue;
        }
        // single enter
        else if (n == 0)
        {
            // reset
            cin.clear();
            clearerr(stdin);
        }
        // overflow
        if (n > BUFFER_LEN - 2)
        {
            // reset
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printf("Reached the upper limit of the words!\n");
            continue;
        }
        cin.get();         // remove '\n' in stdin
        message[n] = '\n'; // add '\n'
        message[n + 1] = '\0';
        // the length of message now is n+1
        n++;
        printf("\n");
        // the length of message that has been sent
        int sent_len = 0;
        // calculate one transfer length
        int trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;

        // send message
        printf(">>>>：%s\n",message);
        while (n > 0)
        {
            // int len = send(sockfd, message + sent_len, trans_len, 0);
            int len = send(sockfd, message, trans_len, 0);
            if (len < 0)
            {
                perror("send");
                return -1;
            }
            // if one transfer has not sent the full message, then send the remain message
            n -= len;
            sent_len += len;
            trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;
        }
        // clean the buffer
        memset(message, 0, sizeof(message));
    }

    pthread_cancel(recv_thread);
    close(sockfd);
    return 0;
}


