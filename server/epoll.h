#pragma once
#include <sys/epoll.h>
#include <vector>
#include <cassert>
#include <unistd.h>

class Epoll
{
public:
	explicit Epoll(int maxEvent = 1024);
	~Epoll();

	bool AddFd(int listenFd, uint32_t events);	
	bool DelFd(int listenFd);
	bool ModFd(int listenFd, uint32_t events);
	int wait(int timeout = -1);
	int getEventFd(size_t pos);
	uint32_t getEvent(size_t pos);
private:
	int _epollFd;
	std::vector<struct epoll_event> _events;
};