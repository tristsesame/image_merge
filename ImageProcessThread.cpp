
#include "define.h"
#include "ImageProcessThread.h"
#include <algorithm>
#include <iostream>
#include "ximage.h"
#include <MMSystem.h>
#include "FormatConvert.c"

CImageProcessThread::CImageProcessThread() {
	background_width = 0;
	background_height = 0;

	background_width_set = 0;
	background_height_set = 0;

	last_write_ipc_timestamp = 0;
	write_ipc_timestamp_interval = 50;

	result_image = nullptr;
	p_vec_frame_info = nullptr;
	p_vec_frame_data = nullptr;
	p_vec_frame_available = nullptr;

	p_img_process_lock = nullptr;


	FrameInfo tempInfo;
	tempInfo.width = 0;
	tempInfo.height = 0;
	tempInfo.size = 0;
	tempInfo.format = 0;

	for (int i = 0; i < MAX_CHANNELS; i++)
	{
		vec_frame_resample_info.push_back(tempInfo);
	}
}


void CImageProcessThread::SetBackImageInfo(std::vector<FrameInfo>& frame_info,
	std::vector<FrameData>& frame_data,
	std::vector<bool>& frame_available,
	bool& image_arrive,
	base::Lock &img_process_lock) {

	if (false == IsStop())
		return;

	p_vec_frame_info = &frame_info;
	p_vec_frame_data = &frame_data;
	p_vec_frame_available = &frame_available;

	background_width = (*p_vec_frame_info)[0].width;
	background_height = (*p_vec_frame_info)[0].height;

	if (!result_image)
		delete result_image;

	result_image = new char[background_width * background_height * 3];

	p_new_image_arrive = &image_arrive;

	p_img_process_lock = &img_process_lock;
}

void CImageProcessThread::SetBackImageSize(int width, int height)
{
	background_width_set = width;
	background_height_set = height;
}

CImageProcessThread::~CImageProcessThread() {
	if (!result_image)
		delete result_image;
	result_image = nullptr;
}


bool CImageProcessThread::GetOneImage(char ** image, int & image_len) 
{
	int width = background_width;
	int height = background_height;

	image_len = width * height * 3;
	*image = result_image;

	return true;
}


void CImageProcessThread::MergeImage(int channel_1)
{
	base::AutoLock al(*p_img_process_lock);
	//认为二者图像格式大小相同，均为rgb24
	char *image0 = (*p_vec_frame_data)[0].data;
	int image0_width = background_width;
	int image0_height = background_height;


	char *image1 = (*p_vec_frame_data)[channel_1].data;
	int index = (*p_vec_frame_data)[channel_1].index;
	int image1_width = (*p_vec_frame_info)[channel_1].width;
	int image1_height = (*p_vec_frame_info)[channel_1].height;

	int half_w = image0_width / 2;
	int quarter_w = image0_width / 4;
	int perBytesLine = image0_width * 3;

	//resample image 1
	if ((*p_vec_frame_data)[channel_1].bIsNew)
	{
		FfmpegResample(image0_width, image0_height, (*p_vec_frame_info)[channel_1].width, (*p_vec_frame_info)[channel_1].height, image1);
		CopyMemory(image1, (char*)g_pFrameRGB->data[0], image0_width * image0_height * 3);
		(*p_vec_frame_data)[channel_1].bIsNew = false;
	}

	for (int i = 0; i < image0_height; i++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (quarter_w * 3 + perBytesLine * i), half_w * 3);
		memcpy((char*)result_image + (half_w * 3 + perBytesLine * i), (char*)image1 + (quarter_w * 3 + perBytesLine * i), half_w * 3);

		//memcpy((char*)result_image + (perBytesLine * i), (char*)image1 + (perBytesLine * i), perBytesLine);
	}

	//std::cout << "image index : " << index << std::endl;

	/*
	CxImage img;
	wchar_t buffer_file_name[64];
	wsprintf(buffer_file_name, L"G:\\PrivateFile\\img\\%d.jpg", index);
	img.CreateFromArray((unsigned char *)result_image, background_width, background_height, 24, perBytesLine, false);
	img.Save(buffer_file_name, CXIMAGE_FORMAT_JPG);
	*/
}

