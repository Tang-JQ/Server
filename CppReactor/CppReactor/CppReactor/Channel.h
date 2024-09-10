#pragma once
#include <functional>

// ���庯��ָ��
// typedef int(*handleFunc)(void* arg);			---- C����д��
//using handleFunc = int(*)(void*);

enum class FDEvent
{
	TimeOut = 0x01,				// һ���ò��ϣ���־���¼���ʱ
	ReadEvent = 0x02,			// ��Ӧ�����Ƶ�10
	WriteEvent = 0x04			// ��Ӧ�����Ƶ�100
};


// �ɵ��ö����װ���������ʲô�� 1.����ָ�룬 2.�ɵ��ö��󣨿�������һ��ʹ��
// ���յõ��˵�ַ������û�е���
class Channel
{
public:
	using handleFunc = std::function<int(void*)>;
	Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);
	// �ص����� --- ���ͣ��ɵ��ö����װ������
	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;

	//�޸�fd��д�¼������ or ����⣩
	void writeEventEnable(bool flag);

	// �ж��Ƿ���Ҫ����ļ���������д�¼�
	bool isWriteEventEnable();

	// ȡ��˽�г�Ա��ֵ     ----------- �������� -- ֱ��ѹջ
	inline int getEvent()
	{
		return m_events;
	}
	inline int getSocket()
	{
		return m_fd;
	}
	inline const void* getArg()				// const ֻ�����ⲿ�����޸�
	{
		return m_arg;
	}

private:
	// �ļ�������
	int m_fd;
	// �¼�
	int m_events;
	// �ص������Ĳ���
	void* m_arg;
};


