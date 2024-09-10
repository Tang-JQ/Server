#include "WorkerThread.h"
#include <stdio.h>
#include <stdlib.h>

// ��ʼ��
int workerThreadInit(struct WorkerThread* thread, int index)
{
	thread->evLoop = NULL;
	thread->threadID = 0;
	sprintf(thread->name, "SubThread-%d", index);
	pthread_mutex_init(&thread->mutex, NULL);
	pthread_cond_init(&thread->cond, NULL);

	return 0;
}

// ���̵߳Ļص�����
void* subThreadRunning(void* arg)
{
	struct WorkerThread* thread = (struct WorkerThread*)arg;
	pthread_mutex_lock(&thread->mutex);
	thread->evLoop = eventLoopInitEx(thread->name);
	pthread_mutex_unlock(&thread->mutex);
	pthread_cond_signal(&thread->cond);							// ���������źţ�֪ͨ���߳��¼�ѭ���ѳ�ʼ��
	eventLoopRun(thread->evLoop);
	return NULL;
}

// �����߳�
void workerThreadRun(struct WorkerThread* thread)
{
	// �������߳�
	pthread_create(&thread->threadID, NULL, subThreadRunning, thread);
	// �������̣߳��õ�ǰ��������ֱ�ӽ���
	pthread_mutex_lock(&thread->mutex);							// ���߳���evLoop�ǹ�����Դ
	while (thread->evLoop == NULL)
	{
		// �ȴ������������������߳�,��������ڵȴ�ʱ���ͷ� mutex��ֱ���յ��źź��ٴλ�ȡ mutex ����ִ��
		// ���߳̽��������ʱ�򣬺����ڲ����������߳��ٴν����mutex���������ϣ��������·����ٽ���
		pthread_cond_wait(&thread->cond, &thread->mutex);
	}
	pthread_mutex_unlock(&thread->mutex);

}










