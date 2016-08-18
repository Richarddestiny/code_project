


#include "srs_ffmpeg_stream_picture.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>  
#include <libavutil/pixfmt.h>



static int save_yuv_data_to_bmp_from_yuvframe(AVFrame *yuvframe, const int width, const int height, const int bpp, const char *filename)
{
	typedef struct tagBITMAPFILEHEADER {
		unsigned short    bfType;
		unsigned long     bfSize;
		unsigned short    bfReserved1;
		unsigned short    bfReserved2;
		unsigned long     bfOffBits;
	} BITMAPFILEHEADER;
	typedef struct tagBITMAPINFOHEADER{
		unsigned long    biSize;
		long             biWidth;
		long             biHeight;
		unsigned short   biPlanes;
		unsigned short   biBitCount;
		unsigned long    biCompression;
		unsigned long    biSizeImage;
		long             biXPelsPerMeter;
		long             biYPelsPerMeter;
		unsigned long    biClrUsed;
		unsigned long    biClrImportant;
	} BITMAPINFOHEADER;


	int flag = 0;
	FILE     *fp = NULL;
	AVFrame  *rgbframe = NULL;
	unsigned char  *rgbbuf = NULL;
	struct SwsContext *rgbSwsCtx = NULL;

	AVFrame  *srcframe = NULL;
	unsigned char  *srcbuf = NULL;
	struct SwsContext *srcSwsCtx = NULL;

	do
	{
		unsigned int srcwidth = yuvframe->width;
		unsigned int srcheight = yuvframe->height;
		unsigned int rgbwidth = width;
		unsigned int rgbheight = height;

		if (NULL == yuvframe){
			break;
		}
		srcframe = av_frame_alloc();
		if (NULL == srcframe){
			break;
		}
		unsigned int src_buf_size = avpicture_get_size(
			AV_PIX_FMT_YUV420P,
			srcwidth,
			srcheight);
		srcbuf = (unsigned char *)av_malloc(src_buf_size);
		if (NULL == srcbuf){
			break;
		}
		avpicture_fill(
			(AVPicture *)srcframe,
			srcbuf,
			AV_PIX_FMT_YUV420P,
			srcwidth,
			srcheight);
		memset(srcbuf, 0, sizeof(char)* src_buf_size);

		unsigned int src_y_size = srcwidth * srcheight;
		srcframe->data[0] = srcbuf;  // 膦嬰槕Y
		srcframe->data[1] = srcbuf + src_y_size;  // U 
		srcframe->data[2] = srcbuf + src_y_size * 5 / 4; // V
		srcframe->width = srcwidth;
		srcframe->height = srcheight;
		srcframe->format = AV_PIX_FMT_YUV420P;
		srcSwsCtx = sws_getContext(
			srcwidth,
			srcheight,
			AV_PIX_FMT_YUV420P,
			srcwidth,
			srcheight,
			AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL);
		if (NULL == srcSwsCtx){
			break;
		}
		int ret = sws_scale(
			srcSwsCtx,
			(const uint8_t* const*)yuvframe->data,
			yuvframe->linesize,
			0,
			srcheight,
			srcframe->data,
			srcframe->linesize
			);
		if (srcheight != ret){
			break;
		}

		rgbframe = av_frame_alloc();
		if (NULL == rgbframe){
			break;
		}
		unsigned int rgbSize = avpicture_get_size(
			AV_PIX_FMT_RGB24,
			rgbwidth,
			rgbheight);
		rgbbuf = (unsigned char *)av_malloc(rgbSize);
		if (NULL == rgbbuf){
			break;
		}
		avpicture_fill(
			(AVPicture *)rgbframe,
			rgbbuf,
			AV_PIX_FMT_RGB24,
			rgbwidth,
			rgbheight);
		memset(rgbbuf, 0, sizeof(char)* rgbSize);

		rgbSwsCtx = sws_getContext(
			srcwidth,
			srcheight,
			AV_PIX_FMT_YUV420P,
			rgbwidth,
			rgbheight,
			PIX_FMT_RGB24,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL);
		if (NULL == rgbSwsCtx){
			break;
		}
		srcframe->data[0] += srcframe->linesize[0] * (srcheight - 1);
		srcframe->linesize[0] *= -1;
		srcframe->data[1] += srcframe->linesize[1] * (srcheight / 2 - 1);
		srcframe->linesize[1] *= -1;
		srcframe->data[2] += srcframe->linesize[2] * (srcheight / 2 - 1);
		srcframe->linesize[2] *= -1;
		ret = sws_scale(
			rgbSwsCtx,
			(const uint8_t* const*)srcframe->data,
			srcframe->linesize,
			0,
			srcheight,
			rgbframe->data,
			rgbframe->linesize
			);
		if (rgbheight == ret){
			/*
			CHANGE_ENDIAN_PIC(
			rgbbuf,
			rgbwidth,
			rgbheight,
			24);
			*/
			BITMAPFILEHEADER bmpheader;
			BITMAPINFOHEADER bmpinfo;
			if ((fp = fopen(filename, "wb+")) == NULL) {
				printf("open file failed!\n");
				break;
			}
			bmpheader.bfType = 0x4d42;
			bmpheader.bfReserved1 = 0;
			bmpheader.bfReserved2 = 0;
			bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
			bmpheader.bfSize = bmpheader.bfOffBits + rgbwidth * rgbheight * bpp / 8;

			bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
			bmpinfo.biWidth = rgbwidth;
			bmpinfo.biHeight = rgbheight;
			bmpinfo.biPlanes = 1;
			bmpinfo.biBitCount = bpp;
			bmpinfo.biCompression = 0L;
			bmpinfo.biSizeImage = (rgbwidth * bpp + 31) / 32 * 4 * rgbheight;
			bmpinfo.biXPelsPerMeter = 100;
			bmpinfo.biYPelsPerMeter = 100;
			bmpinfo.biClrUsed = 0;
			bmpinfo.biClrImportant = 0;
			fwrite(&bmpheader, sizeof(bmpheader), 1, fp);
			fwrite(&bmpinfo, sizeof(bmpinfo), 1, fp);
			//fwrite(rgbframe->data[0], rgbwidth * rgbheight * bpp / 8, 1, fp);
			fwrite(rgbbuf, rgbwidth * rgbheight * bpp / 8, 1, fp);
		}

		flag = 1;

	} while (0);

	if (NULL != fp){
		fclose(fp);
	}
	if (NULL != rgbframe){
		av_frame_free(&rgbframe);
		rgbframe = NULL;
	}
	if (NULL != srcframe){
		av_frame_free(&srcframe);
		srcframe = NULL;
	}
	if (NULL != rgbbuf){
		av_free(rgbbuf);
		rgbbuf = NULL;
	}
	if (NULL != srcbuf){
		av_free(srcbuf);
		srcbuf = NULL;
	}
	if (NULL != rgbSwsCtx){
		sws_freeContext(rgbSwsCtx);
		rgbSwsCtx = NULL;
	}
	if (NULL != srcSwsCtx){
		sws_freeContext(srcSwsCtx);
		srcSwsCtx = NULL;
	}
	return flag;

}

