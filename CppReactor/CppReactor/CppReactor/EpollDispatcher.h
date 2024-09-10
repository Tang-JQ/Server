#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include "Dispatcher.h"
#include <string>
#include <sys/epoll.h>


using namespace std;

struct EventLoop;

class EpollDispatcher : public Dispatcher
{
public:
	EpollDispatcher(EventLoop* evloop);
	~EpollDispatcher();
	// 添加
	int add() override;					// override表示该函数从父类继承来，并在子类重写
	// 删除
	int remove() override;
	// 修改
	int modify() override;
	// 事件检测
	int dispatch(int timeout = 2) override;		// 超时时长，单位;s


private:
	int epollCtl(int op);

private:
	// 根节点 和 epoll事件(读写等
	int m_epfd;
	struct epoll_event* m_events;
	const int m_maxNode = 520;
};