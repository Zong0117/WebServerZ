#include "http.h"

std::atomic<int> HttpConn::usercount;
bool HttpConn::isET;
const char* HttpConn::src;

HttpConn::HttpConn() :
	_fd(-1), _addr({0}), _isClose(true){}

HttpConn::~HttpConn()
{
	Close();
}

void HttpConn::init(int fd, const sockaddr_in& addr)
{
	assert(fd > 0);
	usercount++;
	_fd = fd;
	_addr = addr;
	_readBuf.CleanBuff();
	_writeBuf.CleanBuff();
	_isClose = false;
	//log
}

ssize_t HttpConn::Read(int readErr)
{
	ssize_t	len = -1;
	do
	{
		len = _readBuf.ReadFd(_fd);
		if (len <= 0)
			break;
	} while (isET);
	std::cout << isET << std::endl;
	return len;
}

ssize_t HttpConn::Write(int writeErr)
{
	ssize_t len = -1;
	do
	{
		len = writev(_fd, _iov, _iovcnt);

		if (len <= 0)
		{
			break;
		}

		if (_iov[0].iov_len + _iov[1].iov_len == 0)
		{
			break;
		}
		else if (len > _iov[0].iov_len)
		{
			//如果写长度大于iov【0】的长度让iov【1】接上多出来的
			_iov[1].iov_base = (uint8_t*)_iov[1].iov_base + (len - _iov[0].iov_len);
			_iov[1].iov_len -= (len - _iov[0].iov_len);

			if (_iov[0].iov_len)
			{
				_writeBuf.CleanBuff();
				_iov[0].iov_len = 0;
			}
		}
		else
		{	//更新ioevc参数
			_iov[0].iov_base = (uint8_t*)_iov[0].iov_base + len;
			_iov[0].iov_len -= len;
			_writeBuf.UpDateReadPos(len);
		}
	} while (isET || ToWriteSize() > 10240);
	return len;
}

int HttpConn::GetFd() const
{
	return _fd;
}

int HttpConn::GetPort() const
{
	return _addr.sin_port;
}

sockaddr_in HttpConn::GetAddr() const
{
	return _addr;
}

char* HttpConn::GetIp() const
{
	return inet_ntoa(_addr.sin_addr);
}

void HttpConn::Close()
{
	_response.MunMap();
	if (_isClose == false)
	{
		_isClose = true;
		usercount--;
		close(_fd);
	}
}


bool HttpConn::IsKeepAlive()
{
	return _request.IsKeepAlive();
}



int HttpConn::ToWriteSize()
{
	return _iov[0].iov_len + _iov[1].iov_len;
}

bool HttpConn::Process()
{
	_request.Init();
	if (_readBuf.ReadSize() <= 0)
	{
		return false;
	}
	else if (_request.Parse(_readBuf))
	{
		_response.Init(src, _request.path(), _request.IsKeepAlive(), 200);
	}
	else
	{
		_response.Init(src, _request.path(), false, 400);
	}

	_response.MakeResponse(_writeBuf);
	_iov[0].iov_base = const_cast<char*>(_writeBuf.BeginRead());
	_iov[0].iov_len = _writeBuf.ReadSize();
	_iovcnt = 1;

	if (_response.FileLen() > 0 && _response.File())
	{
		_iov[1].iov_base = _response.File();
		_iov[1].iov_len = _response.FileLen();
		_iovcnt = 2;
		
	}
	return true;
}




