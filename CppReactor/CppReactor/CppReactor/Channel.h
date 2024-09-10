#pragma once
#include <functional>

// 定义函数指针
// typedef int(*handleFunc)(void* arg);			---- C类型写法
//using handleFunc = int(*)(void*);

enum class FDEvent
{
	TimeOut = 0x01,				// 一般用不上，标志读事件超时
	ReadEvent = 0x02,			// 对应二进制的10
	WriteEvent = 0x04			// 对应二进制的100
};


// 可调用对象包装器打包的是什么？ 1.函数指针， 2.可调用对象（可以像函数一样使用
// 最终得到了地址，但是没有调用
class Channel
{
public:
	using handleFunc = std::function<int(void*)>;
	Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);
	// 回调函数 --- 类型：可调用对象包装器类型
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;

	//修改fd的写事件（检测 or 不检测）
	void writeEventEnable(bool flag);

	// 判断是否需要检测文件描述符的写事件
	bool isWriteEventEnable();

	// 取出私有成员的值     ----------- 内联函数 -- 直接压栈
	inline int getEvent()
	{
		return m_events;
	}
	inline int getSocket()
	{
		return m_fd;
	}
	inline const void* getArg()				// const 只读，外部不能修改
	{
		return m_arg;
	}

private:
	// 文件描述符
	int m_fd;
	// 事件
	int m_events;
	// 回调函数的参数
	void* m_arg;
};


