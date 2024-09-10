#include "TcpServer.h"
#include <arpa/inet.h>
#include "TcpConnection.h"
#include <stdio.h>
#include <stdlib.h>
#include "Log.h"


TcpServer::TcpServer(unsigned short port, int threadNum)
{
	m_port = port;
	setListen();
	m_mainLoop = new EventLoop;
	m_threadNum = threadNum;
	m_threadPool = new ThreadPool(m_mainLoop, threadNum);
}

TcpServer::~TcpServer()
{

}

// ��ʼ������
void TcpServer::setListen()
{
	// 1.����������fd
	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd == -1)
	{
		perror("socket");
		return;
	}

	// 2.���ö˿ڸ���
	int opt = 1;
	int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1)
	{
		perror("setsockopt");
		return;
	}

	// 3.��
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(m_lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("bind");
		return;
	}

	// 4.���ü���
	ret = listen(m_lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return;
	}
}

// ����������
void TcpServer::run()
{
	Debug("�����������Ѿ�������...");
	// �����̳߳�
	m_threadPool->run();
	// ���Ӽ������� -
	// ��ʼ��һ��channelʵ��
	Channel* channel = new Channel(m_lfd, FDEvent::ReadEvent, acceptConnection, nullptr, nullptr, this);

	m_mainLoop->addTask(channel, ElemType::ADD);
	// ������Ӧ��ģ��
	m_mainLoop->run();
}



int TcpServer::acceptConnection(void* arg)
{
	struct TcpServer* server = static_cast<TcpServer*>(arg);
	// �Ϳͻ��˽�������
	int cfd = accept(server->m_lfd, NULL, NULL);
	// ���̳߳�ȡ��һ�����̵߳ķ�Ӧ��ʵ����ȥ�������fd
	EventLoop* evLoop = server->m_threadPool->takeWorkerEventLoop();
	// ��cfd�ŵ�TcpConnection��ȥ����
	new TcpConnection(cfd, evLoop);
	return 0;
}


















