#pragma once

#include "windows.h"
#include "define.h"


/*Ffmpeg 重采样参数*/
static uint8_t * g_out_buffer;
static AVFrame * g_pFrame;
static AVFrame * g_pFrameRGB;
static struct SwsContext * g_img_convert_ctx;
static int last_width;
static int last_height;
static int last_aim_width;
static int last_aim_height;

static void FfmpegResample(int aim_width, int aim_height, int source_width, int source_height, char *img_data)
{
	bool source_size_changed = last_height != source_height || last_width != source_width;
	bool aim_size_changed = last_aim_height != aim_height || last_aim_width != aim_width;

	if (source_size_changed)
	{
		if (g_pFrame)
		{
			av_free(g_pFrame);
			g_pFrame = NULL;
		}
	}

	if (aim_size_changed)
	{
		if (g_pFrameRGB)
		{
			av_free(g_pFrameRGB);
			g_pFrameRGB = NULL;
		}

		if (g_out_buffer)
		{
			delete[]g_out_buffer;
			g_out_buffer = NULL;
		}
	}

	if (source_size_changed || aim_size_changed)
	{
		if (g_img_convert_ctx)
		{
			sws_freeContext(g_img_convert_ctx);
			g_img_convert_ctx = NULL;
		}
	}

	if (!g_pFrame)
	{
		g_pFrame = av_frame_alloc();
	}

	if (!g_pFrameRGB)
	{
		g_pFrameRGB = av_frame_alloc();
		g_out_buffer = new uint8_t[avpicture_get_size(PIX_FMT_RGB24, aim_width, aim_height)];
		memset(g_out_buffer, 0, avpicture_get_size(PIX_FMT_RGB24, aim_width, aim_height));
		avpicture_fill((AVPicture *)g_pFrameRGB, g_out_buffer, PIX_FMT_RGB24, aim_width, aim_height);
	}

	if (!g_img_convert_ctx)
	{
		g_img_convert_ctx = sws_getContext(source_width, source_height, PIX_FMT_RGB24, aim_width, aim_height,
			PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}
	g_pFrame->data[0] = (unsigned char *)img_data;

	g_pFrame->linesize[0] = 4 * ((source_width * 24 + 31) / 32);
	sws_scale(g_img_convert_ctx, g_pFrame->data, g_pFrame->linesize, 0, source_height, g_pFrameRGB->data, g_pFrameRGB->linesize);


	last_width = source_width;
	last_height = source_height;

	last_aim_width = aim_width;
	last_aim_height = aim_height;

}



/*Ffmpeg 格式转换 */
// i420 -> rgb24 
static uint8_t * g_out_buffer_format;
static AVFrame * g_pFrame_format_I420;
static AVFrame * g_pFrame_format_RGB24;
static struct SwsContext * g_img_convert_ctx_format;
static int last_width_format;
static int last_height_format;
static int last_aim_width_format;
static int last_aim_height_format;


static void FfmpegI420ToRGB24(int aim_width, int aim_height, int source_width, int source_height, char *img_data)
{
	bool source_size_changed = last_height_format != source_height || last_width_format != source_width;
	bool aim_size_changed = last_aim_height_format != aim_height || last_aim_width_format != aim_width;

	if (source_size_changed)
	{

		if (g_pFrame_format_I420)
		{
			av_free(g_pFrame_format_I420);
			g_pFrame_format_I420 = NULL;
		}
	}


	if (aim_size_changed)
	{
		if (g_pFrame_format_RGB24)
		{
			av_free(g_pFrame_format_RGB24);
			g_pFrame_format_RGB24 = NULL;
		}

		if (g_out_buffer_format)
		{
			delete[]g_out_buffer_format;
			g_out_buffer_format = NULL;
		}
	}

	if (source_size_changed || aim_size_changed)
	{
		if (g_img_convert_ctx_format)
		{
			sws_freeContext(g_img_convert_ctx_format);
			g_img_convert_ctx_format = NULL;
		}
	}


	if (!g_pFrame_format_I420)
	{
		g_pFrame_format_I420 = av_frame_alloc();
	}

	if (!g_pFrame_format_RGB24)
	{
		g_pFrame_format_RGB24 = av_frame_alloc();
		g_out_buffer_format = new uint8_t[avpicture_get_size(PIX_FMT_BGR24, aim_width, aim_height)];
		memset(g_out_buffer_format, 0, avpicture_get_size(PIX_FMT_BGR24, aim_width, aim_height));
		avpicture_fill((AVPicture *)g_pFrame_format_RGB24, g_out_buffer_format, PIX_FMT_BGR24, aim_width, aim_height);
	}

	if (!g_img_convert_ctx_format)
	{
		g_img_convert_ctx_format = sws_getContext(source_width, source_height, PIX_FMT_YUV420P, aim_width, aim_height,
			PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	avpicture_fill((AVPicture *)g_pFrame_format_I420, (unsigned char *)img_data, (AVPixelFormat)PIX_FMT_YUV420P, source_width, source_height);
	/*
	g_pFrame_format_I420->data[0] = (unsigned char *)img_data;

	g_pFrame_format_I420->linesize[0] = source_width;
	g_pFrame_format_I420->linesize[1] = source_width / 2;
	g_pFrame_format_I420->linesize[2] = source_width / 2;
	*/
	sws_scale(g_img_convert_ctx_format, g_pFrame_format_I420->data, g_pFrame_format_I420->linesize, 0, source_height, g_pFrame_format_RGB24->data, g_pFrame_format_RGB24->linesize);


	last_width_format = source_width;
	last_height_format = source_height;

	last_aim_width_format = aim_width;
	last_aim_height_format = aim_height;
}
