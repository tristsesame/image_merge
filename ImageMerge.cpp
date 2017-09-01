#include "define.h"
#include "ImageMerge.h"
#include <math.h>
#include "ximage.h"
#include <memory>
#include <map>
#include <string>
#include <iostream>
#include "ImageProcessThread.h"
#include "FormatConvert.c"

#ifdef DEBUG_LOG_FILE
#include <fstream>

static std::ofstream *m_of;
static std::wstring  file_path;
#endif




static std::vector<FrameInfo> g_vec_frame_info;
static std::vector<FrameData> g_vec_frame_data;
static std::vector<bool> g_vec_frame_available;
static std::map<std::string,int> g_map_channel;

//记录下背景图大小，即channel 0 图像大小
static int background_width;
static int background_height;

//用于存放需转化后的大小
static std::vector<TransformFrameInfo> g_vec_frame_resample_info;

static bool g_init;

static bool g_new_image_arrive;

static CImageProcessThread worker;


//lock
//用路某路添加图像使用
static base::Lock	img_process_lock;
//某路设置时使用
static base::Lock	multi_channel_set_lock;

void image_merge_init()
{
	g_init = true;


	FrameInfo info;
	FrameData data;

	info.width = 0;
	info.height = 0;
	info.format = -1;
	info.size = 0;

	background_width = 800;
	background_height = 600;

	data.data = nullptr;
	data.index = 0;


	g_vec_frame_available.clear();
	g_vec_frame_info.clear();
	g_vec_frame_data.clear();

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		g_vec_frame_available.push_back(false);
		g_vec_frame_info.push_back(info);
		g_vec_frame_data.push_back(data);
	}

	g_map_channel.clear();

	g_map_channel[CHANNEL_BACKGROUND] = 0;

	g_new_image_arrive = false;

	SwsContext *g_img_convert_ctx = nullptr;
	g_pFrame = nullptr;
	g_pFrameRGB = nullptr;
	g_out_buffer = nullptr;

	g_img_convert_ctx_format = nullptr;
	g_out_buffer_format = nullptr;
	g_pFrame_format_I420 = nullptr;
	g_pFrame_format_RGB24 = nullptr;


#ifdef DEBUG_LOG_FILE
	TCHAR szDir[512];
	GetModuleFileName(0, szDir, 513);
	int i;
	i = lstrlen(szDir) - 1;
	while (i > 0)
	{
		if (szDir[i] == _T('\\'))
		{
			szDir[i] = 0;
			break;
		}
		i--;
	}

	swprintf_s(szDir, _T("%s\\ybmerge_debug.log"), szDir);
	file_path = szDir;
	m_of = new std::ofstream(file_path);
#endif

}

void image_merge_uninit()
{
	g_init = false;

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		g_vec_frame_available.push_back(false);

		if (g_vec_frame_data[i].data != nullptr)
		{
			delete g_vec_frame_data[i].data;
			g_vec_frame_data[i].data = nullptr;
		}
	}

	if (g_img_convert_ctx){
		sws_freeContext(g_img_convert_ctx);
		g_img_convert_ctx = NULL;
	}
	if (g_pFrame){
		av_free(g_pFrame);
		g_pFrame = NULL;
	}
	if (g_pFrameRGB){
		av_free(g_pFrameRGB);
		g_pFrameRGB = NULL;
	}
	if (g_out_buffer){
		delete[]g_out_buffer;
		g_out_buffer = NULL;
	}

	if (g_img_convert_ctx_format){
		sws_freeContext(g_img_convert_ctx_format);
		g_img_convert_ctx_format = NULL;
	}
	if (g_pFrame_format_I420){
		av_free(g_pFrame_format_I420);
		g_pFrame_format_I420 = NULL;
	}
	if (g_pFrame_format_RGB24){
		av_free(g_pFrame_format_RGB24);
		g_pFrame_format_RGB24 = NULL;
	}
	if (g_out_buffer_format){
		delete[]g_out_buffer_format;
		g_out_buffer_format = NULL;
	}

#ifdef DEBUG_LOG_FILE
	m_of->flush();
	m_of->close();
	delete m_of;
#endif
}

bool GetAvailChannelId(int& channel)
{
	for (int i = 1; i < MAX_CHANNELS; i++)
	{
		if (!g_vec_frame_available[i])
		{
			channel = i;
			return true;
		}
	}
	return false;
}

std::vector<int> GetChannelUsed()
{
	std::vector<int> vec_channel;
	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		if (g_vec_frame_available[i])
		{
			vec_channel.push_back(i);
		}
	}
	return vec_channel;
}

//是否多人连麦
bool IsMultiUserOnline()
{
	for (int i = 1; i < MAX_CHANNELS; i++)
	{
		if (g_vec_frame_available[i])
		{
			return true;
		}
	}

	return false;
}

unsigned int GetChannelIdByName(const char * channel_name)
{
	if (g_map_channel.find(channel_name) == g_map_channel.end())
	{
		return -1;
	}
	return g_map_channel[channel_name];
}


void image_merge_reset_background_size()
{
	if (g_vec_frame_available[0])
	{
		worker.SetBackImageSize(g_vec_frame_info[0].width, g_vec_frame_info[0].height);
	}
}

