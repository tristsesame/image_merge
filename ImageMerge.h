/** 
* @ImageMerge模块
* @author trist
* @date 2017/05/16
* @description
* 多路图像合成处理模块
* 注意，此模块只支持rgb24的图像,不支持其它格式。
*/

#ifndef __IMAGE_MERGE__
#define __IMAGE_MERGE__

#include <string>
#include <vector>

#define LIB_IMAGE_MERGE_EXPORTS

#ifdef LIB_IMAGE_MERGE_EXPORTS
#define LIB_IMAGE_MERGE_API __declspec(dllexport)
#else
#define LIB_IMAGE_MERGE_API
#endif

//初始化
extern "C" LIB_IMAGE_MERGE_API void	image_merge_init();

//反初始化
extern "C" LIB_IMAGE_MERGE_API void	image_merge_uninit();

/**
* 重置背景图大小（底图大小）background
* 使用background(channel id 0) 通道的图像大小,将其大小设置为背景大小。
* 用于background中途变化的情况
*/
extern "C" LIB_IMAGE_MERGE_API void image_merge_reset_background_size();

/**
*设置某路图像打开还是关闭.
*param[in] channel 有效值0-8, 0必须有，作为background
*param[in] bValid true表示打开,false为关闭.若设置为false,认为此路通道已关闭
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_channel(unsigned int channel, bool bValid);

/**
*设置某路图像属性
*param[in] channel 有效值0-8
*param[in] width 图像宽度
*param[in] height 图像高度
*param[in] isI420orRGB24 图像格式,isI420orRGB24, 0:RGB24, 1:YUV I420 
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_image_property(unsigned int channel, int width, int height);

/**
*设置某路图像打开还是关闭.
*param[in] channel_name, 可任意设置用字符串表示的名字，内部将自动转换为id. 若无空闲id值（0-8），则此函数设置无效
*                         因为channel必须有0, 对应的name名为"background",所以必须有channel_name为"background"的一路图像存在。
*param[in] bValid true表示打开,false为关闭.若设置为false,认为此路通道已关闭
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_channel_by_name(const char* channel_name, bool bValid);

/**
*设置某路图像属性
*param[in] channel_name, 可任意设置用字符串表示的名字，内部将自动转换为id. 若无空闲id值（0-8），则此函数设置无效
*param[in] width 图像宽度
*param[in] height 图像高度
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_image_property_by_name(const char * channel_name, int width, int height);

/**
*往某路增加图像
*param[in] channel 有效值0-8 若无空闲id值（0-8）则无效
*param[in] data 图像的点阵数据(rgb24 or yuv i420)
#param[in] isI420orRGB24 图像格式, true:yuv i420, false:rgb24
*/
extern "C" LIB_IMAGE_MERGE_API void	image_merge_add_image(unsigned int channel, const char *data, int size, bool isI420orRGB24 = false);

/**
*往某路增加图像
*param[in] channel_name, 可任意设置用字符串表示的名字，内部将自动转换为id. 若无空闲id值（0-8），则此函数设置无效
*param[in] data 图像的点阵数据(rgb24 or yuv i420)
#param[in] isI420orRGB24 图像格式, true:yuv i420, false:rgb24
*/
extern "C" LIB_IMAGE_MERGE_API void	image_merge_add_image_by_name(const char * channel_name, const char *data, int size, bool isI420orRGB24 = false);

/**
*取得当前合并好的图像
*param[in] image 需获取的输出的图像大小,若分配空间不足,则失败.
*param[in] image_len 获取图像值的大小
*/
extern "C" LIB_IMAGE_MERGE_API bool	image_merge_get_merged_image(char **image, int * image_len);

/**
*start work thread
*/
extern "C" LIB_IMAGE_MERGE_API void	image_merge_start();

/**
*stop work thread
*/
extern "C" LIB_IMAGE_MERGE_API void	image_merge_stop();

#endif