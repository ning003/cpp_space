#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <queue>

#include "tcpserver.h"

using namespace std;

#define MAXLINE 4096
#define PORT 6666
#define USER_MAX 10
#define BUFFER_LEN 1024

//客户端定义
struct Client
{
    int online;               // to judge whether this user is online
    int fd_id;               // user ID number
    int socket;              // socket to this user
    char name[40]; // name of the user
} client[USER_MAX] = {0};

//互斥锁
pthread_mutex_t num_mutex = PTHREAD_MUTEX_INITIALIZER;


pthread_mutex_t mutex[USER_MAX] = {0};
pthread_cond_t cv[USER_MAX] = {0};

// 玩家线程池
pthread_t chat_thread[USER_MAX] = {0};
pthread_t send_thread[USER_MAX] = {0};

//消息队列
queue<string> message_q[USER_MAX];
int cur_user_num = 0;

TcpServer::TcpServer(/* args */)
{
}

TcpServer::~TcpServer()
{
}

// 每个用户一个发送线程。
void *user_send(void *data)
{
    struct Client *pipe = (struct Client *)data;
    while (1)
    {
        pthread_mutex_lock(&mutex[pipe->fd_id]);
        
        //空消息
        while (message_q[pipe->fd_id].empty())
        {
            printf("玩家%d 等待状态 \n",pipe->fd_id);
            pthread_cond_wait(&cv[pipe->fd_id], &mutex[pipe->fd_id]);
        }
        
        //有消息
        while (!message_q[pipe->fd_id].empty())
        {
            // get the first message from the queue
            string msg_buf = message_q[pipe->fd_id].front();
             printf("玩家%d 发送消息的%s \n",pipe->fd_id,msg_buf.c_str());
            int n = msg_buf.length();
            // calculate one transfer length
            int trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;
            // send the message
            while (n > 0)
            {
                int len = send(pipe->socket, msg_buf.c_str(), trans_len, 0);
                if (len < 0)
                {
                    perror("send");
                    return NULL;
                }
                n -= len;
                msg_buf.erase(0, len); // delete data that has been transported
                trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;
            }
            // delete the message that has been sent
            msg_buf.clear();
            message_q[pipe->fd_id].pop();
        }
        pthread_mutex_unlock(&mutex[pipe->fd_id]);
    }
    return NULL;
}


// get client message and push into queue
void msg_recv(void *data)
{
    printf("\n rec  msg_recv===========\n");
    struct Client *pipe = (struct Client *)data;

    // message buffer
    string message_buffer;
    int message_len = 0;

    // one transfer buffer
    char buffer[BUFFER_LEN + 1];
    int buffer_len = 0;

    // 接收信息
    while ((buffer_len = recv(pipe->socket, buffer, BUFFER_LEN, 0)) > 0)
    {
        // to find '\n' as the end of the message
        for (int i = 0; i < buffer_len; i++)
        {
            // the start of a new message
            if (message_len == 0)
            {
                char temp[100];
                sprintf(temp, "%s:", pipe->name);
                message_buffer = temp;
                message_len = message_buffer.length();
            }

            message_buffer += buffer[i];
            message_len++;

            if (buffer[i] == '\n')
            {
                // send to every client
                
                for (int j = 0; j < USER_MAX; j++)
                {
                    if (client[j].online && client[j].socket != pipe->socket)
                    {
                        pthread_mutex_lock(&mutex[j]);
                        printf("Send to all user rec msg pthread_cond_signal %d\n",j);
                        message_q[j].push(message_buffer);
                        pthread_cond_signal(&cv[j]);
                        pthread_mutex_unlock(&mutex[j]);
                    }
                }
                // new message start
                message_len = 0;
                message_buffer.clear();
            }
        }
        // clear buffer
        buffer_len = 0;
        memset(buffer, 0, sizeof(buffer));
    }
    return;
}


