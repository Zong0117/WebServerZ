#include "buffer.h"
#include <iostream>
Buffer::Buffer(int bufSize) : _buffer(bufSize), _readPos(0), _writePos(0) {}


size_t Buffer::ReadSize() const
{
	return _writePos - _readPos;
}

size_t Buffer::WriteSize() const
{
	return _buffer.size() - _writePos;
}

size_t Buffer::nullSize() const
{
	return _readPos;	//可读位置前无用空间大小
}


const char* Buffer::BeginRead() const
{
	return _CBeginPtr() + _readPos;
}

void Buffer::UpDateReadPos(size_t len)
{
	assert(len <= ReadSize());
	_readPos += len;
}

void Buffer::UpDateReadPosUntil(const char* end)
{
	assert(BeginRead() <= end);
	UpDateReadPos(end - BeginRead());
}

std::string Buffer::CleanB4GetStr()
{
	std::string str(BeginRead(), ReadSize());
	CleanBuff();
	return str;
}

void Buffer::CleanBuff()
{
	bzero(&_buffer[0], _buffer.size());
	_readPos = 0;
	_writePos = 0;
}

const char* Buffer::BeginWriteConst() const
{
	return _CBeginPtr() + _writePos;
}

char* Buffer::BeginWrite()
{
	return _BeginPtr() + _writePos;
}

void Buffer::UpDateWritePos(size_t len)
{
	_writePos += len;
}

size_t Buffer::ReadFd(int fd)//读fd发的数据
{   
	char buf[65535];	//栈上buf
	iovec iov[2];	//分两次读
	const size_t writesiz = WriteSize();

	iov[0].iov_base = BeginWrite();	//先向buffer读in数据
	iov[0].iov_len = writesiz;	//如果过大再向栈上buf读in数据
	iov[1].iov_base = buf;			//
	iov[1].iov_len = sizeof(buf);
	
	const ssize_t len = readv(fd, iov, 2);
	if (len < 0)
	{
		std::cout << "reafd err" << std::endl;
	}
	else if (static_cast<size_t>(len) <= WriteSize())	//有空间正常写入
	{
		_writePos += len;
	}
	else	//空间不足扩容再写入
	{
		_writePos = _buffer.size();
		Append(buf, len - writesiz);
	}	

	return len;		
}

size_t Buffer::WriteFd(int fd)
{
	size_t readSize = ReadSize(); 
	ssize_t len = write(fd, BeginRead(), readSize);
	if (len < 0)
	{
		return len;
	}
	_readPos += len;
	return len;
}

void Buffer::EnsureWrite(size_t len)
{
	if (WriteSize() < len)
	{
		_MakeSpace(len);
	}
	assert(WriteSize() >= len);
}

char* Buffer::_BeginPtr()
{
	return &*_buffer.begin();
}

const char* Buffer::_CBeginPtr() const
{
	return &*_buffer.begin();
}

void Buffer::_MakeSpace(size_t len)
{
	if (WriteSize() + nullSize() < len)
	{
		_buffer.resize(_writePos + len + 1);
	}
	else
	{
		size_t readsiz = ReadSize();
		std::copy(_BeginPtr() + _readPos, _BeginPtr() + _writePos, _BeginPtr());
		_readPos = 0;
		_writePos = _readPos + readsiz;
		assert(readsiz == ReadSize());
	}
}

void Buffer::Append(const char* str, size_t len)
{
	assert(str);
	EnsureWrite(len);

	std::copy(str, str + len, BeginWrite());

	UpDateWritePos(len);

}

void Buffer::Append(const std::string& str)
{
	Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len)
{
	assert(data);
	Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const Buffer& buf)
{
	Append(BeginRead(), buf.ReadSize());
}
