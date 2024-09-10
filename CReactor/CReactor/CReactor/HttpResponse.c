#include "HttpResponse.h"
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"

#define ResHeaderSize 16

// ��ʼ��
struct HttpResponse* httpResponseInit()
{
	struct HttpResponse* response = (struct HttpResponse*)malloc(sizeof(struct HttpResponse));
	response->headerNum = 0;
	int size = sizeof(struct ResponseHeader) * ResHeaderSize;
	response->headers = (struct ResponseHeader*)malloc(size);
	response->statusCode = Unknown;
	// ��ʼ������
	bzero(response->headers, size);
	bzero(response->statusMsg, sizeof(response->statusMsg));
	bzero(response->statusMsg, sizeof(response->fileName));

	// ����ָ��
	response->sendDataFunc = NULL;
	return response;
}


// ����
void httpResponseDestroy(struct HttpResponse* response)
{
	if (response != NULL)
	{
		free(response->headers);
		free(response);
	}
}

// ������Ӧͷ
void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value)
{
	if (response == NULL || key == NULL || value == NULL)
	{
		return;
	}
	strcpy(response->headers[response->headerNum].key, key);
	strcpy(response->headers[response->headerNum].value, value);
	response->headerNum++;
}

// ��֯http��Ӧ����  
void httpResponsePrepareMsg(struct HttpResponse* response, struct Buffer* sendBuf, int socket)
{
	// ״̬��
	char tmp[1024] = { 0 };
	sprintf(tmp, "HTTP/1.1 %d %s\r\n", response->statusCode, response->statusMsg);
	bufferAppendString(sendBuf, tmp);

	// ��Ӧͷ
	for (int i = 0; i < response->headerNum; ++i)
	{
		sprintf(tmp, "%s: %s\r\n", response->headers[i].key, response->headers[i].value);
		bufferAppendString(sendBuf, tmp);
	}

	// ����
	bufferAppendString(sendBuf, "\r\n");
	bufferSendData(sendBuf, socket);
	// �ظ�������
	response->sendDataFunc(response->fileName, sendBuf, socket);
}