void CImageProcessThread::MergeImage(int channel_1, int channel_2)
{
	base::AutoLock al(*p_img_process_lock);

	//std::cout << background_width << " & " << background_height << std::endl;
	//需计算大小，并进行resample
	int h = background_height / 2;
	//round 4
	h = (h / 4) * 4;
	int w = h * 4 / 3;
	w = (w / 4) * 4;

	int x = background_width - w;
	int y = (background_height - h * 2) / 2;


	//resample image 1
	char *image1_data = (*p_vec_frame_data)[channel_1].data;
	if ((*p_vec_frame_data)[channel_1].bIsNew)
	{
		FfmpegResample(w, h, (*p_vec_frame_info)[channel_1].width, (*p_vec_frame_info)[channel_1].height, image1_data);
		CopyMemory(image1_data, (char*)g_pFrameRGB->data[0], w * h * 3);
		(*p_vec_frame_data)[channel_1].bIsNew = false;

	}

	//resample image 2
	char *image2_data = (*p_vec_frame_data)[channel_2].data;
	if ((*p_vec_frame_data)[channel_2].bIsNew)
	{
		FfmpegResample(w, h, (*p_vec_frame_info)[channel_2].width, (*p_vec_frame_info)[channel_2].height, image2_data);
		CopyMemory(image2_data, (char*)g_pFrameRGB->data[0], w * h * 3);
		(*p_vec_frame_data)[channel_2].bIsNew = false;
	}
	int image_perBytesLine = w * 3;


	char *image0 = (*p_vec_frame_data)[0].data;
	int image0_width = background_width;
	int image0_height = background_height;

	int image0_begin_pos = w / 2;
	int image0_bytes_per_line = (image0_width - w) * 3;
	int perBytesLine = image0_width * 3; 

	int i = 0;
	for (; i < y; i++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
	}

	//merge image 1, pos: (x,y)-(x+w,y+h)
	for (int j = 0; i < y + h; i++,j++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
		memcpy((char*)result_image + (image0_bytes_per_line + perBytesLine * i), (char*)image1_data + (image_perBytesLine*j ), w * 3);
	}


	for (; i < background_height - h; i++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
	}

	for (int j = 0; i < background_height; i++,j++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
		memcpy((char*)result_image + (image0_bytes_per_line + perBytesLine * i), (char*)image2_data + (image_perBytesLine*j), w * 3);
	}

	return;
}

void CImageProcessThread::MergeImage(int channel_1, int channel_2, int channel_3)
{
	base::AutoLock al(*p_img_process_lock);
	//计算大小
	//需计算大小，并进行resample
	int h = background_height / 3;
	//round 4
	h = (h / 4) * 4;
	int w = h * 4 / 3;
	w = (w / 4) * 4;

	int x = background_width - w;
	int y = (background_height - h * 3) / 2;


	//resample image 1
	char *image1_data = (*p_vec_frame_data)[channel_1].data;
	if ((*p_vec_frame_data)[channel_1].bIsNew)
	{
		FfmpegResample(w, h, (*p_vec_frame_info)[channel_1].width, (*p_vec_frame_info)[channel_1].height, image1_data);
		CopyMemory(image1_data, (char*)g_pFrameRGB->data[0], w * h * 3);
		(*p_vec_frame_data)[channel_1].bIsNew = false;
	}

	//resample image 2
	char *image2_data = (*p_vec_frame_data)[channel_2].data;
	if ((*p_vec_frame_data)[channel_2].bIsNew)
	{
		FfmpegResample(w, h, (*p_vec_frame_info)[channel_2].width, (*p_vec_frame_info)[channel_2].height, image2_data);
		CopyMemory(image2_data, (char*)g_pFrameRGB->data[0], w * h * 3);
		(*p_vec_frame_data)[channel_2].bIsNew = false;
	}

	//resample image 3
	char *image3_data = (*p_vec_frame_data)[channel_3].data;
	if ((*p_vec_frame_data)[channel_3].bIsNew)
	{
		FfmpegResample(w, h, (*p_vec_frame_info)[channel_3].width, (*p_vec_frame_info)[channel_3].height, image3_data);
		CopyMemory(image3_data, (char*)g_pFrameRGB->data[0], w * h * 3);
		(*p_vec_frame_data)[channel_3].bIsNew = false;
	}

	int image_perBytesLine = w * 3;

	char *image0 = (*p_vec_frame_data)[0].data;
	int image0_width = background_width;
	int image0_height = background_height;

	int image0_begin_pos = w / 2;
	int image0_bytes_per_line = (image0_width - w) * 3;
	int perBytesLine = image0_width * 3;

	int i = 0;
	for (; i < y; i++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
	}

	//merge image 1, pos: (x,y)-(x+w,y+h)
	for (int j = 0; i < y + h; i++, j++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
		memcpy((char*)result_image + (image0_bytes_per_line + perBytesLine * i), (char*)image1_data + (image_perBytesLine*j), w * 3);
	}


	for (; i < background_height - h * 2; i++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
	}

	for (int j = 0; i < background_height - h; i++, j++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
		memcpy((char*)result_image + (image0_bytes_per_line + perBytesLine * i), (char*)image2_data + (image_perBytesLine*j), w * 3);
	}

	for (int j = 0; i < background_height; i++, j++)
	{
		memcpy((char*)result_image + (perBytesLine * i), (char*)image0 + (image0_begin_pos * 3 + perBytesLine * i), image0_bytes_per_line);
		memcpy((char*)result_image + (image0_bytes_per_line + perBytesLine * i), (char*)image3_data + (image_perBytesLine*j), w * 3);
	}

	return;
}


