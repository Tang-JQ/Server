#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <map>
using namespace std;



// ��ǰ�Ľ���״̬
enum class ProcessState:char
{
	ParseReqLine,			// ������������
	ParseReqHeaders,		// ����������ͷ
	ParseReqBody,			// ��������������
	ParseReqDone			// ����Э���ѽ���
};

// ����http����ṹ�壨�Ĳ��֣������С�����ͷ�����С���������
class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	//  ����
	void reset();

	// ��ȡ����״̬
	inline ProcessState getState()
	{
		return m_curState;
	}

	// �������ͷ
	void addHeader(const string key, const string value);

	// ����key�õ�����ͷ��value
	string getHeader(const string key);

	// ����������
	bool parseRequestLine(Buffer* readBuf);

	// ��������ͷ
	bool parseRequestHeader(Buffer* readBuf);

	// ����http����Э��
	bool parseRequest(Buffer* readBuf,
		HttpResponse* response, Buffer* sendBuf, int socket);

	// ����http����
	bool processHttpRequest(HttpResponse* response);

	// �����ַ���
	string decodeMsg(string from);

	// �����ļ��ĺ�׺�������֣���ȡ�ļ�������const char* type
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
	// ������
	string m_method;			// get��post
	string m_url;				// ���������ļ���ַ
	string m_version;			// �汾
	// ����ͷ ��ֵ�� �ж��������һ������,����С
	map<string, string> m_reqHeaders;
	// http����״̬
	ProcessState m_curState;
};

