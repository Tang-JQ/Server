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


HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest()
{
}

//  ����
void HttpRequest::reset()
{
	m_curState = ProcessState::ParseReqLine;
	m_method = m_url = m_version = string();
	m_reqHeaders.clear();
}


// �������ͷ
void HttpRequest::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty())
	{
		return;
	}
	m_reqHeaders.insert(make_pair(key, value));
}

// ����key�õ�����ͷ��value
string HttpRequest::getHeader(const string key)
{
	auto item = m_reqHeaders.find(key);
	if (item == m_reqHeaders.end())
	{
		return string();
	}
	return item->second;
}

// ����������
bool HttpRequest::parseRequestLine(Buffer* readBuf)
{
	// ����������, �����ַ���������ַ(\r\n��ַ��
	char* end = readBuf->findCRLF();

	// �����ַ�����ʼ��ַ
	char* start = readBuf->data();

	// �������ܳ���
	int lineSize = end - start;

	if (lineSize > 0)
	{
		auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", methodFunc);
		auto urlFunc = bind(&HttpRequest::seturl, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", urlFunc);
		auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
		splitRequestLine(start, end, nullptr, versionFunc);
		// Ϊ��������ͷ��׼��
		readBuf->readPosIncrease(lineSize + 2);
		// �޸�״̬
		setState(ProcessState::ParseReqHeaders);
		return true;
	}
	return false;
}

char* HttpRequest::splitRequestLine(const char* start, const char* end,
	const char* sub, function<void(string)>callback)
{
	char* space = const_cast<char*>(end);
	if (sub != nullptr)
	{
		space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
		assert(space != nullptr);
	}
	int length = space - start;
	callback(string(start, length));
	return space + 1;
}

// ��������ͷ
bool HttpRequest::parseRequestHeader(Buffer* readBuf)
{
	char* end = readBuf->findCRLF();
	if (end != nullptr)
	{
		char* start = readBuf->data();
		int lineSize = end - start;
		// ����: �����ַ���
		char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
		if (middle != nullptr)
		{
			int keyLen = middle - start;
			int valueLen = end - middle - 2;
			if (keyLen > 0 && valueLen > 0)
			{
				string key(start, keyLen);
				string value(middle + 2, valueLen);
				addHeader(key, value);
			}
			// �ƶ������ݵ�λ��
			readBuf->readPosIncrease(lineSize + 2);
		}
		else
		{
			// ����ͷ����������, ��������
			readBuf->readPosIncrease(2);
			// �޸Ľ���״̬
			// ���� post ����, ���� get ������
			setState(ProcessState::ParseReqDone);
		}
		return true;
	}
	return false;
}

// ����http����Э��
bool HttpRequest::parseRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket)
{
	bool flag = true;
	while (m_curState != ProcessState::ParseReqDone)
	{
		switch (m_curState)
		{
		case ProcessState::ParseReqLine:
			parseRequestLine(readBuf);
			break;
		case ProcessState::ParseReqHeaders:
			parseRequestHeader(readBuf);
			break;
		case ProcessState::ParseReqBody:
			break;
		default:
			break;
		}
		if (!flag)
		{
			return flag;
		}
		// �ж��Ƿ������ϣ����������ϣ���Ҫ׼���ظ�������
		if (m_curState == ProcessState::ParseReqDone)
		{
			// 1. ���ݽ�������ԭʼ���ݣ��Կͻ��˵�������������
			processHttpRequest(response);
			// 2.��֯��Ӧ����
			response->prepareMsg(sendBuf, socket);
		}
	}
	m_curState = ProcessState::ParseReqLine;			// ״̬��ԭ����֤���ܼ�������ڶ�����֮������
	return flag;
}

