#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <stdbool.h>

// ����ͷ��ֵ��
struct RequestHeader
{
	char* key;
	char* value;
};

// ��ǰ�Ľ���״̬
enum HttpRequestState
{
	ParseReqLine,			// ������������
	ParseReqHeaders,			// ����������ͷ
	ParseReqBody,			// ��������������
	ParseReqDone			// ����Э���ѽ���
};

// ����http����ṹ�壨�Ĳ��֣������С�����ͷ�����С���������
struct HttpRequest
{
	// ������
	char* method;			// get��post
	char* url;				// ���������ļ���ַ
	char* version;			// �汾
	// ����ͷ ��ֵ�� �ж��������һ������,����С
	struct RequestHeader* reqHeaders;
	int reqHeadersNum;
	// http����״̬
	enum HttpRequestState curState;
};

// ��ʼ��
struct HttpRequest* httpRequestInit();

//  ����
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);
void httpRequestDestroy(struct HttpRequest* req);

// ��ȡ����״̬
enum HttpRequestState httpRequestState(struct HttpRequest* req);

// �������ͷ
void httpRequestAddHeader(struct HttpRequest* req, const char* key, const char* value);

// ����key�õ�����ͷ��value
char* httpRequestGetHeader(struct HttpRequest* req, const char* key);

// ����������
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);

// ��������ͷ
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);

// ����http����Э��
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,
	struct HttpResponse* response, struct Buffer* sendBuf, int socket);

// ����http����
bool processHttpRequest(struct HttpRequest* request, struct HttpResponse* response);

// �����ַ���
void decodeMsg(char* to, char* from);

// �����ļ��ĺ�׺�������֣���ȡ�ļ�������const char* type
const char* getFileType(const char* name);

void sendFile(const char* fileName, struct Buffer* sendBuf, int cfd);
void sendDir(const char* dirName, struct Buffer* sendBuf, int cfd);