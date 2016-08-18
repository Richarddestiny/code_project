/*
 * packed and tested by Peter.Xu @ 2009.8
 */
#ifndef		__LM_RGB2YUV_H__
#define		__LM_RGB2YUV_H__

#ifdef		__cplusplus
extern "C" {
#endif

void rgb2yuv_convert(unsigned char *YUV, unsigned char *RGB_RAW, 
					 unsigned int width, unsigned int height);

#ifdef		__cplusplus
}
#endif

#endif
