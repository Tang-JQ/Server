#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <stdbool.h>

// 请求头键值对
struct RequestHeader
{
	char* key;
	char* value;
};

// 当前的解析状态
enum HttpRequestState
{
	ParseReqLine,			// 解析的请求行
	ParseReqHeaders,			// 解析的请求头
	ParseReqBody,			// 解析的请求数据
	ParseReqDone			// 请求协议已结束
};

// 定义http请求结构体（四部分：请求行、请求头、空行、请求数据
struct HttpRequest
{
	// 请求行
	char* method;			// get、post
	char* url;				// 服务器端文件地址
	char* version;			// 版本
	// 请求头 键值对 有多个，定义一个数组,及大小
	struct RequestHeader* reqHeaders;
	int reqHeadersNum;
	// http请求状态
	enum HttpRequestState curState;
};

// 初始化
struct HttpRequest* httpRequestInit();

//  重置
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);
void httpRequestDestroy(struct HttpRequest* req);

// 获取处理状态
enum HttpRequestState httpRequestState(struct HttpRequest* req);

// 添加请求头
void httpRequestAddHeader(struct HttpRequest* req, const char* key, const char* value);

// 根据key得到请求头的value
char* httpRequestGetHeader(struct HttpRequest* req, const char* key);

// 解析请求行
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);

// 解析请求头
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);

// 解析http请求协议
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,
	struct HttpResponse* response, struct Buffer* sendBuf, int socket);

// 处理http请求
bool processHttpRequest(struct HttpRequest* request, struct HttpResponse* response);

// 解码字符串
void decodeMsg(char* to, char* from);

// 根据文件的后缀或者名字，获取文件的类型const char* type
const char* getFileType(const char* name);

void sendFile(const char* fileName, struct Buffer* sendBuf, int cfd);
void sendDir(const char* dirName, struct Buffer* sendBuf, int cfd);