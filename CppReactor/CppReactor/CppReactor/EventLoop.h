#pragma once
#include "Dispatcher.h"
#include <thread>
#include "Channel.h"
#include <queue>
using namespace std;
#include <map>
#include <mutex>



// ����ýڵ��е�Channel
enum class ElemType:char { ADD, DELETE, MODIFY };

// ����������еĽڵ�
struct ChannelElement
{
	ElemType type;						// ��δ���ýڵ��е�Channel
	Channel* channel;
};

class Dispatcher;						// ���߱���������������Ѿ�����

// ��Ӧ��ģ��
class EventLoop
{
public:
	EventLoop();
	EventLoop(const string threadName);

	~EventLoop();

	// ������Ӧ��ģ��
	int run();

	// ��������ļ�fd
	int eventActive(int fd, int event);

	// ��������������
	int addTask(Channel* channel, ElemType type);

	// ������������е�����
	int processTaskQ();

	// ����dispatcher�еĽڵ�
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);

	// �ͷ�channel  ----- ��Ա������������� - ��һ�����ڣ�����ʵ������õ���һ�����󣬶�����ڣ���Ա����������
	int freeChannel(Channel* channel);

	// ������ľ�̬��Ա������������ĳ�����󣬲����ھ�̬�����в���ֱ�ӷ��ʵ�ǰʵ����������this������Ҫ������Ҫ�Ѷ�����
	static int readLocalMessage(void* arg);
	int readMessage();

	inline thread::id getThreadID()
	{
		return m_threadID;
	}

private:
	void takeWakeup();


private:
	bool m_isQuit;						// ����
	Dispatcher* m_dispatcher;			// ָ��ָ�������ʵ��
	void* m_dispatcherData;				// ���ڵ� �� epoll�¼�(��д��

	// ������� ͷ��β�ڵ�
	queue<ChannelElement*> m_taskQ;
	// map
	map<int, Channel*> m_channelMap;

	// �߳�id
	thread::id m_threadID;
	string m_threadName;
	// ������ --- �����������
	mutex m_mutex;

	int m_socketPair[2];					// �洢����ͨ�ŵ�fd ͨ��socketPair ��ʼ��
};







