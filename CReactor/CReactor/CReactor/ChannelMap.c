#include "ChannelMap.h"

// 初始化
struct ChannelMap* channelMapInit(int size)
{
	struct ChannelMap* map = (struct ChannelMap*)malloc(sizeof(struct ChannelMap));
	map->size = size;
	map->list = (struct Chanel**)malloc(size * sizeof(struct Channel*));
	return map;
}

// 清空Map
void ChannelMapClear(struct ChannelMap* map)
{
	if (map != NULL)
	{
		for (int i = 0; i < map->size; ++i)
		{
			if (map->list[i] != NULL)
			{
				free(map->list[i]);
			}
		}
		free(map->list);
		map->list = NULL;
	}
	map->size = 0;
}


// 扩容函数 --- 重新分配内存空间(新ChannelMap的大小，以及单元大小）
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize)
{
	if (map->size < newSize)
	{
		int curSize = map->size;
		// 容量每次扩大为原来的一倍
		while (curSize < newSize)
		{
			curSize *= 2;
		}
		// 扩容 realloc,需要判断返回值（因为可能重新开辟了空间，并未在后面增加
		struct Channel** temp = realloc(map->list, curSize * unitSize);
		if (temp == NULL)
		{
			return false;
		}
		map->list = temp;
		// 初始化 新增
		memset(&map->list[map->size], 0, (curSize - map->size) * unitSize);
		map->size = curSize;
	}
	return true;
}





