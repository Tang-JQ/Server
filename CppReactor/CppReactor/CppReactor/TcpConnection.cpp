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

	// Debug("和客户端建立连接，threadName：%s，threadID：%s，connName：%s",evLoop->threadName, evLoop->threadID, conn->name);
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
	// 接收数据
	int socket = conn->m_channel->getSocket();
	int count = conn->m_readBuf->socketRead(socket);

	Debug("接收到的http请求数据：%s", conn->m_readBuf->data());
	if (count > 0)
	{
		// 接收到了http请求，解析http请求
		// writeEventEnable(conn->channel, true);				// 修改--文件描述符变成读写事件
		// eventLoopAddTask(conn->evLoop, conn->channel, MODIFY);
		bool flag = conn->m_request->parseRequest(conn->m_readBuf, conn->m_response,
			conn->m_writeBuf, socket);
		if (!flag)
		{
			// 解析失败，回复一个简单的html
			string errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
			conn->m_writeBuf->appendString(errMsg);
		}
	}

	// 断开连接
	conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);

	return 0;
}

int TcpConnection::processWrite(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	// 发送数据
	int count = conn->m_writeBuf->sendData(conn->m_channel->getSocket());
	if (count > 0)
	{
		// 判断数据是否全部发送出去了
		if (conn->m_writeBuf->readableSize() == 0)
		{
			// 1. 不再检测写事件 -- 修改channel中保存的事件
			conn->m_channel->writeEventEnable(false);
			// 2. 修改dispatcher检测的集合 -- 添加任务节点
			conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY);
			// 3. 删除这个节点
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












