#include "epoll.h"
Epoll::Epoll(int maxEvent) : _epollFd(epoll_create(1024)), _events(maxEvent)
{
	assert(_epollFd >= 0 && _events.size() > 0);
}

Epoll::~Epoll()
{
	close(_epollFd);
}

bool Epoll::AddFd(int listenFd, uint32_t events)
{
	if (listenFd < 0) return false;
	epoll_event ev;
	ev.data.fd = listenFd;
	ev.events = events;
	return 0 == epoll_ctl(_epollFd, EPOLL_CTL_ADD, listenFd, &ev);
}

bool Epoll::DelFd(int listenFd)
{
	if (listenFd < 0)	return false;
	epoll_event ev;
	//´ÓºìºÚÊ÷ÒÆ³ýfd½Úµã
	return 0 == epoll_ctl(_epollFd, EPOLL_CTL_DEL, listenFd, &ev);
}

bool Epoll::ModFd(int listenFd, uint32_t events)
{
	if (listenFd < 0) return false;
	epoll_event ev;
	ev.data.fd = listenFd;
	ev.events = events;
	return 0 == epoll_ctl(_epollFd, EPOLL_CTL_MOD, listenFd, &ev);
}

int Epoll::wait(int timeout)
{
	return epoll_wait(_epollFd, &_events[0], static_cast<int>(_events.size()), timeout);
}

int Epoll::getEventFd(size_t pos)
{
	assert(pos < _events.size() && pos >= 0);
	return _events[pos].data.fd;
}

uint32_t Epoll::getEvent(size_t pos)
{
	assert(pos < _events.size() && pos >= 0);
	return _events[pos].events;
}

