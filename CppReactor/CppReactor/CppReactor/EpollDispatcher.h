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
	// ���
	int add() override;					// override��ʾ�ú����Ӹ���̳���������������д
	// ɾ��
	int remove() override;
	// �޸�
	int modify() override;
	// �¼����
	int dispatch(int timeout = 2) override;		// ��ʱʱ������λ;s


private:
	int epollCtl(int op);

private:
	// ���ڵ� �� epoll�¼�(��д��
	int m_epfd;
	struct epoll_event* m_events;
	const int m_maxNode = 520;
};