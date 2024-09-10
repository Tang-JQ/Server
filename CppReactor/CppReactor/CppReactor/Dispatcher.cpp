#include "Dispatcher.h"

Dispatcher::Dispatcher(EventLoop* evloop) : m_evLoop(evloop)
{

}

Dispatcher::~Dispatcher()
{

}

// 添加
int Dispatcher::add()
{
	return 0;
}

// 删除
int Dispatcher::remove()
{
	return 0;
}

// 修改
int Dispatcher::modify()
{
	return 0;
}

// 事件检测
int Dispatcher::dispatch(int timeout)
{
	return 0;
}
