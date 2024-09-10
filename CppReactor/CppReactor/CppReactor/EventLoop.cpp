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
	m_isQuit = true;					// Ĭ��û������
	m_threadID = this_thread::get_id(); 
	m_threadName = threadName == string() ? "MainThread" : threadName;
	m_dispatcher = new EpollDispatcher(this);
	// map 
	m_channelMap.clear();
	// ����һ�����ӵ��׽��֡�����׽��ֿ���������ͬһ�����е������̻߳�ͬ����֮�����˫��ͨ�š�
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair);
	if (ret == -1)
	{
		perror("socketpair");
		exit(0);
	}
#if 0
	// ָ������: evLoop->socketPair[0] ��������, evLoop->socketPair[1] ��������
	Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent, readLocalMessage, nullptr, nullptr, this);
#else
	// �� - bind��cpp�пɵ��ö������
	auto obj = bind(&EventLoop::readMessage, this);
	Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent, obj, nullptr, nullptr, this);
#endif
		
	// channel ��ӵ��������
	addTask(channel, ElemType::ADD);
}

EventLoop::~EventLoop()
{
	// ����EventLoop����Ŀִ�й�����һֱ���ڣ�������Ŀ����������������ϵͳ��ִ����һ��������˲��赥������
}

// ������Ӧ��ģ��
int EventLoop::run()
{
	// �Ƚ��߳�ID�Ƿ�����   this_thread::get_id() == pthread_self
	if (m_threadID != this_thread::get_id())
	{
		return -1;
	}
	// ѭ�������¼�����
	m_isQuit = false;
	while (!m_isQuit)
	{
		m_dispatcher->dispatch();    // ��ʱʱ�� 2s
		processTaskQ();
	}
	return 0;
}

// ��������ļ�fd
int EventLoop::eventActive(int fd, int event)
{
	if (fd < 0)
	{
		return -1;
	}
	// ȡ��channel
	Channel* channel =m_channelMap[fd];
	assert(channel->getSocket() == fd);
	if (event & (int)FDEvent::ReadEvent && channel->readCallback)
	{
		channel->readCallback(const_cast<void*>(channel->getArg()));		// channel->getArg()������constֻ�����ͣ�ʹ��const_cast���Ըı�����ȥ��const
	}
	if (event & (int)FDEvent::WriteEvent && channel->writeCallback)
	{
		channel->writeCallback(const_cast<void*>(channel->getArg()));
	}
	return 0;
}

// ��������������
int EventLoop::addTask(Channel* channel, ElemType type)
{
	// ����, ����������Դ
	m_mutex.lock();
	// �����½ڵ�
	ChannelElement* node = new struct ChannelElement;
	node->channel = channel;
	node->type = type;

	m_taskQ.push(node);

	m_mutex.unlock();


	if (m_threadID == this_thread::get_id())
	{
		// ��ǰ���߳�(�������̵߳ĽǶȷ���)
		processTaskQ();
	}
	else
	{
		// ���߳� -- �������̴߳�����������е�����
		// 1. ���߳��ڹ��� 2. ���̱߳�������:select, poll, epoll
		takeWakeup();
	}
	return 0;
}

// ������������е�����
int EventLoop::processTaskQ()
{
	// ȡ��ͷ���
	while (!m_taskQ.empty())
	{
		m_mutex.lock();
		struct ChannelElement* node = m_taskQ.front();
		m_taskQ.pop();
		m_mutex.unlock();

		Channel* channel = node->channel; 
		if (node->type == ElemType::ADD)
		{
			// ���
			add(channel);
		}
		else if (node->type == ElemType::DELETE)
		{
			// ɾ��
			remove(channel);
		}
		else if (node->type == ElemType::MODIFY)
		{
			// �޸�
			modify(channel);
		}
		delete node;
	}
	return 0;
}

// ����dispatcher�еĽڵ�
int EventLoop::add(Channel* channel)
{
	int fd = channel->getSocket();

	// �ҵ�fd'��Ӧ������Ԫ��λ�ã����洢
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

// �ͷ�channel 
int EventLoop::freeChannel(Channel* channel)
{
	// Ѱ�ҵ����� auto��c++11�����ԣ��ܰ����Զ��Ƶ�
	auto it = m_channelMap.find(channel->getSocket());

	if (it != m_channelMap.end())
	{
		m_channelMap.erase(it);
		close(channel->getSocket());
		delete channel;
	}

	return 0;
}





// д����
void EventLoop::takeWakeup()
{
	const char* msg = "��Ҫ���Ϻ�����������";
	write(m_socketPair[0], msg, strlen(msg));
}

// ������  ---- Ŀ�Ĳ��Ƕ����ݣ����Ǵ������¼��� ���ܶ�epoll_wait�������
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






