#pragma once
#include <iostream>
#include <sys/uio.h>
#include <arpa/inet.h>
#include "request.h"
#include "response.h"
#include "../buffer/buffer.h"

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();

    void init(int fd, const sockaddr_in& addr);

    ssize_t Read(int readErr); //读http发过来的数据
    ssize_t Write(int writeErr);   //向http写数据

    int GetFd() const;
    int GetPort()const;
    sockaddr_in GetAddr()const;
    char* GetIp() const;

    void Close();   //关闭连接

    bool IsKeepAlive(); //
    int ToWriteSize();  //将要写多大

    bool Process();

    static bool isET;
    static std::atomic<int> usercount;
    static const char* src;   //
private:

    int _fd;
    sockaddr_in _addr;

    int _iovcnt;
    iovec _iov[2];

    bool _isClose;

    Buffer _readBuf;
    Buffer _writeBuf;

    Request _request;
    Response _response;
};

