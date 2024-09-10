#define _GNU_SOURCE
#include "HttpRequest.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "TcpConnection.h"
#include <assert.h>
#include <ctype.h>


#define HeaderSize 12

// ��ʼ��
struct HttpRequest* httpRequestInit()
{
	struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
	httpRequestReset(request);
	request->reqHeaders = (struct RequestHeader*)malloc(sizeof(struct RequestHeader) * HeaderSize);
	return request;
}

//  ����
void httpRequestReset(struct HttpRequest* request)
{
	request->curState = ParseReqLine;
	request->url = NULL;
	request->method = NULL;
	request->version = NULL;
	request->reqHeadersNum = 0;		// ����޿���Ԫ�أ��´�ʹ�ô�0�ſ�ʼ��
}

void httpRequestResetEx(struct HttpRequest* req)
{
	free(req->url);
	free(req->method);
	free(req->version);
	if (req->reqHeaders != NULL)
	{
		for (int i = 0; i < req->reqHeadersNum; ++i)
		{
			free(req->reqHeaders[i].key);
			free(req->reqHeaders[i].value);
		}
	}
	httpRequestReset(req);
}

void httpRequestDestroy(struct HttpRequest* req)
{
	if (req != NULL)
	{
		httpRequestResetEx(req);
		free(req);
	}

}

// ��ȡ����״̬
enum HttpRequestState httpRequestState(struct HttpRequest* req)
{
	return req->curState;
}

void httpRequestAddHeader(struct HttpRequest* req, const char* key, const char* value)
{
	req->reqHeaders[req->reqHeadersNum].key = (char*)key;
	req->reqHeaders[req->reqHeadersNum].value = (char*)value;
	req->reqHeadersNum++;
}

// ����key�õ�����ͷ��value
char* httpRequestGetHeader(struct HttpRequest* req, const char* key)
{
	if (req != NULL)
	{
		for (int i = 0; i < req->reqHeadersNum; ++i)
		{
			if (strncasecmp(req->reqHeaders[i].key, key, strlen(key)) == 0)
			{
				return req->reqHeaders[i].value;
			}
		}
	}
	return NULL;
}


// ����������
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf)
{
	// ����������, �����ַ���������ַ(\r\n��ַ��
	char* end = bufferFindCRLF(readBuf);

	// �����ַ�����ʼ��ַ
	char* start = readBuf->data + readBuf->readPos;

	// �������ܳ���
	int lineSize = end - start;

	if (lineSize)
	{
		// get /xxx/xx.txt http/1.1
		// ����ʽ
		char* space = memmem(start, lineSize, " ", 1);		// Ѱ�Ҵ�start��ʼsie��С���ո�1��С������ʼλ��
		assert(space != NULL);
		int methodSize = space - start;
		request->method = (char*)malloc(methodSize + 1);	// +1��Ϊ�˱���\0
		strncpy(request->method, start, methodSize);		// ��size��С��start�ַ��������Ƶ�method��
		request->method[methodSize] = '\0';

		// ����ľ�̬��Դ
		start = space + 1;
		space = memmem(start, end - start, " ", 1);		// Ѱ�Ҵ�start��ʼsie��С���ո�1��С������ʼλ��
		assert(space != NULL);
		int urlSize = space - start;
		request->url = (char*)malloc(urlSize + 1);	// +1��Ϊ�˱���\0
		strncpy(request->url, start, urlSize);		// ��size��С��start�ַ��������Ƶ�method��
		request->url[urlSize] = '\0';

		// http�汾
		start = space + 1;
		request->version = (char*)malloc(end - start + 1);	// +1��Ϊ�˱���\0
		strncpy(request->version, start, end - start);		// ��size��С��start�ַ��������Ƶ�method��
		request->version[end - start] = '\0';

		// Ϊ��������ͷ��׼��
		readBuf->readPos += lineSize;
		readBuf->readPos += 2;
		// �޸�״̬
		request->curState = ParseReqHeaders;
		return true;
	}
	return false;
}

