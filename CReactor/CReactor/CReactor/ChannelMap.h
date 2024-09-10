#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ChannelMap
{
	int size;		// 记录指针指向的数组的元素总个数
	// struct Channel* list[]     下标对应文件描述符fd， list【fd】是对应的Channel地址
	struct Channel** list;
};

// 初始化
struct ChannelMap* channelMapInit(int size);

// 清空Map
void ChannelMapClear(struct ChannelMap* map);

// 扩容函数 --- 重新分配内存空间(新ChannelMap的大小，以及单元大小）
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);