#include "WorkerThread.h"
#include <stdio.h>
#include <stdlib.h>


WorkerThread::WorkerThread(int index)
{
	m_evLoop = nullptr;
	m_thread = nullptr;
	m_threadID = thread::id();				// �õ�һ����Ч��id
	m_name = "SubThread-" + to_string(index);
}

WorkerThread::~WorkerThread()
{
	if (m_thread != nullptr)
	{
		delete m_thread;
	}
}

// �����߳�
void WorkerThread::run()
{
	// �������߳�
	m_thread = new thread(&WorkerThread::running, this);
	// �������̣߳��õ�ǰ��������ֱ�ӽ���
	// unique_lock��һ��ģ���࣬��װ�˻���������������Զ����� 
	unique_lock<mutex>locker(m_mutex);
	while (m_evLoop == nullptr)
	{
		// ����waitʱ���Զ���locker�⿪��m_cond����notify_one֮��
		m_cond.wait(locker);		
	}
}


// ���̵߳Ļص�����
void WorkerThread::running()
{
	m_mutex.lock();
	m_evLoop = new EventLoop(m_name);
	m_mutex.unlock();
	m_cond.notify_one();				
	m_evLoop->run();
}






