#include "request.h"

const std::unordered_set<std::string> Request::HTML{
	"/index","/video","/images",
};

Request::Request()
{
	Init();
}

void Request::Init()
{
	_method = _path = _version = _body = "";
	_state = REQUEST_LINE;
	_header.clear();
	_post.clear();
}

bool Request::Parse(Buffer& buf)
{
	const char END_FLAG[] = "\r\n";
	if (buf.ReadSize() <= 0)
	{
		return false;
	}
	while (buf.ReadSize() && _state != SPACE)
	{
		//从可读位置找到\r\n
		const char* lineEnd = std::search(buf.BeginRead(), buf.BeginWriteConst(),
			END_FLAG, END_FLAG + 2);
		//可读位置到\r\n
		std::string line(buf.BeginRead(), lineEnd);
		switch (_state)
		{
		case REQUEST_LINE:
			if (!_ParseRequest(line))//如果不是请求行
			{
				return false;
			}
			_ParsePath();	//解析文件路径
			break;

		case HEADER:
			_ParseHeader(line);	//解析请求头
			if (buf.ReadSize() <= 2)//如果可读数小于2
			{						//就是到末尾了
				_state = SPACE;
			}
			break;

		case BODY:
			_ParseBody(line);
			break;

		default:
			break;
		}

		if (lineEnd == buf.BeginWrite())
		{
			break;
		}
		buf.UpDateReadPosUntil(lineEnd + 2);
	}
	return true;
}

std::string Request::path() const
{
	return _path;
}

std::string& Request::path()
{
	return _path;
}

std::string Request::Method() const
{
	return _method;
}

std::string Request::Version() const
{
	return _version;
}

//std::string Request::GetPost(const std::string& key)
//{
//	if (_method == "POST" && _header["Content-Type"] == "application/x-www-form-urlencoded")
//	{
//
//	}
//}

bool Request::IsKeepAlive() const
{
	if (_header.count("Connection") == 1)
	{
		return _header.find("Connection")->second
			== "Keep-alive" && _version == "1.1";
	}
	return false;
}

bool Request::_ParseRequest(const std::string& line)
{
	std::regex pat("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
	std::smatch match;
	if (regex_match(line, match, pat))
	{
		_method = match[1];
		_path = match[2];
		_version = match[3];
		_state = SPACE;
		return true;
	}
	return false;
}

void Request::_ParseHeader(const std::string& line)
{
	std::regex pat("^([^:]*): ?(.*)$");
	std::smatch match;
	if (regex_match(line, match, pat))
	{
		_header[match[1]] = match[2];
	}
	else
	{
		_state = BODY;
	}
}

void Request::_ParseBody(const std::string& line)
{
	_body = line;
	//_ParsePost();
	_state = SPACE;
}

void Request::_ParsePath()
{
	if (_path == "/")
	{
		_path = "/index.html";
	}
	else
	{
		for (auto& i : HTML)
		{
			if (i == _path)
			{
				_path += ".html";
				break;
			}
		}
	}
}

//void Request::_ParsePost()
//{
//
//}