// ��������ͷ --- �ú�����������ͷ�е�һ��
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf)
{
	char* end = bufferFindCRLF(readBuf);
	if (end != NULL)
	{
		char* start = readBuf->data + readBuf->readPos;
		int lineSize = end - start;
		// ���ڣ������ַ���
		char* middle = memmem(start, lineSize, ": ", 2);
		if (middle != NULL)
		{
			char* key = malloc(middle - start + 1);
			strncpy(key, start, middle - start);
			key[middle - start] = '\0';

			char* value = malloc(end - (middle + 2) + 1);
			strncpy(value, (middle + 2), end - (middle + 2));
			value[end - (middle + 2)] = '\0';

			httpRequestAddHeader(request, key, value);

			// �ƶ������ݵ�λ��
			readBuf->readPos += lineSize;
			readBuf->readPos += 2;
		}
		else
		{
			// ����ͷ����������
			readBuf->readPos += 2;
			// �޸Ľ���״̬, ����post���󣬰���get������
			request->curState = ParseReqDone;
		}
		return true;
	}
	return false;
}


// ����http����Э��
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,
	struct HttpResponse* response, struct Buffer* sendBuf, int socket)
{
	bool flag = true;
	while (request->curState != ParseReqDone)
	{
		switch (request->curState)
		{
		case ParseReqLine:
			parseHttpRequestLine(request, readBuf);
			break;
		case ParseReqHeaders:
			parseHttpRequestHeader(request, readBuf);
			break;
		case ParseReqBody:
			break;
		default:
			break;
		}
		if (!flag)
		{
			return flag;
		}
		// �ж��Ƿ������ϣ����������ϣ���Ҫ׼���ظ�������
		if (request->curState == ParseReqDone)
		{
			// 1. ���ݽ�������ԭʼ���ݣ��Կͻ��˵�������������
			processHttpRequest(request, response);
			// 2.��֯��Ӧ����
			httpResponsePrepareMsg(response, sendBuf, socket);
		}
	}
	request->curState = ParseReqLine;			// ״̬��ԭ����֤���ܼ��������ڶ�����֮������
	return flag;
}

// ����http���� -- ��������get��http����
bool processHttpRequest(struct HttpRequest* request, struct HttpResponse* response)
{
	// ����������   get /xxx/1.jpg http/1.1
	if (strcasecmp(request->method, "get") != 0)
	{
		return -1;
	}

	// ��ʮ��λתΪʮλ��תΪ���� ·��ת��  --- ʶ������
	decodeMsg(request->url, request->url);

	// �����ͻ�������ľ�̬��Դ��Ŀ¼���ļ���
	char* file = NULL;
	if (strcmp(request->url, "/") == 0)
	{
		file = "./";
	}
	else
	{
		file = request->url + 1;
	}

	// ��ȡ�ļ�
	struct stat st;
	int ret = stat(file, &st);
	if (ret == -1)
	{
		// �ļ������� --- �ظ�404
		// sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
		// sendFile("404.html", cfd);
		strcpy(response->fileName, "404.html");
		response->statusCode = NotFound;
		strcpy(response->statusMsg, "Not Found");
		// ��Ӧͷ
		httpResponseAddHeader(response, "Content-type", getFileType(".html"));
		response->sendDataFunc = sendFile;
		return 0;
	}

	strcpy(response->fileName, file);
	response->statusCode = OK;
	strcpy(response->statusMsg, "OK");
	if (S_ISDIR(st.st_mode))
	{	// �ж�file���ļ�����Ŀ¼
		// �����Ŀ¼�����Ŀ¼���ݷ��͸��ͻ���
		// sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
		// sendDir(file, cfd);
		// ��Ӧͷ
		httpResponseAddHeader(response, "Content-type", getFileType(".html"));
		response->sendDataFunc = sendDir;
	}
	else
	{
		// ���ļ������ݷ��͸��ͻ���
		// sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
		// sendFile(file, cfd);
		// ��Ӧͷ
		char tmp[12] = { 0 };
		sprintf(tmp, "%ld", st.st_size);
		httpResponseAddHeader(response, "Content-type", getFileType(file));
		httpResponseAddHeader(response, "Content-length", tmp);

		response->sendDataFunc = sendFile;

	}
	return false;
}


