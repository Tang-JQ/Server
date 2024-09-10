#pragma once
#include "Dispatcher.h"
#include "ChannelMap.h"
#include <stdbool.h>
#include <pthread.h>
#include "Channel.h"


extern struct Dispatcher EpollDispatcher;

// 处理该节点中的Channel
enum ElemType { ADD, DELETE, MODIFY };

// 定义任务队列的节点
struct ChannelElement
{
	int type;						// 如何处理该节点中的Channel
	struct Channel* channel;
	struct ChannelElement* next;
};

struct Dispatcher;
// 反应堆模型
struct EventLoop
{
	bool isQuit;						// 开关
	struct Dispatcher* dispatcher;
	void* dispatcherData;				// 根节点 和 epoll事件(读写等
	// 任务队列 头和尾节点
	struct ChannelElement* head;
	struct ChannelElement* tail;
	// map任务队列和文件描述符的对应关系
	struct ChannelMap* channelMap;
	// 线程id
	pthread_t threadID;
	char threadName[32];
	// 互斥锁 --- 保护任务队列
	pthread_mutex_t mutex;

	int socketPair[2];					// 存储本地通信的fd 通过socketPair 初始化
};


// 初始化
struct EventLoop* eventLoopInit();
struct EventLoop* eventLoopInitEx(const char* threadName);

// 启动反应堆模型
int eventLoopRun(struct EventLoop* evLoop);

// 处理激活的文件fd
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// 添加任务到任务队列
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);

// 处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);

// 处理dispatcher中的节点
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);

// 释放channel 
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);




