#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <map>
using namespace std;



// 当前的解析状态
enum class ProcessState:char
{
	ParseReqLine,			// 解析的请求行
	ParseReqHeaders,		// 解析的请求头
	ParseReqBody,			// 解析的请求数据
	ParseReqDone			// 请求协议已结束
};

// 定义http请求结构体（四部分：请求行、请求头、空行、请求数据
class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	//  重置
	void reset();

	// 获取处理状态
	inline ProcessState getState()
	{
		return m_curState;
	}

	// 添加请求头
	void addHeader(const string key, const string value);

	// 根据key得到请求头的value
	string getHeader(const string key);

	// 解析请求行
	bool parseRequestLine(Buffer* readBuf);

	// 解析请求头
	bool parseRequestHeader(Buffer* readBuf);

	// 解析http请求协议
	bool parseRequest(Buffer* readBuf,
		HttpResponse* response, Buffer* sendBuf, int socket);

	// 处理http请求
	bool processHttpRequest(HttpResponse* response);

	// 解码字符串
	string decodeMsg(string from);

	// 根据文件的后缀或者名字，获取文件的类型const char* type
	const string getFileType(string name);

	static void sendFile(string fileName, Buffer* sendBuf, int cfd);
	static void sendDir(string dirName, Buffer* sendBuf, int cfd);

	inline void setMethod(string method)
	{
		m_method = method;
	}
	inline void seturl(string url)
	{
		m_url = url;
	}
	inline void setVersion(string version)
	{
		m_version = version;
	}

	inline void setState(ProcessState state)
	{
		m_curState = state;
	}



private:
	char* splitRequestLine(const char* start, const char* end,
		const char* sub, function<void(string)>callback);
	int hexToDec(char c);


private:
	// 请求行
	string m_method;			// get、post
	string m_url;				// 服务器端文件地址
	string m_version;			// 版本
	// 请求头 键值对 有多个，定义一个数组,及大小
	map<string, string> m_reqHeaders;
	// http请求状态
	ProcessState m_curState;
};

