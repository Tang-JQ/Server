#include "Channel.h"
#include <stdlib.h>


// 初始化一个Channel
struct Channel* channelInit(int fd, int events, handleFunc readFunc,
    handleFunc writeFunc, handleFunc destroyFunc, void* arg) {
    struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
    channel->arg = arg;
    channel->fd = fd;
    channel->events = events;
    channel->readCallback = readFunc;
    channel->writeCallback = writeFunc;
    channel->destroyCallback = destroyFunc;
    return channel;
}


//修改fd的写事件（检测 or 不检测）
void writeEventEnable(struct Channel* channel, bool flag)
{
    if (flag)
    {
        channel->events |= WriteEvent;
    }
    else
    {
        channel->events = channel->events & ~WriteEvent;    // 把WriteEvent取反
    }
}

// 判断是否需要检测文件描述符的写事件
bool isWriteEventEnable(struct Channel* channel)
{
    return channel->events & WriteEvent;
}