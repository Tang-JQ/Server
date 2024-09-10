#include "ThreadPool.h"

// ��ʼ���̳߳�
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count)
{
	struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
	pool->index = 0;
	pool->isStart = false;
	pool->mainLoop = mainLoop;
	pool->threadNum = count;
	pool->workerThreads = (struct WorkerThread*)malloc(sizeof(struct WorkerThread) * count);
	return pool;
}


// �����̳߳�
void threadPoolRun(struct ThreadPool* pool)
{
	assert(pool && !pool->isStart);
	if (pool->mainLoop->threadID != pthread_self())
	{
		// �����̳߳ص�Ϊ���߳�
		exit(0);
	}
	pool->isStart = true;
	if (pool->threadNum)
	{
		for (int i = 0; i < pool->threadNum; i++)
		{
			workerThreadInit(&pool->workerThreads[i], i);
			workerThreadRun(&pool->workerThreads[i]);
		}
	}
}

// ȡ���̳߳��е�ĳ�����̵߳ķ�Ӧ��ʵ��
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
	assert(pool->isStart);
	if (pool->mainLoop->threadID != pthread_self())
	{
		exit(0);
	}
	// ���̳߳�����һ�����̣߳�Ȼ��ȡ�����еķ�Ӧ��ʵ��
	struct EventLoop* evLoop = pool->mainLoop;
	if (pool->threadNum > 0)
	{
		evLoop = pool->workerThreads[pool->index].evLoop;
		int t = ++pool->index;
		pool->index = t % pool->threadNum;
	}
	return evLoop;
}









