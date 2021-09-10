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

	size_t ReadSize() const;	//���ؿɶ��ֽڳ���
	size_t WriteSize() const;	//���ؿ�д�ֽڳ���
	size_t nullSize() const;	//�����Ѷ������ÿռ��С

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

	size_t ReadFd(int fd);	//��fd������ӵ�buffer
	size_t WriteFd(int fd);	//��buferr��ɶ����ȷ���

	void EnsureWrite(size_t len);

private:
	char* _BeginPtr();	//����ͷָ��
	const char* _CBeginPtr() const;
	void _MakeSpace(size_t len);
	std::vector<char> _buffer;
	std::atomic<std::size_t> _readPos;
	std::atomic<std::size_t> _writePos;
};
