#pragma once
#include "Channel.h"
#include "EventLoop.h"
#include <string>
using namespace std;

class EventLoop;

class Dispatcher
{
public:
	Dispatcher(EventLoop* evloop);
	virtual ~Dispatcher();
	// 添加
	virtual int add();
	// 删除
	virtual int remove();
	// 修改
	virtual int modify();
	// 事件检测
	virtual int dispatch(int timeout = 2);		// 超时时长，单位;s

	inline void setChannel(Channel* channel)
	{
		m_channel = channel;
	}

protected:						// protect 权限受保护的，类的外部不能访问，但是可以被子类继承（私有成员不能被继承
	string m_name = string();	//创建一个名为 m_name 的字符串对象，并使用默认构造函数进行初始化,即空
	Channel* m_channel;
	EventLoop* m_evLoop;
};