static int save_yuv_data_to_jpg_from_yuvframe(AVFrame *yuvframe, const int width, const int height, const char *filename)
{
	AVFormatContext *av_fmtctx = NULL;
	AVStream        *av_stream = NULL;
	AVCodecContext  *av_codec_ctx = NULL;
	AVCodec         *av_codec = NULL;

	AVFrame *enc_frame = NULL;
	unsigned char *enc_buf = NULL;
	struct SwsContext *enc_sws_ctx = NULL;

	int flag = 0;
	do
	{
		if (NULL == yuvframe){
			break;
		}
		unsigned int src_width = yuvframe->width;
		unsigned int src_height = yuvframe->height;
		unsigned int enc_width = width;
		unsigned int enc_height = height;

		enc_frame = av_frame_alloc();
		if (NULL == enc_frame){
			break;
		}
		unsigned int enc_buf_size = avpicture_get_size(AV_PIX_FMT_YUV420P, enc_width, enc_height);
		enc_buf = (unsigned char *)av_malloc(enc_buf_size);
		if (NULL == enc_buf){
			break;
		}
		avpicture_fill(
			(AVPicture *)enc_frame,
			enc_buf,
			AV_PIX_FMT_YUV420P,
			enc_width,
			enc_height
			);
		memset(enc_buf, 0, sizeof(char)* enc_buf_size);
		unsigned int src_y_size = enc_width * enc_height;
		enc_frame->data[0] = enc_buf;  // 膦嬰槕Y
		enc_frame->data[1] = enc_buf + src_y_size;  // U 
		enc_frame->data[2] = enc_buf + src_y_size * 5 / 4; // V
		enc_frame->width = enc_width;
		enc_frame->height = enc_height;
		enc_frame->format = AV_PIX_FMT_YUV420P;
		enc_sws_ctx = sws_getContext(
			src_width,
			src_height,
			AV_PIX_FMT_YUV420P,
			enc_width,
			enc_height,
			AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL);
		if (NULL == enc_sws_ctx){
			break;
		}
		int ret = sws_scale(
			enc_sws_ctx,
			(const uint8_t* const*)yuvframe->data,
			yuvframe->linesize,
			0,
			src_height,
			enc_frame->data,
			enc_frame->linesize
			);
		if (enc_height != ret){
			break;
		}

		av_fmtctx = avformat_alloc_context();
		if (NULL == av_fmtctx){
			break;
		}
		av_fmtctx->oformat = av_guess_format("mjpeg", NULL, NULL);
		if (avio_open(&av_fmtctx->pb, filename, AVIO_FLAG_READ_WRITE) < 0) {
			break;
		}
		av_stream = avformat_new_stream(av_fmtctx, 0);
		if (av_stream == NULL) {
			break;
		}
		av_codec_ctx = av_stream->codec;
		av_codec_ctx->codec_id = av_fmtctx->oformat->video_codec;
		av_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
		av_codec_ctx->pix_fmt = PIX_FMT_YUVJ420P;
		av_codec_ctx->width = enc_width;
		av_codec_ctx->height = enc_height;
		av_codec_ctx->time_base.num = 1;
		av_codec_ctx->time_base.den = 25;
		av_dump_format(av_fmtctx, 0, filename, 1);
		av_codec = avcodec_find_encoder(av_codec_ctx->codec_id);
		if (!av_codec) {
			break;
		}
		if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
			break;
		}
		ret = avformat_write_header(av_fmtctx, NULL);
		if (ret < 0) {
			break;
		}
		int y_size = av_codec_ctx->width * av_codec_ctx->height;
		AVPacket pkt;
		av_new_packet(&pkt, y_size * 3);
		int got_picture = 0;
		ret = avcodec_encode_video2(av_codec_ctx, &pkt, enc_frame, &got_picture);
		if (ret < 0) {
			break;
		}
		if (got_picture == 1) {
			ret = av_write_frame(av_fmtctx, &pkt);
		}
		av_free_packet(&pkt);
		av_write_trailer(av_fmtctx);

	} while (0);

	if (NULL != enc_frame){
		av_frame_free(&enc_frame);
	}
	if (NULL != enc_buf){
		av_free(enc_buf);
	}
	if (NULL != av_stream) {
		avcodec_close(av_stream->codec);
	}
	if (NULL != av_fmtctx->pb){
		avio_close(av_fmtctx->pb);
	}
	if (NULL != av_fmtctx){
		avformat_free_context(av_fmtctx);
	}
	if (NULL != enc_sws_ctx){
		sws_freeContext(enc_sws_ctx);
		enc_sws_ctx = NULL;
	}
	return  flag;


}


