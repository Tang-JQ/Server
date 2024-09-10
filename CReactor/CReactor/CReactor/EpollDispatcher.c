#include "Dispatcher.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>


#define Max 520
struct EpollData
{	// ���ڵ� �� epoll�¼�(��д��
	int epfd;
	struct epoll_event* events;
};

static void* epollInit();
static int epollAdd(struct Channel* channel, struct EventLoop* evloop);
static int epollRemove(struct Channel* channel, struct EventLoop* evloop);
static int epollModify(struct Channel* channel, struct EventLoop* evloop);
static int epollDispatch(struct EventLoop* evLoop, int timeout);
static int epollClear(struct EventLoop* evloop);

struct Dispatcher EpollDispatcher = {
	epollInit,
	epollAdd,
	epollRemove,
	epollModify,
	epollDispatch,
	epollClear
};

static void* epollInit()
{
	struct EpollData* data = (struct EpollData*)malloc(sizeof(struct EpollData));
	data->epfd = epoll_create(1);
	if (data->epfd == -1)
	{
		perror("epoll_create");
		exit(0);
	}
	// calloc��malloc���
	data->events = (struct epoll_event*)calloc(Max, sizeof(struct epoll_event));
	return data;
}

static int epollAdd(struct Channel* channel, struct EventLoop* evloop)
{
	struct EpollData* data = (struct EpollData*)evloop->dispatcherData;
	struct epoll_event ev;
	ev.data.fd = channel->fd;
	int events = 0;
	// channel->events���Լ�����ģ���ReadEvent = 0x02����epoll�����EPOLLIN�ȣ�����ϵͳ����ģ�
	if (channel->events & ReadEvent)
	{	// ͨ����־λ����¼�¼�
		events |= EPOLLIN;
	}
	if (channel->events & WriteEvent)
	{
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(data->epfd, EPOLL_CTL_ADD, channel->fd, &ev);

	return ret;
}

static int epollRemove(struct Channel* channel, struct EventLoop* evloop)
{
	struct EpollData* data = (struct EpollData*)evloop->dispatcherData;
	struct epoll_event ev;
	ev.data.fd = channel->fd;
	int events = 0;
	if (channel->events & ReadEvent)
	{
		events |= EPOLLIN;
	}
	if (channel->events & WriteEvent)
	{
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(data->epfd, EPOLL_CTL_DEL, channel->fd, &ev);

	// ͨ��Chanel�ͷŶ�Ӧ��TcpConnection��Դ
	channel->destroyCallback(channel->arg);

	return ret;
}

static int epollModify(struct Channel* channel, struct EventLoop* evloop)
{
	struct EpollData* data = (struct EpollData*)evloop->dispatcherData;
	struct epoll_event ev;
	ev.data.fd = channel->fd;
	int events = 0;
	if (channel->events & ReadEvent)
	{
		events |= EPOLLIN;
	}
	if (channel->events & WriteEvent)
	{
		events |= EPOLLOUT;
	}
	ev.events = events;
	int ret = epoll_ctl(data->epfd, EPOLL_CTL_MOD, channel->fd, &ev);

	return ret;
}

// �¼����
static int epollDispatch(struct EventLoop* evLoop, int timeout)
{
	struct EpollData* data = (struct EpollData*)evLoop->dispatcherData;
	// data->eventsֻ���ʼ����С���ɣ���һ���������� 
	int count = epoll_wait(data->epfd, data->events, Max, timeout * 1000);
	for (int i = 0; i < count; ++i)
	{
		int events = data->events[i].events;
		int fd = data->events[i].data.fd;
		// �Զ˶Ͽ����� �� �Զ˶Ͽ����ӻ����Է���������
		if (events & EPOLLERR || events & EPOLLHUP)
		{
			// epollRemove(Channel, evLoop);
			continue;
		}
		if (events & EPOLLIN)
		{
			eventActivate(evLoop, fd, ReadEvent);
		}
		if (events & EPOLLOUT)
		{
			eventActivate(evLoop, fd, WriteEvent);
		}
	}
	return 0;
}

static int epollClear(struct EventLoop* evloop)
{
	struct EpollData* data = (struct EpollData*)evloop->dispatcherData;
	free(data->events);
	close(data->epfd);
	free(data);
	return 0;
}