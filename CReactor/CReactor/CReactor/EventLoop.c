#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "EventLoop.h"

// ��ʼ��
struct EventLoop* eventLoopInit()
{
	return eventLoopInitEx(NULL);
}

// д����
void takeWakeup(struct EventLoop* evLoop)
{
	const char* msg = "��Ҫ���Ϻ�����������";
	write(evLoop->socketPair[0], msg, strlen(msg));
}

// ������  ---- Ŀ�Ĳ��Ƕ����ݣ����Ǵ������¼��� ���ܶ�epoll_wait�������
int readLocalMessage(void* arg)
{
	struct EventLoop* evLoop = (struct EventLoop*)arg;
	char buf[256];
	read(evLoop->socketPair[1], buf, sizeof(buf));
	return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName)
{
	struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
	evLoop->isQuit = false;
	evLoop->threadID = pthread_self();
	pthread_mutex_init(&evLoop->mutex, NULL);
	strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
	evLoop->dispatcher = &EpollDispatcher;
	evLoop->dispatcherData = evLoop->dispatcher->init();
	// ����
	evLoop->head = evLoop->tail = NULL;
	// map
	evLoop->channelMap = channelMapInit(128);
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
	if (ret == -1)
	{
		perror("socketpair");
		exit(0);
	}
	// ָ������: evLoop->socketPair[0] ��������, evLoop->socketPair[1] ��������
	struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent,
		readLocalMessage, NULL, NULL, evLoop);
	// channel ���ӵ��������
	eventLoopAddTask(evLoop, channel, ADD);

	return evLoop;
}

// ������Ӧ��ģ��
int eventLoopRun(struct EventLoop* evLoop)
{
	assert(evLoop != NULL);
	// ȡ���¼��ַ��ͼ��ģ��
	struct Dispatcher* dispatcher = evLoop->dispatcher;
	// �Ƚ��߳�ID�Ƿ�����
	if (evLoop->threadID != pthread_self())
	{
		return -1;
	}
	// ѭ�������¼�����
	while (!evLoop->isQuit)
	{
		dispatcher->dispatch(evLoop, 2);    // ��ʱʱ�� 2s
		eventLoopProcessTask(evLoop);
	}
	return 0;
}

// ����������ļ�fd
int eventActivate(struct EventLoop* evLoop, int fd, int event)
{
	if (fd < 0 || evLoop == NULL)
	{
		return -1;
	}
	// ȡ��channel
	struct Channel* channel = evLoop->channelMap->list[fd];
	assert(channel->fd == fd);
	if (event & ReadEvent && channel->readCallback)
	{
		channel->readCallback(channel->arg);
	}
	if (event & WriteEvent && channel->writeCallback)
	{
		channel->writeCallback(channel->arg);
	}
	return 0;
}

// ���������������
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type)
{
	// ����, ����������Դ
	pthread_mutex_lock(&evLoop->mutex);
	// �����½ڵ�
	struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
	node->channel = channel;
	node->type = type;
	node->next = NULL;
	// ����Ϊ��
	if (evLoop->head == NULL)
	{
		evLoop->head = evLoop->tail = node;
	}
	else
	{
		evLoop->tail->next = node;  // add
		evLoop->tail = node;        // ����
	}
	pthread_mutex_unlock(&evLoop->mutex);
	// �����ڵ�
	/*
	* ϸ��:
	*   1. ���������ڵ������: �����ǵ�ǰ�߳�Ҳ�����������߳�(���߳�)
	*       1). �޸�fd���¼�, ��ǰ���̷߳���, ��ǰ���̴߳���
	*       2). �����µ�fd, ��������ڵ�Ĳ����������̷߳����
	*   2. ���������̴߳����������, ��Ҫ�ɵ�ǰ�����߳�ȡ����
	*/
	if (evLoop->threadID == pthread_self())
	{
		// ��ǰ���߳�(�������̵߳ĽǶȷ���)
		eventLoopProcessTask(evLoop);
	}
	else
	{
		// ���߳� -- �������̴߳�����������е�����
		// 1. ���߳��ڹ��� 2. ���̱߳�������:select, poll, epoll
		takeWakeup(evLoop);
	}
	return 0;
}

// ������������е�����
int eventLoopProcessTask(struct EventLoop* evLoop)
{
    pthread_mutex_lock(&evLoop->mutex);
    // ȡ��ͷ���
    struct ChannelElement* head = evLoop->head;
    while (head != NULL)
    {
        struct Channel* channel = head->channel;
        if (head->type == ADD)
        {
            // ����
            eventLoopAdd(evLoop, channel);
        }
        else if (head->type == DELETE)
        {
            // ɾ��
            eventLoopRemove(evLoop, channel);
        }
        else if (head->type == MODIFY)
        {
            // �޸�
            eventLoopModify(evLoop, channel);
        }
        struct ChannelElement* tmp = head;
        head = head->next;
        free(tmp);
    }
    evLoop->head = evLoop->tail = NULL;
    pthread_mutex_unlock(&evLoop->mutex);
    return 0;
}

// ����dispatcher�еĽڵ�
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (fd >= channelMap->size)
	{
		// û���㹻�Ŀռ�洢��ֵ�� fd - channel ==������
		if (!makeMapRoom(channelMap, fd, sizeof(struct Channel*)))
		{
			return -1;
		}
	}
	// �ҵ�fd'��Ӧ������Ԫ��λ�ã����洢
	if (channelMap->list[fd] == NULL)
	{
		channelMap->list[fd] = channel;
		evLoop->dispatcher->add(channel, evLoop);
	}
	return 0;
}
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (fd >= channelMap->size)
	{
		return -1;
	}
	return evLoop->dispatcher->remove(channel, evLoop);
}
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel)
{
	int fd = channel->fd;
	struct ChannelMap* channelMap = evLoop->channelMap;
	if (channelMap->list[fd] == NULL)
	{
		return -1;
	}
	return evLoop->dispatcher->modify(channel, evLoop);
}

// �ͷ�channel 
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel)
{
	// ɾ��channel �� fd�Ķ�Ӧ��ϵ
	evLoop->channelMap->list[channel->fd] = NULL;
	// �ر�fd
	close(channel->fd);
	// �ͷ�channel
	free(channel);
	return 0;
}

