#include "ThreadPool.h"
#include <assert.h>

ThreadPool::ThreadPool(EventLoop* mainLoop, int count)
{
	m_index = 0;
	m_isStart = false;
	m_mainLoop = mainLoop;
	m_threadNum = count;
	m_workerThreads.clear();
}

ThreadPool::~ThreadPool()
{
	// C++11新特性，基于范围的for循环
	for (auto item : m_workerThreads)
	{
		delete item;
	}
}

// 启动线程池
void ThreadPool::run()
{
	assert(!m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id())
	{
		// 启动线程池的为主线程
		exit(0);
	}
	m_isStart = true;
	if (m_threadNum > 0)
	{
		for (int i = 0; i < m_threadNum; i++)
		{
			WorkerThread* subThread = new WorkerThread(i);
			subThread->run();
			m_workerThreads.push_back(subThread);
		}
	}
}


// 取出线程池中的某个子线程的反应堆实例
EventLoop* ThreadPool::takeWorkerEventLoop()
{
	assert(m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id())
	{
		exit(0);
	}
	// 从线程池中找一个子线程，然后取出其中的反应堆实例
	EventLoop* evLoop = m_mainLoop;
	if (m_threadNum > 0)
	{
		evLoop = m_workerThreads[m_index]->getEventLoop();
		int t = ++m_index;
		m_index = t % m_threadNum;
	}
	return evLoop;
}




