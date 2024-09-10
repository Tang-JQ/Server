#include "WorkerThread.h"
#include <stdio.h>
#include <stdlib.h>


WorkerThread::WorkerThread(int index)
{
	m_evLoop = nullptr;
	m_thread = nullptr;
	m_threadID = thread::id();				// 得到一个无效的id
	m_name = "SubThread-" + to_string(index);
}

WorkerThread::~WorkerThread()
{
	if (m_thread != nullptr)
	{
		delete m_thread;
	}
}

// 启动线程
void WorkerThread::run()
{
	// 创建子线程
	m_thread = new thread(&WorkerThread::running, this);
	// 阻塞主线程，让当前函数不会直接结束
	// unique_lock是一个模板类，封装了互斥锁，定义出了自动加锁 
	unique_lock<mutex>locker(m_mutex);
	while (m_evLoop == nullptr)
	{
		// 调用wait时，自动把locker解开，m_cond调用notify_one之后
		m_cond.wait(locker);		
	}
}


// 子线程的回调函数
void WorkerThread::running()
{
	m_mutex.lock();
	m_evLoop = new EventLoop(m_name);
	m_mutex.unlock();
	m_cond.notify_one();				
	m_evLoop->run();
}







