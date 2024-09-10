#include "WorkerThread.h"
#include <stdio.h>
#include <stdlib.h>

// 初始化
int workerThreadInit(struct WorkerThread* thread, int index)
{
	thread->evLoop = NULL;
	thread->threadID = 0;
	sprintf(thread->name, "SubThread-%d", index);
	pthread_mutex_init(&thread->mutex, NULL);
	pthread_cond_init(&thread->cond, NULL);

	return 0;
}

// 子线程的回调函数
void* subThreadRunning(void* arg)
{
	struct WorkerThread* thread = (struct WorkerThread*)arg;
	pthread_mutex_lock(&thread->mutex);
	thread->evLoop = eventLoopInitEx(thread->name);
	pthread_mutex_unlock(&thread->mutex);
	pthread_cond_signal(&thread->cond);							// 发送条件信号，通知主线程事件循环已初始化
	eventLoopRun(thread->evLoop);
	return NULL;
}

// 启动线程
void workerThreadRun(struct WorkerThread* thread)
{
	// 创建子线程
	pthread_create(&thread->threadID, NULL, subThreadRunning, thread);
	// 阻塞主线程，让当前函数不会直接结束
	pthread_mutex_lock(&thread->mutex);							// 子线程中evLoop是共享资源
	while (thread->evLoop == NULL)
	{
		// 等待条件变量，阻塞主线程,这个函数在等待时会释放 mutex，直到收到信号后再次获取 mutex 继续执行
		// 当线程解除阻塞的时候，函数内部会帮助这个线程再次将这个mutex互斥锁锁上，继续向下访问临界区
		pthread_cond_wait(&thread->cond, &thread->mutex);
	}
	pthread_mutex_unlock(&thread->mutex);

}











