
#ifndef __IMAGE_PROCESS_THREAD__H_
#define __IMAGE_PROCESS_THREAD__H_

#include "define.h"
#include "base\SimpleThread.h"
#include <map>
#include <list>

#include <GdiPlus.h>
#pragma comment(lib, "Gdiplus.lib")

class CImageProcessThread : public base::SimpleThread {
public:
	CImageProcessThread();
	void SetBackImageInfo(std::vector<FrameInfo>& frame_info,
	std::vector<FrameData>& frame_data,
	std::vector<bool>& frame_available,
	bool& image_arrive,
	base::Lock &img_process_lock);

	void SetBackImageSize(int width, int height);

	~CImageProcessThread();

	virtual void Run();

	bool GetOneImage( char ** image, int & image_len);

	void AddOneImage(int channel, char * buf, int buf_len);

private:

	void MergeImage(int channel_1);
	void MergeImage(int channel_1, int channel_2);
	void MergeImage(int channel_1, int channel_2, int channel_3);

private:
	int background_width;
	int background_height;

	int background_width_set;
	int background_height_set;

	char * result_image;

	std::vector<FrameInfo>* p_vec_frame_info;
	std::vector<FrameData>* p_vec_frame_data;
	std::vector<bool>* p_vec_frame_available;

	//用于存放需转化后的大小
	std::vector<FrameInfo> vec_frame_resample_info;

	bool* p_new_image_arrive;
	base::Lock* p_img_process_lock;


	//上一次写入ipc的的timestamp
	unsigned int last_write_ipc_timestamp;
	//时间间隔，默认为15fps左右,定为50毫秒
	int write_ipc_timestamp_interval;
};

#endif	// __IMAGE_PROCESS_THREAD__H_
