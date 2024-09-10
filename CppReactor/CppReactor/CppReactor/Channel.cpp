#include "Channel.h"
#include <stdlib.h>


Channel::Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg)
{
    m_arg = arg;
    m_fd = fd;
    m_events = (int)events;
    readCallback = readFunc;
    writeCallback = writeFunc;
    destroyCallback = destroyFunc;
}

//修改fd的写事件（检测 or 不检测）
void Channel::writeEventEnable(bool flag)
{
    if (flag)
    {
        // m_events |= (int)FDEvent::WriteEvent;   ---  C类型写法
        m_events |= static_cast<int>(FDEvent::WriteEvent);
    }
    else
    {
        m_events = m_events & ~static_cast<int>(FDEvent::WriteEvent);    // 把WriteEvent取反
    }
}

// 判断是否需要检测文件描述符的写事件
bool Channel::isWriteEventEnable()
{
    return m_events & static_cast<int>(FDEvent::WriteEvent);
}


