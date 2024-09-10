#pragma once
#include "Dispatcher.h"
#include <thread>
#include "Channel.h"
#include <queue>
using namespace std;
#include <map>
#include <mutex>



// 处理该节点中的Channel
enum class ElemType:char { ADD, DELETE, MODIFY };

// 定义任务队列的节点
struct ChannelElement
{
	ElemType type;						// 如何处理该节点中的Channel
	Channel* channel;
};

class Dispatcher;						// 告诉编译器，这个声明已经存在

// 反应堆模型
class EventLoop
{
public:
	EventLoop();
	EventLoop(const string threadName);

	~EventLoop();

	// 启动反应堆模型
	int run();

	// 处理激活的文件fd
	int eventActive(int fd, int event);

	// 添加任务到任务队列
	int addTask(Channel* channel, ElemType type);

	// 处理任务队列中的任务
	int processTaskQ();

	// 处理dispatcher中的节点
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);

	// 释放channel  ----- 成员函数，定义出来 - 不一定存在，对类实例化后得到的一个对象，对象存在，成员函数即存在
	int freeChannel(Channel* channel);

	// 类里面的静态成员函数，不属于某个对象，不能在静态函数中不能直接访问当前实例化出来的this对象，想要访问需要把对象传入
	static int readLocalMessage(void* arg);
	int readMessage();

	inline thread::id getThreadID()
	{
		return m_threadID;
	}

private:
	void takeWakeup();


private:
	bool m_isQuit;						// 开关
	Dispatcher* m_dispatcher;			// 指针指向子类的实例
	void* m_dispatcherData;				// 根节点 和 epoll事件(读写等

	// 任务队列 头和尾节点
	queue<ChannelElement*> m_taskQ;
	// map
	map<int, Channel*> m_channelMap;

	// 线程id
	thread::id m_threadID;
	string m_threadName;
	// 互斥锁 --- 保护任务队列
	mutex m_mutex;

	int m_socketPair[2];					// 存储本地通信的fd 通过socketPair 初始化
};







