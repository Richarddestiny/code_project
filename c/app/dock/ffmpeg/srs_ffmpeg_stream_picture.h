
#ifndef FF_PICTURE_OBJ_H
#define FF_PICTURE_OBJ_H

/*
#ifdef __cplusplus
extern "C" {
#endif
*/


/*****************************************************************************
 函 数 名  : convert_jpg_from_yuv_frame_data
 功能描述  : mmpeg转换
 输入参数  : unsigned char *yuv   输入buff
             const int srcwidth   原始宽
             const int srcheight  原始高
             const int realwidth   输出宽
             const int realheight  输出高
             const char *filename
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
int convert_jpg_from_yuv_frame_data(unsigned char *yuv, const int srcwidth, const int srcheight, const int realwidth, const int realheight, const char *filename);


int convert_bmp_from_yuv_frame_data(unsigned char *yuv, const int srcwidth, const int srcheight, const int realwidth, const int realheight, const char *filename);


/*
#ifdef __cplusplus
}
#endif
*/
#endif /* FF_PICTURE_OBJ_H */
