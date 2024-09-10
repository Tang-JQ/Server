#pragma once
#include "Channel.h"
#include "EventLoop.h"

struct EventLoop;
struct Dispatcher
{
	// init  --- ��ʼ��epoll��poll����select����Ҫ�����ݿ�
	void* (*init)();
	// ���
	int (*add)(struct Channel* channel, struct EventLoop* evloop);
	// ɾ��
	int (*remove)(struct Channel* channel, struct EventLoop* evloop);
	// �޸�
	int (*modify)(struct Channel* channel, struct EventLoop* evloop);
	// �¼����
	int (*dispatch)(struct EventLoop* evloop, int timeout);		// ��ʱʱ������λ;s
	// ������ݣ��ر�fd�����ͷ��ڴ棩
	int (*clear)(struct EventLoop* evloop);
};