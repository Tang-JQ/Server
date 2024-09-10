#include "TcpConnection.h"
#include "HttpRequest.h"
#include <stdlib.h>
#include <stdio.h>
#include "Log.h"

int processRead(void* arg)
{
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	// ��������
	int count = bufferSocketRead(conn->readBuf, conn->channel->fd);

	Debug("���յ���http�������ݣ�%s", conn->readBuf->data + conn->readBuf->readPos);
	if (count > 0)
	{
		// ���յ���http���󣬽���http����
		int socket = conn->channel->fd;
		// writeEventEnable(conn->channel, true);				// �޸�--�ļ���������ɶ�д�¼�
		// eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);

		bool flag = parseHttpRequest(conn->request, conn->readBuf, conn->response, conn->writeBuf, socket);
		if (!flag)
		{
			// ����ʧ�ܣ��ظ�һ���򵥵�html
			char* errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
			bufferAppendString(conn->writeBuf, errMsg);
		}
	}

	// �Ͽ�����
	eventLoopAddTask(conn->evLoop, conn->channel, DELETE);

	return 0;
}

int processWrite(void* arg)
{
	Debug("��ʼ���������ˣ�����д�¼�����...");
	struct TcpConnection* conn = (struct TcpConnection*)arg;
	// ��������
	int count = bufferSendData(conn->writeBuf, conn->channel->fd);
	if (count > 0)
	{
		// �ж������Ƿ�ȫ�����ͳ�ȥ��
		if (bufferReadableSize(conn->writeBuf) == 0)
		{
			// 1. ���ټ��д�¼� -- �޸�channel�б�����¼�
			writeEventEnable(conn->channel, false);
			// 2. �޸�dispatcher���ļ��� -- �������ڵ�
			eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
			// 3. ɾ������ڵ�
			eventLoopAddTask(conn->evLoop, conn->channel, DELETE);
		}
	}
	return 0;
}

// ��ʼ��
struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop)
{
	struct TcpConnection* conn = (struct TcpConnection*)malloc(sizeof(struct TcpConnection));
	conn->evLoop = evLoop;
	conn->readBuf = bufferInit(10240);
	conn->writeBuf = bufferInit(10240);
	// http
	conn->request = httpRequestInit();
	conn->response = httpResponseInit();
	sprintf(conn->name, "Connection-%d", fd);
	conn->channel = channelInit(fd, ReadEvent, processRead, processWrite, tcpConnectionDestroy, conn);
	eventLoopAddTask(evLoop, conn->channel, ADD);

	Debug("�Ϳͻ��˽������ӣ�threadName��%s��threadID��%s��connName��%s",
		evLoop->threadName, evLoop->threadID, conn->name);
	return conn;
}

// �ͷ���Դ
int tcpConnectionDestroy(void* arg)
{

	struct TcpConnection* conn = (struct TcpConnection*)arg;
	Debug("���ӶϿ����ͷ���Դ��gameover��connName��%s", conn->name);
	if (conn != NULL)
	{
		if (conn->readBuf && bufferReadableSize(conn->readBuf) == 0 &&
			conn->writeBuf && bufferReadableSize(conn->writeBuf) == 0)
		{ //readBuf��writeBufָ����Ч�ڴ沢��û�пɴ��������
			destroyChannel(conn->evLoop, conn->channel);
			bufferDestroy(conn->readBuf);
			bufferDestroy(conn->writeBuf);
			httpResponseDestroy(conn->response);
			httpRequestDestroy(conn->request);
			free(conn);
		}
	}
	Debug("���ӶϿ�, �ͷ���Դ, gameover, connName: %s", conn->name);
	return 0;
}














