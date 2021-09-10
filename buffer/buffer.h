#pragma once
#include <string>
#include <unistd.h>
#include <sys/uio.h>
#include <cstring>
#include <vector>
#include <atomic>
#include <cassert>

class Buffer
{
public:
	Buffer(int bufSize = 1024);
	~Buffer() = default;

	size_t ReadSize() const;	//返回可读字节长度
	size_t WriteSize() const;	//返回可写字节长度
	size_t nullSize() const;	//返回已读完无用空间大小

	const char* BeginRead() const;
	void UpDateReadPos(size_t len);
	void UpDateReadPosUntil(const char* end);

	std::string CleanB4GetStr();
	void CleanBuff();

	const char* BeginWriteConst()const;
	char* BeginWrite();
	void UpDateWritePos(size_t);

	void Append(const char* str, size_t len);
	void Append(const std::string& str);
	void Append(const void* data, size_t len);
	void Append(const Buffer& buf);

	size_t ReadFd(int fd);	//读fd读完添加到buffer
	size_t WriteFd(int fd);	//把buferr里可读长度返回

	void EnsureWrite(size_t len);

private:
	char* _BeginPtr();	//返回头指针
	const char* _CBeginPtr() const;
	void _MakeSpace(size_t len);
	std::vector<char> _buffer;
	std::atomic<std::size_t> _readPos;
	std::atomic<std::size_t> _writePos;
};
