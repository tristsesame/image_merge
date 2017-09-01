/** 
* @ImageMergeģ��
* @author trist
* @date 2017/05/16
* @description
* ��·ͼ��ϳɴ���ģ��
* ע�⣬��ģ��ֻ֧��rgb24��ͼ��,��֧��������ʽ��
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

//��ʼ��
extern "C" LIB_IMAGE_MERGE_API void	image_merge_init();

//����ʼ��
extern "C" LIB_IMAGE_MERGE_API void	image_merge_uninit();

/**
* ���ñ���ͼ��С����ͼ��С��background
* ʹ��background(channel id 0) ͨ����ͼ���С,�����С����Ϊ������С��
* ����background��;�仯�����
*/
extern "C" LIB_IMAGE_MERGE_API void image_merge_reset_background_size();

/**
*����ĳ·ͼ��򿪻��ǹر�.
*param[in] channel ��Чֵ0-8, 0�����У���Ϊbackground
*param[in] bValid true��ʾ��,falseΪ�ر�.������Ϊfalse,��Ϊ��·ͨ���ѹر�
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_channel(unsigned int channel, bool bValid);

/**
*����ĳ·ͼ������
*param[in] channel ��Чֵ0-8
*param[in] width ͼ����
*param[in] height ͼ��߶�
*param[in] isI420orRGB24 ͼ���ʽ,isI420orRGB24, 0:RGB24, 1:YUV I420 
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_image_property(unsigned int channel, int width, int height);

/**
*����ĳ·ͼ��򿪻��ǹر�.
*param[in] channel_name, �������������ַ�����ʾ�����֣��ڲ����Զ�ת��Ϊid. ���޿���idֵ��0-8������˺���������Ч
*                         ��Ϊchannel������0, ��Ӧ��name��Ϊ"background",���Ա�����channel_nameΪ"background"��һ·ͼ����ڡ�
*param[in] bValid true��ʾ��,falseΪ�ر�.������Ϊfalse,��Ϊ��·ͨ���ѹر�
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_channel_by_name(const char* channel_name, bool bValid);

/**
*����ĳ·ͼ������
*param[in] channel_name, �������������ַ�����ʾ�����֣��ڲ����Զ�ת��Ϊid. ���޿���idֵ��0-8������˺���������Ч
*param[in] width ͼ����
*param[in] height ͼ��߶�
*/
extern "C" LIB_IMAGE_MERGE_API bool    image_merge_set_image_property_by_name(const char * channel_name, int width, int height);

/**
*��ĳ·����ͼ��
*param[in] channel ��Чֵ0-8 ���޿���idֵ��0-8������Ч
*param[in] data ͼ��ĵ�������(rgb24 or yuv i420)
#param[in] isI420orRGB24 ͼ���ʽ, true:yuv i420, false:rgb24
*/
extern "C" LIB_IMAGE_MERGE_API void	image_merge_add_image(unsigned int channel, const char *data, int size, bool isI420orRGB24 = false);

/**
*��ĳ·����ͼ��
*param[in] channel_name, �������������ַ�����ʾ�����֣��ڲ����Զ�ת��Ϊid. ���޿���idֵ��0-8������˺���������Ч
*param[in] data ͼ��ĵ�������(rgb24 or yuv i420)
#param[in] isI420orRGB24 ͼ���ʽ, true:yuv i420, false:rgb24
*/
extern "C" LIB_IMAGE_MERGE_API void	image_merge_add_image_by_name(const char * channel_name, const char *data, int size, bool isI420orRGB24 = false);

/**
*ȡ�õ�ǰ�ϲ��õ�ͼ��
*param[in] image ���ȡ�������ͼ���С,������ռ䲻��,��ʧ��.
*param[in] image_len ��ȡͼ��ֵ�Ĵ�С
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