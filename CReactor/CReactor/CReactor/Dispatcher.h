#pragma once
#include "Channel.h"
#include "EventLoop.h"

struct EventLoop;
struct Dispatcher
{
	// init  --- 初始化epoll、poll或者select所需要的数据块
	void* (*init)();
	// 添加
	int (*add)(struct Channel* channel, struct EventLoop* evloop);
	// 删除
	int (*remove)(struct Channel* channel, struct EventLoop* evloop);
	// 修改
	int (*modify)(struct Channel* channel, struct EventLoop* evloop);
	// 事件检测
	int (*dispatch)(struct EventLoop* evloop, int timeout);		// 超时时长，单位;s
	// 清空数据（关闭fd或者释放内存）
	int (*clear)(struct EventLoop* evloop);
};