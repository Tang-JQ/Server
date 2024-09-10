#include "ThreadPool.h"

// 初始化线程池
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


// 启动线程池
void threadPoolRun(struct ThreadPool* pool)
{
	assert(pool && !pool->isStart);
	if (pool->mainLoop->threadID != pthread_self())
	{
		// 启动线程池的为主线程
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

// 取出线程池中的某个子线程的反应堆实例
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
{
	assert(pool->isStart);
	if (pool->mainLoop->threadID != pthread_self())
	{
		exit(0);
	}
	// 从线程池中找一个子线程，然后取出其中的反应堆实例
	struct EventLoop* evLoop = pool->mainLoop;
	if (pool->threadNum > 0)
	{
		evLoop = pool->workerThreads[pool->index].evLoop;
		int t = ++pool->index;
		pool->index = t % pool->threadNum;
	}
	return evLoop;
}