/*****************************************************************************
 函 数 名  : convert_jpg_from_yuv_frame_data
 功能描述  : mmpeg转换
 输入参数  : unsigned char *yuv   输入buff
             const int srcwidth   原始宽
             const int srcheight  原始高
             const int realwidth   转换后宽
             const int realheight  转换后高
             const char *filename  转换后的文件的名称和路径
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
int convert_jpg_from_yuv_frame_data(unsigned char *yuv, const int srcwidth, const int srcheight, const int realwidth, const int realheight, const char *filename)
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();


	AVFrame *enc_frame = NULL;
	unsigned char *enc_buf = NULL;
	int flag = 0;
	do
	{
		int  enc_width = srcwidth;
		int  enc_height = srcheight;
		enc_frame = av_frame_alloc();
		if (NULL == enc_frame){
			break;
		}
		unsigned int enc_buf_size = avpicture_get_size(AV_PIX_FMT_YUV420P, enc_width, enc_height);
		enc_buf = (unsigned char *)av_malloc(enc_buf_size);
		if (NULL == enc_buf){
			break;
		}
		avpicture_fill(
			(AVPicture *)enc_frame,
			enc_buf,
			AV_PIX_FMT_YUV420P,
			enc_width,
			enc_height
			);
		memset(enc_buf, 0, sizeof(char)* enc_buf_size);
		unsigned int src_y_size = enc_width * enc_height;
		enc_frame->data[0] = enc_buf;  // 膦嬰槕Y
		enc_frame->data[1] = enc_buf + src_y_size;  // U 
		enc_frame->data[2] = enc_buf + src_y_size * 5 / 4; // V
		enc_frame->width = enc_width;
		enc_frame->height = enc_height;
		enc_frame->format = AV_PIX_FMT_YUV420P;
		memcpy(enc_buf, yuv, enc_buf_size);

		flag = save_yuv_data_to_jpg_from_yuvframe(enc_frame, realwidth, realheight, filename);

	} while (0);

	if (NULL != enc_frame){
		av_frame_free(&enc_frame);
	}
	if (NULL != enc_buf){
		av_free(enc_buf);
	}

	return flag;


}


int convert_bmp_from_yuv_frame_data(unsigned char *yuv, const int srcwidth, const int srcheight, const int realwidth, const int realheight, const char *filename)
{
	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	AVFrame *enc_frame = NULL;
	unsigned char *enc_buf = NULL;
	int flag = 0;
	do
	{
		int  enc_width = srcwidth;
		int  enc_height = srcheight;
		enc_frame = av_frame_alloc();
		if (NULL == enc_frame){
			break;
		}
		unsigned int enc_buf_size = avpicture_get_size(AV_PIX_FMT_YUV420P, enc_width, enc_height);
		enc_buf = (unsigned char *)av_malloc(enc_buf_size);
		if (NULL == enc_buf){
			break;
		}
		avpicture_fill(
			(AVPicture *)enc_frame,
			enc_buf,
			AV_PIX_FMT_YUV420P,
			enc_width,
			enc_height
			);
		memset(enc_buf, 0, sizeof(char)* enc_buf_size);
		unsigned int src_y_size = enc_width * enc_height;
		enc_frame->data[0] = enc_buf;  // 膦嬰槕Y
		enc_frame->data[1] = enc_buf + src_y_size;  // U 
		enc_frame->data[2] = enc_buf + src_y_size * 5 / 4; // V
		enc_frame->width = enc_width;
		enc_frame->height = enc_height;
		enc_frame->format = AV_PIX_FMT_YUV420P;
		memcpy(enc_buf, yuv, enc_buf_size);

		flag = save_yuv_data_to_bmp_from_yuvframe(enc_frame, realwidth, realheight,24, filename);

	} while (0);

	if (NULL != enc_frame){
		av_frame_free(&enc_frame);
	}
	if (NULL != enc_buf){
		av_free(enc_buf);
	}

	return flag;


}
