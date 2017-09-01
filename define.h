#pragma once

#include "stdafx.h"
#include <vector>
#include "Lock.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#define MAX_CHANNELS (4)
#define CHANNEL_BACKGROUND ("background")


struct FrameInfo
{
	unsigned int width;
	unsigned int height;
	unsigned int format;  //ͼ���ʽ
	unsigned int size;
};

struct FrameData
{
	char* data;  //ͼ������
	int index;
	bool bIsNew;
};

struct TransformFrameInfo
{
	unsigned int width;
	unsigned int height;
	unsigned int format;  //ͼ���ʽ
	unsigned int size;
	unsigned int x;
	unsigned int y;
};