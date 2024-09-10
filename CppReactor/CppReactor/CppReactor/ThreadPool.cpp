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
	// C++11�����ԣ����ڷ�Χ��forѭ��
	for (auto item : m_workerThreads)
	{
		delete item;
	}
}

// �����̳߳�
void ThreadPool::run()
{
	assert(!m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id())
	{
		// �����̳߳ص�Ϊ���߳�
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


// ȡ���̳߳��е�ĳ�����̵߳ķ�Ӧ��ʵ��
EventLoop* ThreadPool::takeWorkerEventLoop()
{
	assert(m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id())
	{
		exit(0);
	}
	// ���̳߳�����һ�����̣߳�Ȼ��ȡ�����еķ�Ӧ��ʵ��
	EventLoop* evLoop = m_mainLoop;
	if (m_threadNum > 0)
	{
		evLoop = m_workerThreads[m_index]->getEventLoop();
		int t = ++m_index;
		m_index = t % m_threadNum;
	}
	return evLoop;
}