// ���ַ�ת��Ϊ������
int hexToDec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}

// ����
// to �洢����֮�������, ��������, from�����������, �������
void decodeMsg(char* to, char* from)
{
	for (; *from != '\0'; ++to, ++from)
	{
		// isxdigit -> �ж��ַ��ǲ���16���Ƹ�ʽ, ȡֵ�� 0-f
		// Linux%E5%86%85%E6%A0%B8.jpg
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			// ��16���Ƶ��� -> ʮ���� �������ֵ��ֵ�����ַ� int -> char
			// B2 == 178
			// ��3���ַ�, �����һ���ַ�, ����ַ�����ԭʼ����
			*to = hexToDec(from[1]) * 16 + hexToDec(from[2]);

			// ���� from[1] �� from[2] ����ڵ�ǰѭ�����Ѿ���������
			from += 2;
		}
		else
		{
			// �ַ�����, ��ֵ
			*to = *from;
		}

	}
	*to = '\0';
}



// �����ļ��ĺ�׺�������֣���ȡ�ļ�������const char* type
const char* getFileType(const char* name)
{
	// a.jpg a.mp4 a.html
	// ����������ҡ�.���ַ�, �粻���ڷ���NULL
	const char* dot = strrchr(name, '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8";	// ���ı�
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

// �����ļ�
void sendFile(const char* fileName, struct Buffer* sendBuf, int cfd)
{
	// 1.���ļ�
	int fd = open(fileName, O_RDONLY);
	assert(fd > 0);						// ����0�������������������ֹ
	// �����ļ�
	while (1)
	{
		char buf[1024];
		int len = read(fd, buf, sizeof(buf));
		if (len > 0)
		{
			// send(cfd, buf, len, 0);
			bufferAppendData(sendBuf, buf, len);
			bufferSendData(sendBuf, cfd);
		}
		else if (len == 0)
		{
			break;
		}
		else
		{
			close(fd);
			perror("read");
		}
	}
	close(fd);
}




/*
<html>
	<head>
		<title>test</title>
	</head>
	<body>
		<table>
			<tr>
				<td></td>
				<td></td>
			</tr>
			<tr>
				<td></td>
				<td></td>
			</tr>
		</table>
	</body>
</html>
*/
// ����Ŀ¼
void sendDir(const char* dirName, struct Buffer* sendBuf, int cfd)
{
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);
	struct dirent** namelist;
	int num = scandir(dirName, &namelist, NULL, alphasort);		// ���صõ��˶��ٸ��ļ�
	for (int i = 0; i < num; ++i)
	{
		// ȡ���ļ��� namelistָ��һ��ָ������ struct dirent* tmp[]
		char* name = namelist[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName, name);				// ע�⣺/��ʾ������ϵ
		// stat()����������file_name ��ָ���ļ�״̬, ���Ƶ�����buf ��ָ�Ľṹ�С�
		stat(name, &st);
		if (S_ISDIR(st.st_mode))
		{
			// a��ǩ �� <a href = "">name</a> ͨ��a��ǩʵ����ת����Ӧλ��  -- ��Ϊ�Ѿ���""���ټ�˫������Ҫת����ţ�\��
			// ��%s�������/����ʾ����ĳ���ļ�������/��ʾ�����ļ���ȥ
			sprintf(buf + strlen(buf), "<tr><td><a href = \"%s/\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		else
		{
			sprintf(buf + strlen(buf), "<tr><td><a href = \"%s\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		// send(cfd, buf, strlen(buf), 0);
		bufferAppendString(sendBuf, buf);
		bufferSendData(sendBuf, cfd);
		memset(buf, 0, sizeof(buf));
		// �ͷ���Դ
		free(namelist[i]);
	}
	sprintf(buf, "</table></body></html>");
	// send(cfd, buf, strlen(buf), 0);
	bufferAppendString(sendBuf, buf);
	bufferSendData(sendBuf, cfd);
	free(namelist);
}







