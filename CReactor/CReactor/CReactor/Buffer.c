#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>

// 初始化
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

// 销毁内存
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

// 扩容
void bufferExtendRoom(struct Buffer* buffer, int size)
{
	// 1.内存够用 - 不需要扩容
	if (bufferWriteableSize(buffer) >= size)
	{
		return;
	}
	// 2.内存需要合并才够用(剩余可写+已读） - 不需要扩容
	else if (buffer->readPos + bufferWriteableSize(buffer) >= size)
	{
		// 得到未读的内存大小
		int readable = bufferReadableSize(buffer);
		// 移动内存
		memcpy(buffer->data, buffer->data + buffer->readPos, readable);
		// 更新位置
		buffer->readPos = 0;
		buffer->writePos = readable;
	}
	// 3.内存不够用 - 扩容
	else
	{
		// 指针名=（数据类型*）realloc（要改变内存大小的指针名，新的大小）。
		void* temp = realloc(buffer->data, buffer->capacity + size);
		if (temp == NULL)
		{
			return; //失败了
		}
		// 将后size赋0
		memset(temp + buffer->capacity, 0, size);
		// 更新数据
		buffer->data = temp;
		buffer->capacity += size;
	}
}

// 得到剩余的可写的内存容量
int bufferWriteableSize(struct Buffer* buffer)
{
	return buffer->capacity - buffer->writePos;
}
// 得到剩余的可读的内存容量
int bufferReadableSize(struct Buffer* buffer)
{
	return buffer->writePos - buffer->readPos;
}

// 写内存 1.直接写 2.接收套接字数据
int bufferAppendData(struct Buffer* buffer, const char* data, int size)
{
	if (buffer == NULL || data == NULL || data <= 0)
	{
		return -1;
	}
	// 判断是否需要扩容
	bufferExtendRoom(buffer, size);
	// 数据拷贝
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
	struct iovec vec[2];							// readv需要的参数.接收数据
	// 初始化数组元素
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
		// 内存够用。仅使用vec[0]接收数据
		buffer->writePos += result;
	}
	else
	{
		// 内存不够用，使用堆内存【1】接收数据
		buffer->writePos = buffer->capacity;
		bufferAppendData(buffer, tmpbuf, result - writeable);
	}
	free(tmpbuf);
	return result;
}


// 根据\r\n取出一行，找到其在数据库中的位置，返回该位置
char* bufferFindCRLF(struct Buffer* buffer)
{
	// strstr --> 大字符串中匹配子字符串（遇到\0结束
	// memmem --> 从一个大的数据块中匹配一个子数据块（需要指定数据库大小
	char* ptr = memmem(buffer->data + buffer->readPos, 
		bufferReadableSize(buffer), "\r\n", 2);
	return ptr;
}

// 发送数据
int bufferSendData(struct Buffer* buffer, int socket)
{
	// 判断有无数据
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

















