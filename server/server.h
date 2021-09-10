#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cassert>
#include <netinet/in.h>
#include <cstring>
#include "../timer/timer.h"
#include "../threadpool/threadpool.h"
#include "../http/http.h"
#include "epoll.h"
class Server
{
public:
    Server(int port, int mod, int timeoutMs,  int threadNum);
    ~Server();
    void ServerStart();
private:
    bool _InitSocket();  //����socket
    void InitEventMode(int mod);
    void _AddClient(int fd, sockaddr_in addr);   //���socket
    
    void _DoAccept();   //��������accept
    void _DoRead(HttpConn* client);     //���̳߳��������
    void _DoWrite(HttpConn* client);    //���̳߳����д����
    void _UpdateTime(HttpConn* client); //��д������ϸ���clientʱ��
    
    void _CloseConn(HttpConn* client);  //�ر�һ������

    void _ClieRead(HttpConn* client);   //��client���͵����ݶ������״̬
    void _ClieWrite(HttpConn* client);      //д�����״̬
    void _Process(HttpConn* client);    //����״̬���� in��out

    int _SetFdNoblock(int fd);  //����Ϊ������io
private:
    const int MAX_FD = 65535;

private:
    int _port;  //�˿�
    int _timeoutms; //΢��
    int _listenFd;  //listen
    bool _isClose;  //�ж�fd�Ƿ�ر�
    char* _dir; //������Դ·��

    uint32_t _listenEvent;
    uint32_t _connEvent;
private:
    std::unique_ptr<MinTimeHeap> _timer;    //ʱ��
    std::unique_ptr<ThreadPool> _threadpool;    //�̳߳�
    std::unique_ptr<Epoll> _epoll;  //
    std::unordered_map<int, HttpConn> _usrs;    //�û�
};

