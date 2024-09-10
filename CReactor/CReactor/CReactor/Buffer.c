#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>

// ��ʼ��
struct Buffer* bufferInit(int size)
{
	struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));
	if (buffer != NULL)
	{
		buffer->data = (char*)malloc(size);
		buffer->capacity = size;
		buffer->writePos = buffer->readPos = 0;
		memset(buffer->data, 0, size);
	}
	return buffer;
}

// �����ڴ�
void bufferDestroy(struct Buffer* buf)
{
	if (buf != NULL)
	{
		if (buf->data != NULL)
		{
			free(buf->data);
		}
	}
	free(buf);
}

// ����
void bufferExtendRoom(struct Buffer* buffer, int size)
{
	// 1.�ڴ湻�� - ����Ҫ����
	if (bufferWriteableSize(buffer) >= size)
	{
		return;
	}
	// 2.�ڴ���Ҫ�ϲ��Ź���(ʣ���д+�Ѷ��� - ����Ҫ����
	else if (buffer->readPos + bufferWriteableSize(buffer) >= size)
	{
		// �õ�δ�����ڴ��С
		int readable = bufferReadableSize(buffer);
		// �ƶ��ڴ�
		memcpy(buffer->data, buffer->data + buffer->readPos, readable);
		// ����λ��
		buffer->readPos = 0;
		buffer->writePos = readable;
	}
	// 3.�ڴ治���� - ����
	else
	{
		// ָ����=����������*��realloc��Ҫ�ı��ڴ��С��ָ�������µĴ�С����
		void* temp = realloc(buffer->data, buffer->capacity + size);
		if (temp == NULL)
		{
			return; //ʧ����
		}
		// ����size��0
		memset(temp + buffer->capacity, 0, size);
		// ��������
		buffer->data = temp;
		buffer->capacity += size;
	}
}

// �õ�ʣ��Ŀ�д���ڴ�����
int bufferWriteableSize(struct Buffer* buffer)
{
	return buffer->capacity - buffer->writePos;
}
// �õ�ʣ��Ŀɶ����ڴ�����
int bufferReadableSize(struct Buffer* buffer)
{
	return buffer->writePos - buffer->readPos;
}

// д�ڴ� 1.ֱ��д 2.�����׽�������
int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
	if (buffer == NULL || data == NULL || data <= 0)
	{
		return -1;
	}
	// �ж��Ƿ���Ҫ����
	bufferExtendRoom(buffer, size);
	// ���ݿ���
	memcpy(buffer->data + buffer->writePos, data, size);
	buffer->writePos += size;
	return 0;
}
int bufferAppendString(struct Buffer* buffer, const char* data)
{
	int size = strlen(data);
	int ret = bufferAppendData(buffer, data, size);
	return ret;
}

int bufferSocketRead(struct Buffer* buffer, int fd)
{
	// read\recv\readv
	struct iovec vec[2];							// readv��Ҫ�Ĳ���.��������
	// ��ʼ������Ԫ��
	int writeable = bufferWriteableSize(buffer);
	vec[0].iov_base = buffer->data + buffer->writePos;
	vec[0].iov_len = writeable;
	char* tmpbuf = (char*)malloc(40960);
	vec[1].iov_base = tmpbuf;
	vec[1].iov_len = 40960;
	int result = readv(fd, vec, 2);
	if (result == -1)
	{
		return -1;
	}
	else if (result <= writeable)
	{
		// �ڴ湻�á���ʹ��vec[0]��������
		buffer->writePos += result;
	}
	else
	{
		// �ڴ治���ã�ʹ�ö��ڴ桾1����������
		buffer->writePos = buffer->capacity;
		bufferAppendData(buffer, tmpbuf, result - writeable);
	}
	free(tmpbuf);
	return result;
}


// ����\r\nȡ��һ�У��ҵ��������ݿ��е�λ�ã����ظ�λ��
char* bufferFindCRLF(struct Buffer* buffer)
{
	// strstr --> ���ַ�����ƥ�����ַ���������\0����
	// memmem --> ��һ��������ݿ���ƥ��һ�������ݿ飨��Ҫָ�����ݿ��С
	char* ptr = memmem(buffer->data + buffer->readPos, 
		bufferReadableSize(buffer), "\r\n", 2);
	return ptr;
}

// ��������
int bufferSendData(struct Buffer* buffer, int socket)
{
	// �ж���������
	int readable = bufferReadableSize(buffer);
	if (readable > 0)
	{
		int count = send(socket, buffer->data + buffer->readPos, readable, MSG_NOSIGNAL);
		if (count)
		{
			buffer->readPos += count;
			usleep(1);
		}
		return count;
	}
	return 0;
}

