// ����http����
bool HttpRequest::processHttpRequest(HttpResponse* response)
{
	// ����������   get /xxx/1.jpg http/1.1
	if (strcasecmp(m_method.data(), "get") != 0)
	{
		return -1;
	}

	// ��ʮ��λתΪʮλ��תΪ���� ·��ת��  --- ʶ������
	m_url = decodeMsg(m_url);

	// ����ͻ�������ľ�̬��Դ��Ŀ¼���ļ���
	const char* file = NULL;
	if (strcmp(m_url.data(), "/") == 0)
	{
		file = "./";
	}
	else
	{
		file = m_url.data() + 1;
	}

	// ��ȡ�ļ�
	struct stat st;
	int ret = stat(file, &st);
	if (ret == -1)
	{
		// �ļ������� --- �ظ�404
		// sendHeadMsg(cfd, 404, "Not Found", getFileType(".html"), -1);
		// sendFile("404.html", cfd);
		response->setFileName("404.html");
		response->setStatusCode(statusCode::NotFound);
		// ��Ӧͷ
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendFile;
		return 0;
	}

	response->setFileName(file);
	response->setStatusCode(statusCode::OK);
	if (S_ISDIR(st.st_mode))
	{	// �ж�file���ļ�����Ŀ¼
		// �����Ŀ¼�����Ŀ¼���ݷ��͸��ͻ���
		// sendHeadMsg(cfd, 200, "OK", getFileType(".html"), -1);
		// sendDir(file, cfd);
		// ��Ӧͷ
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendDir;
	}
	else
	{
		// ���ļ������ݷ��͸��ͻ���
		// sendHeadMsg(cfd, 200, "OK", getFileType(file), st.st_size);
		// sendFile(file, cfd);
		// ��Ӧͷ
		response->addHeader("Content-type", getFileType(file));
		response->addHeader("Content-length", to_string(st.st_size));

		response->sendDataFunc = sendFile;

	}
	return false;
}

// �����ַ���
string HttpRequest::decodeMsg(string msg)
{
	string str = string();
	const char* from = msg.data();
	for (; *from != '\0'; ++from)
	{
		// isxdigit -> �ж��ַ��ǲ���16���Ƹ�ʽ, ȡֵ�� 0-f
		// Linux%E5%86%85%E6%A0%B8.jpg
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			// ��16���Ƶ��� -> ʮ���� �������ֵ��ֵ�����ַ� int -> char
			// B2 == 178
			// ��3���ַ�, �����һ���ַ�, ����ַ�����ԭʼ����
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

			// ���� from[1] �� from[2] ����ڵ�ǰѭ�����Ѿ��������
			from += 2;
		}
		else
		{
			// �ַ�����, ��ֵ
			str.append(1, *from);
		}

	}
	str.append(1, '\0');
	return str;
}

// �����ļ��ĺ�׺�������֣���ȡ�ļ�������const char* type
const string HttpRequest::getFileType(string name)
{
	// a.jpg a.mp4 a.html
// ����������ҡ�.���ַ�, �粻���ڷ���NULL
	const char* dot = strrchr(name.data(), '.');
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

void HttpRequest::sendFile(string fileName, Buffer* sendBuf, int cfd)
{
	// 1.���ļ�
	int fd = open(fileName.data(), O_RDONLY);
	assert(fd > 0);						// ����0�������������������ֹ
	// �����ļ�
	while (1)
	{
		char buf[1024];
		int len = read(fd, buf, sizeof(buf));
		if (len > 0)
		{
			// send(cfd, buf, len, 0);
			sendBuf->appendString(buf, len);
			sendBuf->sendData(cfd);
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

void HttpRequest::sendDir(string dirName, Buffer* sendBuf, int cfd)
{
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.data());
	struct dirent** namelist;
	int num = scandir(dirName.data(), &namelist, NULL, alphasort);		// ���صõ��˶��ٸ��ļ�
	for (int i = 0; i < num; ++i)
	{
		// ȡ���ļ��� namelistָ��һ��ָ������ struct dirent* tmp[]
		char* name = namelist[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName.data(), name);				// ע�⣺/��ʾ������ϵ
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
		sendBuf->appendString(buf);
		sendBuf->sendData(cfd);
		memset(buf, 0, sizeof(buf));
		// �ͷ���Դ
		free(namelist[i]);
	}
	sprintf(buf, "</table></body></html>");
	// send(cfd, buf, strlen(buf), 0);
	sendBuf->appendString(buf);
	sendBuf->sendData(cfd);
	free(namelist);
}


// ���ַ�ת��Ϊ������
int HttpRequest::hexToDec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}

