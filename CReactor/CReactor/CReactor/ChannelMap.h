#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ChannelMap
{
	int size;		// ��¼ָ��ָ��������Ԫ���ܸ���
	// struct Channel* list[]     �±��Ӧ�ļ�������fd�� list��fd���Ƕ�Ӧ��Channel��ַ
	struct Channel** list;
};

// ��ʼ��
struct ChannelMap* channelMapInit(int size);

// ���Map
void ChannelMapClear(struct ChannelMap* map);

// ���ݺ��� --- ���·����ڴ�ռ�(��ChannelMap�Ĵ�С���Լ���Ԫ��С��
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);