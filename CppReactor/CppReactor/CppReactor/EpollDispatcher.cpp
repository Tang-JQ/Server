#include "EpollDispatcher.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define Max 520



int EpollDispatcher::epollCtl(int op)
{
	struct epoll_event ev;
	ev.data.fd = m_channel->getSocket();
	int events = 0;
	if (m_channel->getEvent() & static_cast<int>(FDEvent::ReadEvent))
	{
		events |= EPOLLIN;
	}
	if (m_channel->getEvent() & static_cast<int>(FDEvent::WriteEvent))
	{
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(m_epfd, op, m_channel->getSocket(), &ev);
	return ret;
}

EpollDispatcher::EpollDispatcher(EventLoop* evloop) : Dispatcher(evloop)
{
	m_epfd = epoll_create(1);
	if (m_epfd == -1)
	{
		perror("epoll_create");
		exit(0);
	}
	// calloc��malloc��� 
	m_events = new struct epoll_event[m_maxNode];
}

EpollDispatcher::~EpollDispatcher()
{
	close(m_epfd);
	delete[] m_events;
}

// ���
int EpollDispatcher::add()
{
	int ret = epollCtl(EPOLL_CTL_ADD);
	if (ret == -1)
	{
		perror("epoll_crl add");
		exit(0);
	}
	return ret;
}

// ɾ��
int EpollDispatcher::remove()
{
	int ret = epollCtl(EPOLL_CTL_DEL);
	if (ret == -1)
	{
		perror("epoll_crl delete");
		exit(0);
	}
	// const_cast����ȥ��const��ֻ������
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArg()));
	return ret;
}

// �޸�
int EpollDispatcher::modify()
{
	int ret = epollCtl(EPOLL_CTL_MOD);
	if (ret == -1)
	{
		perror("epoll_crl modify");
		exit(0);
	}
	return ret;
}

// �¼����
int EpollDispatcher::dispatch(int timeout)
{
	// data->eventsֻ���ʼ����С���ɣ���һ���������� 
	int count = epoll_wait(m_epfd, m_events, m_maxNode, timeout * 1000);
	for (int i = 0; i < count; ++i)
	{
		int events = m_events[i].events;
		int fd = m_events[i].data.fd;
		// �Զ˶Ͽ����� �� �Զ˶Ͽ����ӻ����Է���������
		if (events & EPOLLERR || events & EPOLLHUP)
		{
			// epollRemove(Channel, evLoop);
			continue;
		}
		if (events & EPOLLIN)
		{
			m_evLoop->eventActive(fd, (int)FDEvent::ReadEvent);
		}
		if (events & EPOLLOUT)
		{
			m_evLoop->eventActive(fd, (int)FDEvent::WriteEvent);
		}
	}
	return 0;
}







