#include "response.h"
#include <iostream>
#include <fstream>
const std::unordered_map<std::string, std::string> Response::TYPE
{
	{ ".html",  "text/html" },
	{ ".xml",   "text/xml" },
	{ ".xhtml", "application/xhtml+xml" },
	{ ".txt",   "text/plain" },
	{ ".rtf",   "application/rtf" },
	{ ".pdf",   "application/pdf" },
	{ ".word",  "application/nsword" },
	{ ".png",   "image/png" },
	{ ".gif",   "image/gif" },
	{ ".jpg",   "image/jpeg" },
	{ ".jpeg",  "image/jpeg" },
	{ ".au",    "audio/basic" },
	{ ".mpeg",  "video/mpeg" },
	{ ".mpg",   "video/mpeg" },
	{ ".avi",   "video/x-msvideo" },
	{ ".gz",    "application/x-gzip" },
	{ ".tar",   "application/x-tar" },
	{ ".css",   "text/css "},
	{ ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> Response::CODE_STATUS
{
	{200, "OK"},
	{400, "Bad Request"},
	{404, "Not Found"},
};

const std::unordered_map<int, std::string> Response::CODE_PATH
{
	{400, "/400.html"},
	{404, "/404.html"},
};

Response::Response()
{
	_code = -1;
	_path = _dir = "";
	_isKeepAlive = false;
	_mmFiPtr = nullptr;
	_mmFileState = { 0 };
}

Response::~Response()
{
	MunMap();
}

void Response::Init(const std::string& srcdir, std::string path, bool isKeepAlive, int code)
{
	assert(srcdir != "");
	if (_mmFiPtr)
	{
		MunMap();
	}
	_code = code;
	_isKeepAlive = isKeepAlive;
	_path = path;
	_dir = srcdir;
	_mmFiPtr = nullptr;
	_mmFileState = { 0 };
}

void Response::MakeResponse(Buffer& buf)
{
	if (stat((_dir + _path).data(), &_mmFileState) < 0
		|| S_ISDIR(_mmFileState.st_mode))
	{
		_code = 404;
	}
	else if (_code == -1)
	{
		_code = 200;
	}
	_ErrorHtml();
	_AddStateLine(buf);
	_AddHeader(buf);
	_AddContent(buf);
}

void Response::MunMap()
{
	if (_mmFiPtr)
	{
		munmap(_mmFiPtr, _mmFileState.st_size);//释放内存映射
		_mmFiPtr = nullptr;
	}
}

char* Response::File()
{
	return _mmFiPtr;
}

size_t Response::FileLen() const
{
	return _mmFileState.st_size;
}

void Response::ErrorCont(Buffer& buf, std::string message)
{
	std::string body;
	std::string status;
	body += "<html><title>Error</title>";
	body += "<body bgcolor=\"ffffff\">";
	if (CODE_STATUS.count(_code) == 1)
	{
		status = CODE_STATUS.find(_code)->second;
	}
	else
	{
		status = "Bad Request";
	}
	body += std::to_string(_code) + " : " + status + "\n";
	body += "<p>" + message + "</p>";
	body += "<hr><em>Zong</em></body></html>";
	buf.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
	buf.Append(body);
}

void Response::_AddStateLine(Buffer& buf)
{
	std::string status;
	if (CODE_STATUS.count(_code) == 1)
	{
		status = CODE_STATUS.find(_code)->second;
	}
	else
	{
		_code = 400;
		status = CODE_STATUS.find(400)->second;
	}
	buf.Append("HTTP/1.1 " + std::to_string(_code) + " " + status + "\r\n");
}

void Response::_AddHeader(Buffer& buf)
{
	buf.Append("Connection: ");
	if (_isKeepAlive)
	{
		buf.Append("keep-alive\r\n");
		buf.Append("keep-alive: max=6, timeout=120\r\n");
	}
	else
	{
		buf.Append("close\r\n");
	}
	buf.Append("Content-type: " + _GetFileType() + "\r\n");
}

void Response::_AddContent(Buffer& buf)
{
	int srcFd = open((_dir + _path).data(), O_RDONLY);
	if (srcFd < 0)
	{
		ErrorCont(buf, "File NotFound!");
		return;
	}

	int* mmptr = static_cast<int*>(mmap(0, _mmFileState.st_size, PROT_READ,
		MAP_PRIVATE, srcFd, 0));
	if (*mmptr == -1)
	{
		ErrorCont(buf, "File NotFound!");
		return;
	}
	_mmFiPtr = (char*)(mmptr);
	close(srcFd);
	buf.Append("Content-length " + std::to_string(_mmFileState.st_size) + "\r\n\r\n");

}

void Response::_ErrorHtml()
{
	if (CODE_PATH.count(_code) == 1)
	{
		_path = CODE_PATH.find(_code)->second;
		stat((_dir + _path).data(), &_mmFileState);
	}
}

std::string Response::_GetFileType()
{
	std::string::size_type index = _path.find_last_of('.');
	if (index == std::string::npos)
	{
		return "text/plain";
	}
	//复制.之后的字符
	std::string type = _path.substr(index);
	if (TYPE.count(type) == 1)
	{
		return TYPE.find(type)->second;
	}
	return "text/plain";
}

