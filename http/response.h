#pragma once
#include <unordered_map>
#include <string>
#include <cassert>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../buffer/buffer.h"
class Response
{
public:
	Response();
	~Response();
	void Init(const std::string& dir, std::string path,
				bool isKeepAlive = false, int code = -1);
	void MakeResponse(Buffer& buf);
	void MunMap();
	char* File();
	size_t FileLen() const;
	void ErrorCont(Buffer& buf, std::string message);
	int Code()const { return _code; }
private:
	void _AddStateLine(Buffer& buf);
	void _AddHeader(Buffer& buf);
	void _AddContent(Buffer& buf);
	void _ErrorHtml();
	std::string _GetFileType();
private:
	int _code;
	bool _isKeepAlive;
	std::string _path;
	std::string _dir;

	char* _mmFiPtr;
	struct stat _mmFileState;

	static const std::unordered_map<std::string, std::string> TYPE;
	static const std::unordered_map<int, std::string> CODE_STATUS;
	static const std::unordered_map<int, std::string> CODE_PATH;
	
};