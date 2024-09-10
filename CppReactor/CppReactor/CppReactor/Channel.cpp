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

//�޸�fd��д�¼������ or ����⣩
void Channel::writeEventEnable(bool flag)
{
    if (flag)
    {
        // m_events |= (int)FDEvent::WriteEvent;   ---  C����д��
        m_events |= static_cast<int>(FDEvent::WriteEvent);
    }
    else
    {
        m_events = m_events & ~static_cast<int>(FDEvent::WriteEvent);    // ��WriteEventȡ��
    }
}

// �ж��Ƿ���Ҫ����ļ���������д�¼�
bool Channel::isWriteEventEnable()
{
    return m_events & static_cast<int>(FDEvent::WriteEvent);
}


