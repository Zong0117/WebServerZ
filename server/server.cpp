#include <iostream>
#include "server.h"
using namespace std;

Server::Server(int port,int mod, int timeoutMs, int threadNum) 
    : _port(port), _timeoutms(timeoutMs),_isClose(false),
    _timer(new MinTimeHeap()), _threadpool(new ThreadPool(threadNum)),_epoll(new Epoll())
{
    _dir = getcwd(nullptr, 256);
    assert(_dir);
    strncat(_dir, "/sources", 16);
    HttpConn::usercount = 0;
    HttpConn::src = _dir;

    InitEventMode(mod);
    if (!_InitSocket())
    {
        _isClose = true;
    }
}

Server::~Server()
{
    close(_listenFd);
    _isClose = true;
}

void Server::ServerStart()
{
    std::cout << "server start" << std::endl;
    
    int timeMs = -1;
    while (!_isClose)
    {
        if (_timeoutms > 0)
        {
            timeMs = _timer->NextTick();
        }
        int eventCount = _epoll->wait(timeMs);
        for (int i = 0; i < eventCount; i++)
        {
			int fd = _epoll->getEventFd(i);
			uint32_t events = _epoll->getEvent(i);
			if (fd == _listenFd)
			{
				_DoAccept();
			}
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                assert(_usrs.count(fd) > 0);
                _CloseConn(&_usrs[fd]);
            }
			else if (events & EPOLLIN)
			{
                assert(_usrs.count(fd) > 0);
				_DoRead(&_usrs[fd]);
			}
			else if (events & EPOLLOUT)
			{
                assert(_usrs.count(fd) > 0);
				_DoWrite(&_usrs[fd]);
			}
			else
			{
                std::cout << "fd erro" <<std::endl;
			}
		} 
    }
}

bool Server::_InitSocket()
{
    int ret;
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
    {
        return false;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int flag = 1;
    ret = setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if(ret == -1)
    {
        close(_listenFd);
        return false;
    }

    ret = bind(_listenFd, (struct sockaddr*)& addr, sizeof(addr));
    if(ret < 0)//草！！！！！！！！！！！！
    {
        close(_listenFd);
        return false;
    }

    ret = listen(_listenFd, 8);
    if(ret < 0)
    {
        close(_listenFd);
        return false;
    }
    ret = _epoll->AddFd(_listenFd, _listenEvent | EPOLLIN);
    if (ret == 0)
    {
        close(_listenFd);
        return false;
    }
    // 把socket设为非阻塞
    _SetFdNoblock(_listenFd);
    return true;
    
}


void Server::InitEventMode(int mod)
{
    _listenEvent = EPOLLRDHUP;
    _connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (mod)
    {
    case 0:
        break;
    case 1:
        _connEvent |= EPOLLET;
        break;
	case 2:
        _listenEvent |= EPOLLET;
		break;
	case 3:
		_listenEvent |= EPOLLET;
		_connEvent |= EPOLLET;
		break;
    default:
		_listenEvent |= EPOLLET;
		_connEvent |= EPOLLET;
        break;
    }
    HttpConn::isET = (_connEvent & EPOLLET);
}

void Server::_AddClient(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    _usrs[fd].init(fd, addr);
    if (_timeoutms > 0)
    {   //绑定client 如果时间到关闭连接
        _timer->Add(fd, _timeoutms, std::bind(&Server::_CloseConn, this, &_usrs[fd]));
    }
    _epoll->AddFd(fd, EPOLLIN |_connEvent);
    _SetFdNoblock(fd);
}


void Server::_DoAccept()    //建立连接
{
	sockaddr_in addr;
	socklen_t len = sizeof(addr);
    do 
    {
		int fd = accept(_listenFd, (sockaddr*)&addr, &len);
		if (fd <= 0)
			return;
		else if (HttpConn::usercount >= MAX_FD)
		{
			return;
		}
        _AddClient(fd, addr);
    } while (_listenEvent & EPOLLET); 
}


void Server::_DoRead(HttpConn* client)  //绑定读操作放入线程池任务队列
{
    assert(client);
    _UpdateTime(client);
    _threadpool->addTask(std::bind(&Server::_ClieRead, this, client));
    //std::cout <<"do read" << std::endl;
}

void Server::_DoWrite(HttpConn* client)
{
    assert(client);
    _UpdateTime(client);
    _threadpool->addTask(std::bind(&Server::_ClieWrite, this, client));
}


void Server::_UpdateTime(HttpConn* client)
{
    assert(client);
    if (_timeoutms > 0)
    {   //timer 更新client 时间
        _timer->Adjust(client->GetFd(), _timeoutms);
    }
}

void Server::_CloseConn(HttpConn* client)
{
    assert(client);
    //log
    _epoll->DelFd(client->GetFd());
    client->Close();
}


void Server::_ClieRead(HttpConn* client)
{
    assert(client);
    int ret = -1;
    int readErr = 0;
    ret = client->Read(readErr);//服务器读fd发的数据
    if (ret <= 0 && readErr != EAGAIN) //反复读取无数据可读
    {
        _CloseConn(client);
        return;
    }
    _Process(client); //更改fd状态
}


void Server::_ClieWrite(HttpConn* client)
{
    assert(client);
    int writeErr = 0;
    int ret = client->Write(writeErr); //
    if (client->ToWriteSize() == 0)//如果
    {
		if (client->IsKeepAlive())
		{
			_Process(client);
			return;
		}
    }
    else if (ret < 0)
    {
        if (writeErr == EAGAIN)
        {
            _epoll->ModFd(client->GetFd(), _connEvent | EPOLLOUT);
            return;
        }
    }
    _CloseConn(client);
}


void Server::_Process(HttpConn* client)
{
    if (client->Process())
    {
        _epoll->ModFd(client->GetFd(), _connEvent | EPOLLOUT);
    }
    else
    {
        _epoll->ModFd(client->GetFd(), _connEvent | EPOLLIN);
    }
}

int Server::_SetFdNoblock(int fd)
{
    assert(fd > 0);
	// 把fd设为非阻塞
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