// deal with each client
void *user_chat_chan(void *data)
{
    printf("\n user_chat_chan ===========\n");
    struct Client *pipe = (struct Client *)data;

    // 添加消息队列
    char wel_buf[100];
    sprintf(wel_buf, "Hello %s, Welcome to join the chatroom. Online User Number: %d\n", pipe->name, cur_user_num);
    pthread_mutex_lock(&mutex[pipe->fd_id]);
    message_q[pipe->fd_id].push(wel_buf);
    //触发另外的线程
    pthread_cond_signal(&cv[pipe->fd_id]);
    pthread_mutex_unlock(&mutex[pipe->fd_id]);

    memset(wel_buf, 0, sizeof(wel_buf));
    sprintf(wel_buf, "New User %s join in! Online User Number: %d\n", pipe->name, cur_user_num);
  
    //非本人，发送给所有人
    for (int j = 0; j < USER_MAX; j++)
    {
        if (client[j].online && client[j].socket != pipe->socket)
        {
            pthread_mutex_lock(&mutex[j]);
            message_q[j].push(wel_buf);
            pthread_cond_signal(&cv[j]);
            pthread_mutex_unlock(&mutex[j]);
        }
    }

    // 创建一个socket 对应的发送线程
    pthread_create(&send_thread[pipe->fd_id], NULL, user_send, (void *)pipe);

    //接收数据
    msg_recv(data);

    // because the recv() function is blocking, so when msg_recv() return, it means this user is offline
    pthread_mutex_lock(&num_mutex);
    pipe->online = 0;
    cur_user_num--;
    pthread_mutex_unlock(&num_mutex);
    // printf bye message
    printf("%s left the chatroom. Online Person Number: %d\n", pipe->name, cur_user_num);
    char bye[100];
    sprintf(bye, "%s left the chatroom. Online Person Number: %d\n", pipe->name, cur_user_num);
    // send offline message to other clients
    for (int j = 0; j < USER_MAX; j++)
    {
        if (client[j].online && client[j].socket != pipe->socket)
        {
            pthread_mutex_lock(&mutex[j]);
            message_q[j].push(bye);
            pthread_cond_signal(&cv[j]);
            pthread_mutex_unlock(&mutex[j]);
        }
    }

    pthread_mutex_destroy(&mutex[pipe->fd_id]);
    pthread_cond_destroy(&cv[pipe->fd_id]);
    pthread_cancel(send_thread[pipe->fd_id]);

    return NULL;
}


int TcpServer::Run()
{

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    // char buff[4096];
    char buff[4096], send_buf[4096];
    int n;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, USER_MAX) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("======聊天服务器 启动...... port: .======\n");

    while (1)
    {
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1)
        {
            printf("accept socket error: %s(errno: %d)", strerror(errno), errno);
        }

        if (cur_user_num >= USER_MAX)
        {
            if (send(connfd, "ERROR", strlen("ERROR"), 0) < 0)
                perror("send");
            shutdown(connfd, 2);
            continue;
        }
        else
        {
            if (send(connfd, "OK  ", strlen("OK  "), 0) < 0)
                perror("send");
        }

        // n = recv(connfd, buff, MAXLINE, 0);
        // buff[n] = '\0';
        // printf("收到客户端消息: %s\n", buff);


        //添加用户
        for (int i = 0;i < USER_MAX; i++)
        {
            // printf("ning run ...%d\n",i);
            //未使用客户端结构体
            if(! client[i].online)
            {
                pthread_mutex_lock(&num_mutex);
                memset(client[i].name,0,sizeof(client[i].name));
                std::string str = "ning_" + i;
                strcpy(client[i].name,str.c_str());

                client[i].online = 1;
                client[i].fd_id = i;
                client[i].socket = connfd;

                mutex[i] = PTHREAD_MUTEX_INITIALIZER;
                cv[i] = PTHREAD_COND_INITIALIZER;
                
                cur_user_num++;
                pthread_mutex_unlock(&num_mutex);

                pthread_create(&chat_thread[i], NULL, user_chat_chan, (void *)&client[i]);
                printf("%s join in the chatroom. Online User: %d\n", client[i].name, cur_user_num);

                break;
            }
        }
    }

    for (int i = 0; i < USER_MAX; i++)
        if (client[i].online)
            shutdown(client[i].socket, 2);
    shutdown(connfd, 2);
    close(connfd);
    close(listenfd);
    return 0;
}