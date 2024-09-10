#pragma once
#include <stdbool.h>

// 定义函数指针
typedef int(*handleFunc)(void* arg);

struct Channel
{
	// 文件描述符
	int fd;
	// 事件
	int events;
	// 回调函数
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;
	// 回调函数的参数
	void* arg;
};

enum FDEvent
{
	TimeOut = 0x01,				// 一般用不上，标志读事件超时
	ReadEvent = 0x02,			// 对应二进制的10
	WriteEvent = 0x04			// 对应二进制的100
};

// 初始化一个Channel
struct Channel* channelInit(int fd, int events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);

//修改fd的写事件（检测 or 不检测）
void writeEventEnable(struct Channel* channel, bool flag);

// 判断是否需要检测文件描述符的写事件
bool isWriteEventEnable(struct Channel* channel);