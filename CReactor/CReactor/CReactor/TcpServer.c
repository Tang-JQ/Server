#include "TcpServer.h"
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdio.h>
#include <stdlib.h>
#include "Log.h"


// ��ʼ�� -- ���ض˿ں����̸߳���
struct TcpServer* tcpServerInit(unsigned short port, int threadNum)
{
	struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
	tcp->listener = listenerInit(port);
	tcp->mainLoop = eventLoopInit();
	tcp->threadNum = threadNum;
	tcp->threadPool = threadPoolInit(tcp->mainLoop, tcp->threadNum);
	return tcp;
}

int acceptConnection(void* arg)
{
	struct TcpServer* server = (struct TcpServer*)arg;
	// �Ϳͻ��˽�������
	int cfd = accept(server->listener->lfd, NULL, NULL);
	// ���̳߳�ȡ��һ�����̵߳ķ�Ӧ��ʵ����ȥ�������fd
	struct EventLoop* evLoop = takeWorkerEventLoop(server->threadPool);
	// ��cfd�ŵ�TcpConnection��ȥ����
	tcpConnectionInit(cfd, evLoop);
	return 0;
}
// ����������
void tcpServerRun(struct TcpServer* server)
{
	Debug("1�����������Ѿ�������...");
	// �����̳߳�
	threadPoolRun(server->threadPool);
	// ���Ӽ������� -
	// ��ʼ��һ��channelʵ��
	struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptConnection, NULL, NULL, server);

	eventLoopAddTask(server->mainLoop, channel, ADD);
	// ������Ӧ��ģ��
	eventLoopRun(server->mainLoop);
	Debug("�����������Ѿ�������...");
}

// ��ʼ������
struct Listener* listenerInit(unsigned short port)
{
	struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
	// 1.����������fd
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		return NULL;
	}

	// 2.���ö˿ڸ���
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1)
	{
		perror("setsockopt");
		return NULL;
	}

	// 3.��
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("bind");
		return NULL;
	}

	// 4.���ü���
	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return NULL;
	}

	// ����listener
	listener->lfd = lfd;
	listener->port = port;
	return listener;
}




















