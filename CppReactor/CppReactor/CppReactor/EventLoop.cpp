#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "EventLoop.h"
#include "EpollDispatcher.h"

EventLoop::EventLoop() : EventLoop(string())
{

}

EventLoop::EventLoop(const string threadName)
{
	m_isQuit = true;					// 默认没有启动
	m_threadID = this_thread::get_id(); 
	m_threadName = threadName == string() ? "MainThread" : threadName;
	m_dispatcher = new EpollDispatcher(this);
	// map 
	m_channelMap.clear();
	// 创建一对连接的套接字。这对套接字可以用来在同一进程中的两个线程或不同进程之间进行双向通信。
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair);
	if (ret == -1)
	{
		perror("socketpair");
		exit(0);
	}
#if 0
	// 指定规则: evLoop->socketPair[0] 发送数据, evLoop->socketPair[1] 接收数据
	Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent, readLocalMessage, nullptr, nullptr, this);
#else
	// 绑定 - bind（cpp中可调用对象绑定器
	auto obj = bind(&EventLoop::readMessage, this);
	Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent, obj, nullptr, nullptr, this);
#endif
		
	// channel 添加到任务队列
	addTask(channel, ElemType::ADD);
}

EventLoop::~EventLoop()
{
	// 由于EventLoop在项目执行过程中一直存在，仅在项目结束后析构。操作系统会执行这一操作。因此不需单独处理
}

// 启动反应堆模型
int EventLoop::run()
{
	// 比较线程ID是否正常   this_thread::get_id() == pthread_self
	if (m_threadID != this_thread::get_id())
	{
		return -1;
	}
	// 循环进行事件处理
	m_isQuit = false;
	while (!m_isQuit)
	{
		m_dispatcher->dispatch();    // 超时时长 2s
		processTaskQ();
	}
	return 0;
}

// 处理激活的文件fd
int EventLoop::eventActive(int fd, int event)
{
	if (fd < 0)
	{
		return -1;
	}
	// 取出channel
	Channel* channel =m_channelMap[fd];
	assert(channel->getSocket() == fd);
	if (event & (int)FDEvent::ReadEvent && channel->readCallback)
	{
		channel->readCallback(const_cast<void*>(channel->getArg()));		// channel->getArg()属性是const只读类型，使用const_cast可以改变属性去掉const
	}
	if (event & (int)FDEvent::WriteEvent && channel->writeCallback)
	{
		channel->writeCallback(const_cast<void*>(channel->getArg()));
	}
	return 0;
}

// 添加任务到任务队列
int EventLoop::addTask(Channel* channel, ElemType type)
{
	// 加锁, 保护共享资源
	m_mutex.lock();
	// 创建新节点
	ChannelElement* node = new struct ChannelElement;
	node->channel = channel;
	node->type = type;

	m_taskQ.push(node);

	m_mutex.unlock();


	if (m_threadID == this_thread::get_id())
	{
		// 当前子线程(基于子线程的角度分析)
		processTaskQ();
	}
	else
	{
		// 主线程 -- 告诉子线程处理任务队列中的任务
		// 1. 子线程在工作 2. 子线程被阻塞了:select, poll, epoll
		takeWakeup();
	}
	return 0;
}

// 处理任务队列中的任务
int EventLoop::processTaskQ()
{
	// 取出头结点
	while (!m_taskQ.empty())
	{
		m_mutex.lock();
		struct ChannelElement* node = m_taskQ.front();
		m_taskQ.pop();
		m_mutex.unlock();

		Channel* channel = node->channel; 
		if (node->type == ElemType::ADD)
		{
			// 添加
			add(channel);
		}
		else if (node->type == ElemType::DELETE)
		{
			// 删除
			remove(channel);
		}
		else if (node->type == ElemType::MODIFY)
		{
			// 修改
			modify(channel);
		}
		delete node;
	}
	return 0;
}

// 处理dispatcher中的节点
int EventLoop::add(Channel* channel)
{
	int fd = channel->getSocket();

	// 找到fd'对应的数组元素位置，并存储
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		m_channelMap.insert(make_pair(fd, channel));
		m_dispatcher->setChannel(channel);
		int ret = m_dispatcher->add();
		return ret;
	}
	return -1;
}

int EventLoop::remove(Channel* channel)
{
	int fd = channel->getSocket();

	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->remove();
	return ret;
}

int EventLoop::modify(Channel* channel)
{
	int fd = channel->getSocket();

	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->modify();
	return ret;
}

// 释放channel 
int EventLoop::freeChannel(Channel* channel)
{
	// 寻找迭代器 auto是c++11新特性，能帮助自动推导
	auto it = m_channelMap.find(channel->getSocket());

	if (it != m_channelMap.end())
	{
		m_channelMap.erase(it);
		close(channel->getSocket());
		delete channel;
	}

	return 0;
}





// 写数据
void EventLoop::takeWakeup()
{
	const char* msg = "我要当上海贼王！！！";
	write(m_socketPair[0], msg, strlen(msg));
}

// 读数据  ---- 目的不是读数据，而是触发读事件， 就能对epoll_wait解除阻塞
int EventLoop::readLocalMessage(void* arg)
{
	EventLoop* evLoop = static_cast<EventLoop*>(arg);
	char buf[256];
	read(evLoop->m_socketPair[1], buf, sizeof(buf));
	return 0;
}

int EventLoop::readMessage()
{
	char buf[256];
	read(m_socketPair[1], buf, sizeof(buf));
	return 0;
}






