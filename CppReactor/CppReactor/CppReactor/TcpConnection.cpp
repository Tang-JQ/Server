#include "TcpConnection.h"
#include "HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"


TcpConnection::TcpConnection(int fd, EventLoop* evLoop)
{
	m_evLoop = evLoop;
	m_readBuf = new Buffer(10240);
	m_writeBuf = new Buffer(10240);
	// http
	m_request = new HttpRequest;
	m_response = new HttpResponse;
	m_name = "Connection-%d" + to_string(fd);
	m_channel = new Channel(fd, FDEvent::ReadEvent, processRead, processWrite, destroy, this);
	evLoop->addTask(m_channel, ElemType::ADD);

	// Debug("�Ϳͻ��˽������ӣ�threadName��%s��threadID��%s��connName��%s",evLoop->threadName, evLoop->threadID, conn->name);
}


TcpConnection::~TcpConnection()
{
	if (m_readBuf && m_readBuf->readableSize() == 0 &&
		m_writeBuf && m_writeBuf->readableSize() == 0)
	{ 
		delete m_readBuf;
		delete m_writeBuf;
		delete m_request;
		delete m_response;
		m_evLoop->freeChannel(m_channel);
	}
}

int TcpConnection::processRead(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	// ��������
	int socket = conn->m_channel->getSocket();
	int count = conn->m_readBuf->socketRead(socket);

	Debug("���յ���http�������ݣ�%s", conn->m_readBuf->data());
	if (count > 0)
	{
		// ���յ���http���󣬽���http����
		// writeEventEnable(conn->channel, true);				// �޸�--�ļ���������ɶ�д�¼�
		// eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
		bool flag = conn->m_request->parseRequest(conn->m_readBuf, conn->m_response,
			conn->m_writeBuf, socket);
		if (!flag)
		{
			// ����ʧ�ܣ��ظ�һ���򵥵�html
			string errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
			conn->m_writeBuf->appendString(errMsg);
		}
	}

	// �Ͽ�����
	conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);

	return 0;
}

int TcpConnection::processWrite(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	// ��������
	int count = conn->m_writeBuf->sendData(conn->m_channel->getSocket());
	if (count > 0)
	{
		// �ж������Ƿ�ȫ�����ͳ�ȥ��
		if (conn->m_writeBuf->readableSize() == 0)
		{
			// 1. ���ټ��д�¼� -- �޸�channel�б�����¼�
			conn->m_channel->writeEventEnable(false);
			// 2. �޸�dispatcher���ļ��� -- �������ڵ�
			conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY);
			// 3. ɾ������ڵ�
			conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
		}
	}
	return 0;
}


int TcpConnection::destroy(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	if (conn != nullptr)
	{
		delete conn;
	}
	return 0;
}












