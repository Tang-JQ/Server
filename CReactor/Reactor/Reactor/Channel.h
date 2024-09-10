#pragma once
#include <stdbool.h>

// ���庯��ָ��
typedef int(*handleFunc)(void* arg);

struct Channel
{
	// �ļ�������
	int fd;
	// �¼�
	int events;
	// �ص�����
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;
	// �ص������Ĳ���
	void* arg;
};

enum FDEvent
{
	TimeOut = 0x01,				// һ���ò��ϣ���־���¼���ʱ
	ReadEvent = 0x02,			// ��Ӧ�����Ƶ�10
	WriteEvent = 0x04			// ��Ӧ�����Ƶ�100
};

// ��ʼ��һ��Channel
struct Channel* channelInit(int fd, int events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);

//�޸�fd��д�¼������ or ����⣩
void writeEventEnable(struct Channel* channel, bool flag);

// �ж��Ƿ���Ҫ����ļ���������д�¼�
bool isWriteEventEnable(struct Channel* channel);