void CImageProcessThread::Run() {
	int bpp = 0;
	int image_len = 0;
	

	int buf_len = background_width*background_height * 3;
	char *back_img = new char[buf_len];

	CxImage img;
	int bytesPerLine = background_width* 3;
	int k = 0;
	//wchar_t buffer_file_name[256];
    while (false == IsStop()) {
		bpp = 24;
		image_len = buf_len;
		int debug_begin_time = ::timeGetTime();
		
		if (!(*p_vec_frame_available)[0] || !(*p_new_image_arrive) )
		{
			Sleep(20);
			continue;
		}

		int nCount = 0;
		int image_index[3];

		for (int i = 1; i< MAX_CHANNELS; ++i) {

			if (! (*p_vec_frame_available)[i])
				continue;

			image_index[nCount] = i;
			nCount++;
		}

		//对一张图暂不处理
		if (nCount < 1 )
		{
			Sleep(30);
			//std::cout << "sleep 30 " << std::endl;
			continue;
		}

		if (background_height != background_height_set || background_width != background_width_set)
		{
			if (!result_image)
				delete result_image;

			background_height = background_height_set;
			background_width = background_width_set;

			result_image = new char[background_width * background_height * 3];
		}
         
		if (nCount == 0 )
		{ 
			CopyMemory(result_image, (*p_vec_frame_data)[0].data, background_height * background_width * 3);
			//std::cout << "one image" << std::endl;
		}
		else if (nCount == 1)
		{
			MergeImage(image_index[0]);
		}
		else if (nCount == 2)
		{
			MergeImage(image_index[0], image_index[1]);
		}
		else
		{
			MergeImage(image_index[0], image_index[1], image_index[2]);
		}


		(*p_new_image_arrive) = false;

		
		/*
		k++;
		wsprintf(buffer_file_name, L"G:\\PrivateFile\\img\\%d.jpg", k);
		img.CreateFromArray((unsigned char *)result_image, background_width, background_height, 24, bytesPerLine, false);
		img.Save(buffer_file_name, CXIMAGE_FORMAT_JPG);
		*/

		//调节fps
		unsigned int time = ::timeGetTime();
		int elapse_time = time - last_write_ipc_timestamp;

		if (elapse_time > write_ipc_timestamp_interval )
		{

			last_write_ipc_timestamp = ::timeGetTime();
		}
		else
		{
			unsigned int sleep_time = write_ipc_timestamp_interval - elapse_time;
			if (sleep_time > 15 && sleep_time < 50 )
			{
				//std::cout << "sleep time : " << sleep_time <<  std::endl;
				Sleep(sleep_time);
			}
		}

	}

	delete [] back_img;

}

