#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <regex>


#include "../buffer/buffer.h"
class Request
{
public:
	enum STATE
	{
		REQUEST_LINE,
		HEADER,
		BODY,
		SPACE,
	};
	enum HTTP_CODE
	{
		NO_REQUEST = 0,
		GET_REQUEST,
		BAD_REQUEST,
		NO_RESOURSE,
		FILE_REQUEST,
		CLOSE_CONN,
	};
public:

	Request();
	~Request() = default;
	void Init();
	bool Parse(Buffer& buf);

	std::string path()const;
	std::string& path();
	std::string Method()const;
	std::string Version()const;
	//std::string GetPost(const std::string& key);

	bool IsKeepAlive() const;

private:
	bool _ParseRequest(const std::string&);
	void _ParseHeader(const std::string&);
	void _ParseBody(const std::string&);

	void _ParsePath();
	//void _ParsePost();


private:

	STATE _state;
	std::string _method, _path, _version, _body;
	std::unordered_map<std::string, std::string> _header;
	std::unordered_map<std::string, std::string> _post;

	static const std::unordered_set<std::string> HTML;

};