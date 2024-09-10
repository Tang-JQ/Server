#pragma once
#include "Dispatcher.h"
#include "ChannelMap.h"
#include <stdbool.h>
#include <pthread.h>
#include "Channel.h"


extern struct Dispatcher EpollDispatcher;

// ����ýڵ��е�Channel
enum ElemType { ADD, DELETE, MODIFY };

// ����������еĽڵ�
struct ChannelElement
{
	int type;						// ��δ���ýڵ��е�Channel
	struct Channel* channel;
	struct ChannelElement* next;
};

struct Dispatcher;
// ��Ӧ��ģ��
struct EventLoop
{
	bool isQuit;						// ����
	struct Dispatcher* dispatcher;
	void* dispatcherData;				// ���ڵ� �� epoll�¼�(��д��
	// ������� ͷ��β�ڵ�
	struct ChannelElement* head;
	struct ChannelElement* tail;
	// map������к��ļ��������Ķ�Ӧ��ϵ
	struct ChannelMap* channelMap;
	// �߳�id
	pthread_t threadID;
	char threadName[32];
	// ������ --- �����������
	pthread_mutex_t mutex;

	int socketPair[2];					// �洢����ͨ�ŵ�fd ͨ��socketPair ��ʼ��
};


// ��ʼ��
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);

// ������Ӧ��ģ��
int eventLoopRun(struct EventLoop* evLoop);

// ��������ļ�fd
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// ��������������
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);

// ������������е�����
int eventLoopProcessTask(struct EventLoop* evLoop);

// ����dispatcher�еĽڵ�
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);

// �ͷ�channel 
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);




