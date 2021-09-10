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
    bool _InitSocket();  //建立socket
    void InitEventMode(int mod);
    void _AddClient(int fd, sockaddr_in addr);   //添加socket
    
    void _DoAccept();   //建立连接accept
    void _DoRead(HttpConn* client);     //向线程池添加任务
    void _DoWrite(HttpConn* client);    //向线程池添加写任务
    void _UpdateTime(HttpConn* client); //读写操作完毕更新client时间
    
    void _CloseConn(HttpConn* client);  //关闭一个连接

    void _ClieRead(HttpConn* client);   //读client发送的数据读完更改状态
    void _ClieWrite(HttpConn* client);      //写完更改状态
    void _Process(HttpConn* client);    //更改状态函数 in，out

    int _SetFdNoblock(int fd);  //设置为非阻塞io
private:
    const int MAX_FD = 65535;

private:
    int _port;  //端口
    int _timeoutms; //微妙
    int _listenFd;  //listen
    bool _isClose;  //判断fd是否关闭
    char* _dir; //保存资源路径

    uint32_t _listenEvent;
    uint32_t _connEvent;
private:
    std::unique_ptr<MinTimeHeap> _timer;    //时间
    std::unique_ptr<ThreadPool> _threadpool;    //线程池
    std::unique_ptr<Epoll> _epoll;  //
    std::unordered_map<int, HttpConn> _usrs;    //用户
};

