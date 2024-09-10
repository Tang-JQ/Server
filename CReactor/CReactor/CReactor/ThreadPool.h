#pragma once
#include "EventLoop.h"
#include <stdbool.h>
#include "WorkerThread.h"
#include <assert.h>

// �����̳߳�
struct ThreadPool
{
	// ���̵߳ķ�Ӧ��ģ��
	struct EventLoop* mainLoop;
	bool isStart;
	int threadNum;
	struct WorkerThread* workerThreads;
	int index;									// ��Ч�̵߳�����
};

// ��ʼ���̳߳�
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count);

// �����̳߳�
void threadPoolRun(struct ThreadPool* pool);


// ȡ���̳߳��е�ĳ�����̵߳ķ�Ӧ��ʵ��
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);