bool  image_merge_set_channel_by_name(const char* channel_name, bool bValid)
{

#ifdef DEBUG_LOG_FILE
	*m_of << "image_merge_set_channel_by_name " << channel_name << " " << bValid <<  std::endl;
#endif
	base::AutoLock al(multi_channel_set_lock);

	//如果bValid为 true, 建立此条通道，否则删除 map 中 channel_name
	if (g_map_channel.find(channel_name) == g_map_channel.end())
	{
		if (bValid)
		{
			int channel = -1;
			if (!GetAvailChannelId(channel))
			{
				return false;
			}

			g_map_channel[channel_name] = channel;
			image_merge_set_channel(channel, bValid);
		}
		else
		{
			return false;
		}
	}
	else
	{
		int channel = g_map_channel[channel_name];
		image_merge_set_channel(channel, bValid);

		if (!bValid)
		{
			std::map<std::string, int>::iterator iter = g_map_channel.find(channel_name);
			g_map_channel.erase(iter);
		}
	}

	return true;
}


bool  image_merge_set_channel(unsigned int channel, bool bValid)
{
#ifdef DEBUG_LOG_FILE
	*m_of << "image_merge_set_channel " << channel << " " << bValid << std::endl;
#endif
	base::AutoLock al(multi_channel_set_lock);
	if (channel > MAX_CHANNELS)
		return false;

	g_vec_frame_available[channel] = bValid;

	return true;

}


bool image_merge_set_image_property_by_name(const char * channel_name, int width, int height)
{

#ifdef DEBUG_LOG_FILE
	*m_of << "image_merge_set_image_property_by_name " << channel_name << " " << width << " " << height << std::endl;
#endif
	base::AutoLock al(multi_channel_set_lock);
	if (g_map_channel.find(channel_name) == g_map_channel.end())
	{
		return false;
	}
	else
	{
		int channel = g_map_channel[channel_name];
		image_merge_set_image_property(channel, width, height);
		return true;
	}
}

bool image_merge_set_image_property(unsigned int channel, int width, int height)
{
#ifdef DEBUG_LOG_FILE
	*m_of << "image_merge_set_image_property " << channel << " " << width << " " << height << std::endl;
#endif
	if (channel > MAX_CHANNELS )
		return false;

	base::AutoLock al(multi_channel_set_lock);
	if (g_vec_frame_available[channel])
	{
		g_vec_frame_info[channel].width = width;
		g_vec_frame_info[channel].height = height;
		//g_vec_frame_info[channel].format = format;

		if (g_vec_frame_data[channel].data)
		{
			delete g_vec_frame_data[channel].data;
		}

		if (width != background_width)
		{
			width = background_width;
		}

		if (height != background_height)
		{
			height = background_height;
		}
			 
		g_vec_frame_data[channel].data = new char[width * height * 3];
		g_vec_frame_data[channel].index = 0;
		g_vec_frame_data[channel].bIsNew = false;

		g_vec_frame_info[channel].size = width * height * 3;

		memset(g_vec_frame_data[channel].data, 0, g_vec_frame_info[channel].size);

		if (channel == 0)
		{
			background_width = width;
			background_height = height;
		}

		return true;
	}

	
	return false;
}

void image_merge_add_image_by_name(const char * channel_name, const char *data, int size, bool isI420orRGB24)
{
	int channel = GetChannelIdByName(channel_name);
	if (channel > -1)
	{
		image_merge_add_image(channel, data, size, isI420orRGB24);
	}
}


void image_merge_add_image(unsigned int channel, const char *data, int size, bool isI420orRGB24)
{
	if ( (channel > MAX_CHANNELS) || (!g_vec_frame_available[channel] ) )
		return;

	base::AutoLock al(img_process_lock);
	//std::cout << g_vec_frame_data[channel].data << std::endl;
	//std::cout << data << std::endl;
	//std::cout << g_vec_frame_info[channel].size << std::endl;

	if (isI420orRGB24)
	{
		memcpy(g_vec_frame_data[channel].data, data, size);
	}
	else
	{
		memcpy(g_vec_frame_data[channel].data, data, g_vec_frame_info[channel].size);
	}

	//convert image 
	int w = g_vec_frame_info[channel].width;
	int h = g_vec_frame_info[channel].height;

	if (isI420orRGB24)
	{
		FfmpegI420ToRGB24(w, h, w, h, g_vec_frame_data[channel].data);
		CopyMemory(g_vec_frame_data[channel].data, (char*)g_out_buffer_format, w * h * 3);
	}

	/*
	CxImage img;
	img.CreateFromArray((uint8_t*)g_vec_frame_data[channel].data, 640, 480, 24, 640 * 3,true);
	img.Save(L"g:\\bak\\temp.jpg",CXIMAGE_FORMAT_JPG);
	*/

	g_vec_frame_data[channel].index++;
	g_vec_frame_data[channel].bIsNew = true;

	//if (channel == 0)
	{
		g_new_image_arrive = true;
	}
}

bool image_merge_get_merged_image(char **image, int * image_len)
{
	return worker.GetOneImage(image, *image_len);
}

void Clear()
{
	for (int i = 1; i < MAX_CHANNELS; i++)
		image_merge_set_channel(i, false);
}

void image_merge_start()
{
	if (worker.IsStop())
	{
		worker.SetBackImageInfo(g_vec_frame_info, g_vec_frame_data, g_vec_frame_available,g_new_image_arrive,img_process_lock);
		worker.Start();
	}
}

void image_merge_stop()
{
	worker.Stop();
	worker.Join();
	Clear();
}

