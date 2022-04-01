#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits>

#include "tcpclient.h"
#include "def.h"

using namespace std;

#define MAXLINE 4096
#define CHAT_LEN 1024

TcpClient::TcpClient(/* args */)
{
    m_sockfd = 0;
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
    char buffer[CHAT_LEN + 1];
    int buffer_len = 0;

    // receive
    while ((buffer_len = recv(pipe, buffer, CHAT_LEN, 0)) > 0)
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
                cout << message_buffer << endl;

                // new message start
                message_len = 0;
                message_buffer.clear();
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }

    printf("服务器关闭\n");
    close(pipe);
    return NULL;
}

int TcpClient::Run()
{
    int n;
    char recvline[4096], send_buf[4096];
    struct sockaddr_in servaddr;

    if ((m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

    if (connect(m_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf(" Run Client =========\n");
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, handle_recv, (void *)&m_sockfd);
    while (1)
    {
        MsgData msg;
        msg.msg_type = MSG_NAME;

        printf("输入消息：\n");
        // char message[CHAT_LEN + 1];
        cin.get(msg.data, MSG_LEN);
        int input_len = strlen(msg.data);
        if (cin.eof())
        {
            // reset
            cin.clear();
            clearerr(stdin);
            continue;
        }
        // single enter
        else if (input_len == 0)
        {
            // reset
            cin.clear();
            clearerr(stdin);
        }
        // overflow
        if (input_len > CHAT_LEN - 2)
        {
            // reset
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            printf("Reached the upper limit of the words!\n");
            continue;
        }
        cin.get();                  // remove '\n' in stdin
        msg.data[input_len] = '\n'; // add '\n'
        msg.data[input_len + 1] = '\0';
        input_len++;
        printf("\n");

        // send message
        printf(">>>>：%s\n", msg.data);
        SendMsg(msg, input_len);
        // clean the buffer
        memset(msg.data, 0, sizeof(msg.data));
    }

    pthread_cancel(recv_thread);
    close(m_sockfd);
    return 0;
}

int TcpClient::SendMsg(MsgData msg, int msg_len)
{
    //组装协议
    string send_str = msg.data;
    send_str.insert(0, "|");
    send_str.insert(0, to_string(msg.msg_type));
    msg_len = msg_len + 2;
  
    int offset_len = 0;
    int send_len = CHAT_LEN > msg_len ? msg_len : CHAT_LEN;

    // send message
    printf(">>>>：%s\n", send_str.c_str());
    while (msg_len > 0)
    {
        int ok_len = send(m_sockfd, send_str.c_str() + offset_len, send_len, 0);
        if (ok_len < 0)
        {
            perror("send");
            return -1;
        }

        //按1024为一段发送
        msg_len -= ok_len;
        offset_len += ok_len;
        send_len = CHAT_LEN > msg_len ? msg_len : CHAT_LEN;
    }
}
