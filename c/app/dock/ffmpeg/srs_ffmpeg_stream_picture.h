
#ifndef FF_PICTURE_OBJ_H
#define FF_PICTURE_OBJ_H

/*
#ifdef __cplusplus
extern "C" {
#endif
*/


/*****************************************************************************
 �� �� ��  : convert_jpg_from_yuv_frame_data
 ��������  : mmpegת��
 �������  : unsigned char *yuv   ����buff
             const int srcwidth   ԭʼ��
             const int srcheight  ԭʼ��
             const int realwidth   �����
             const int realheight  �����
             const char *filename
 �������  : ��
 �� �� ֵ  :
*****************************************************************************/
int convert_jpg_from_yuv_frame_data(unsigned char *yuv, const int srcwidth, const int srcheight, const int realwidth, const int realheight, const char *filename);


int convert_bmp_from_yuv_frame_data(unsigned char *yuv, const int srcwidth, const int srcheight, const int realwidth, const int realheight, const char *filename);


/*
#ifdef __cplusplus
}
#endif
*/
#endif /* FF_PICTURE_OBJ_H */
