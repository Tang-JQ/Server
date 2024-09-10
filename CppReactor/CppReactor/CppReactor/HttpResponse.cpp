#include "HttpResponse.h"
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"

#define ResHeaderSize 16


HttpResponse::HttpResponse()
{
	m_statusCode = statusCode::Unknown;
	m_headers.clear();
	m_fileName = string();
	sendDataFunc = nullptr;

}

HttpResponse::~HttpResponse()
{

}

// �����Ӧͷ
void HttpResponse::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty())
	{
		return;
	}
	m_headers.insert(make_pair(key, value));
}

// ��֯http��Ӧ����
void HttpResponse::prepareMsg(Buffer* sendBuf, int socket)
{
	// ״̬��
	char tmp[1024] = { 0 };
	int code = static_cast<int>(m_statusCode);
	sprintf(tmp, "HTTP/1.1 %d %s\r\n", code, m_info.at(code).data());
	sendBuf->appendString(tmp);

	// ��Ӧͷ
	for (auto it = m_headers.begin(); it != m_headers.end(); ++it)
	{
		sprintf(tmp, "%s: %s\r\n", it->first.data(), it->second.data());
		sendBuf->appendString(tmp);
	}

	// ����
	sendBuf->appendString("\r\n");
	sendBuf->sendData(socket);
	// �ظ�������
	sendDataFunc(m_fileName, sendBuf, socket);
}



