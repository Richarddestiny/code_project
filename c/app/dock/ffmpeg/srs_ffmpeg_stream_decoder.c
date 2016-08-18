
#include "srs_ffmpeg_stream_decoder.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>  
#include <libavutil/pixfmt.h>


#define  IMG_FILE_NAME  512
#define  IMG_FILE_SIZE  100

typedef struct tag_image_store{
	unsigned int srs_img_type  ;
	unsigned int srs_img_width ;
	unsigned int srs_img_height;
	char srs_img_path_name[IMG_FILE_NAME];
}srs_image_store;


//interface for decoder
typedef struct tag_ffmpeg_stream_decoder{

	AVFormatContext   *srs_src_fctx;
	AVCodecContext    *srs_src_cdctx;
	AVCodec           *srs_src_codec;
	struct SwsContext *srs_src_sws_ctx;
	AVPacket          srs_src_pkt;
	int srs_src_video_index;
	int srs_src_audio_index;
	int srs_src_fps;
	unsigned char  *srs_src_buf;
	unsigned char  *srs_dec_buf;
	AVFrame  *srs_src_frame;
	AVFrame  *srs_dec_frame;
	unsigned int srs_src_width;
	unsigned int srs_src_height;
	unsigned int srs_dec_width;
	unsigned int srs_dec_height;
	srs_image_store srs_img_store[IMG_FILE_SIZE];
	char srs_src_filename[1024];

}srs_ffmpeg_stream_decoder;



static int save_yuv_data_to_bmp_from_yuvframe(AVFrame *yuvframe, const int width, const int height, const int bpp, const char *filename);
static int save_yuv_data_to_jpg_from_yuvframe(AVFrame *yuvframe, const int width, const int height, const char *filename);

static void CHANGE_ENDIAN_24(unsigned char *data);
static void CHANGE_ENDIAN_32(unsigned char *data);
static void CHANGE_ENDIAN_32(unsigned char *data);
static void CHANGE_ENDIAN_PIC(unsigned char *image, int w, int h, int bpp);




ff_decoder_obj  *ff_decoder_obj_create()
{
	srs_ffmpeg_stream_decoder *obj = (srs_ffmpeg_stream_decoder *)malloc(sizeof(srs_ffmpeg_stream_decoder));
	memset(obj, 0, sizeof(srs_ffmpeg_stream_decoder));

	return obj;

}

ff_decoder_obj  *ff_decoder_dec_create(ff_decoder_obj  *obj)
{
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder *)obj;

	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	int flag = 0;
	do{
		if (NULL == this_obj){
			printf("this_obj struct is null. \n");
			break;
		}
		this_obj->srs_src_fctx = avformat_alloc_context();
		if (NULL == this_obj->srs_src_fctx){
			printf("couldn't create AVFormatContext. \n");
			break;
		} 
		if (avformat_open_input(&this_obj->srs_src_fctx, this_obj->srs_src_filename, NULL, NULL) != 0){
			printf("couldn't open input stream.\n");
			break;
		}
		if (avformat_find_stream_info(this_obj->srs_src_fctx, NULL) < 0){
			printf("couldn't find stream information.\n");
			break;
		}
	
		this_obj->srs_src_video_index = -1;
		unsigned int i = 0;
		for (i = 0; i < this_obj->srs_src_fctx->nb_streams; i++){
			if (this_obj->srs_src_fctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
				this_obj->srs_src_video_index = i;
				break;
			}
		}
		if (this_obj->srs_src_video_index < 0){
			break;
		}
		av_dump_format(this_obj->srs_src_fctx, 0, this_obj->srs_src_filename, 0);

		this_obj->srs_src_cdctx = this_obj->srs_src_fctx->streams[this_obj->srs_src_video_index]->codec;
		this_obj->srs_src_codec = avcodec_find_decoder(this_obj->srs_src_cdctx->codec_id);
		if (NULL == this_obj->srs_src_codec){
			printf("codec not found.\n");
			break;
		}
		if (avcodec_open2(this_obj->srs_src_cdctx, this_obj->srs_src_codec, NULL) < 0){
			printf("could not open codec.\n");
			break;
		}
		this_obj->srs_src_width = this_obj->srs_src_cdctx->width;
		this_obj->srs_src_height = this_obj->srs_src_cdctx->height;
		if (0 == this_obj->srs_dec_width){
			this_obj->srs_dec_width = this_obj->srs_src_width;
		}
		if (0 == this_obj->srs_dec_height){
			this_obj->srs_dec_height = this_obj->srs_src_height;
		}

		int srs_src_r_frame_rate_den = this_obj->srs_src_fctx->streams[this_obj->srs_src_video_index]->r_frame_rate.den;
		int srs_src_r_frame_rate_num = this_obj->srs_src_fctx->streams[this_obj->srs_src_video_index]->r_frame_rate.num;
		this_obj->srs_src_fps = srs_src_r_frame_rate_num / srs_src_r_frame_rate_den;

		this_obj->srs_src_frame = av_frame_alloc();
		if (NULL == this_obj->srs_src_frame){
			printf("could not create yuvframe.\n");
			break;
		}
		unsigned int srs_src_buf_size = avpicture_get_size(
			AV_PIX_FMT_YUV420P,
			this_obj->srs_src_cdctx->width,
			this_obj->srs_src_cdctx->height
			);
		this_obj->srs_src_buf = (unsigned char *)av_malloc(srs_src_buf_size);
		if (NULL == this_obj->srs_src_buf){
			printf("could not create yuvbuf.\n");
			break;
		}
		avpicture_fill(
			(AVPicture *)this_obj->srs_src_frame,
			this_obj->srs_src_buf,
			this_obj->srs_src_cdctx->pix_fmt,
			this_obj->srs_src_cdctx->width,
			this_obj->srs_src_cdctx->height
			);
		this_obj->srs_src_frame->width = this_obj->srs_src_width;
		this_obj->srs_src_frame->height = this_obj->srs_src_height;
		this_obj->srs_src_frame->format = this_obj->srs_src_cdctx->pix_fmt;


		this_obj->srs_dec_frame = av_frame_alloc();
		if (NULL == this_obj->srs_dec_frame){
			printf("could not create realframe.\n");
			break;
		}
		unsigned int srs_dec_buf_size = 
			avpicture_get_size(AV_PIX_FMT_YUV420P,
			this_obj->srs_dec_width,
			this_obj->srs_dec_height
			);
		this_obj->srs_dec_buf = (unsigned char *)av_malloc(srs_dec_buf_size);
		if (NULL == this_obj->srs_dec_buf){
			printf("could not create realbuf.\n");
			break;
		}
		avpicture_fill(
			(AVPicture *)this_obj->srs_dec_frame,
			this_obj->srs_dec_buf,
			AV_PIX_FMT_YUV420P,
			this_obj->srs_dec_width,
			this_obj->srs_dec_height
			);
		unsigned int srs_dec_y_size = this_obj->srs_dec_width * this_obj->srs_dec_height;
		this_obj->srs_dec_frame->data[0] = this_obj->srs_dec_buf;  // 좋똑Y
		this_obj->srs_dec_frame->data[1] = this_obj->srs_dec_buf + srs_dec_y_size;  // U 
		this_obj->srs_dec_frame->data[2] = this_obj->srs_dec_buf + srs_dec_y_size * 5 / 4; // V
		this_obj->srs_dec_frame->width   = this_obj->srs_dec_width;
		this_obj->srs_dec_frame->height  = this_obj->srs_dec_height;
		this_obj->srs_dec_frame->format  = AV_PIX_FMT_YUV420P;

		//begin
		this_obj->srs_src_sws_ctx = sws_getContext(
			this_obj->srs_src_width,
			this_obj->srs_src_height,
			this_obj->srs_src_cdctx->pix_fmt,
			this_obj->srs_dec_width,
			this_obj->srs_dec_height,
			AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL);
		if (NULL == this_obj->srs_src_sws_ctx){
			printf("could not create img_convert_ctx.\n");
			break;
		}
		flag = 1;

	} while (0);

	if (flag){
		return obj;
	}
	else{
		return NULL;
	}


}


ff_decoder_obj  *ff_decoder_obj_destory(ff_decoder_obj *obj)
{
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder *)obj;
	do
	{
		if (NULL != this_obj->srs_src_frame){
			av_frame_free(&this_obj->srs_src_frame);
			this_obj->srs_src_frame = NULL;
		}
		if (NULL != this_obj->srs_src_buf){
			av_free(this_obj->srs_src_buf);
			this_obj->srs_src_buf = NULL;
		}
		if (NULL != this_obj->srs_dec_frame){
			av_frame_free(&this_obj->srs_dec_frame);
			this_obj->srs_dec_frame = NULL;
		}
		if (NULL != this_obj->srs_dec_buf){
			av_free(this_obj->srs_dec_buf);
			this_obj->srs_dec_buf = NULL;
		}
		if (NULL != this_obj->srs_src_sws_ctx){
			sws_freeContext(this_obj->srs_src_sws_ctx);
			this_obj->srs_src_sws_ctx = NULL;
		}
		if (NULL != this_obj->srs_src_fctx){
			avformat_close_input(&this_obj->srs_src_fctx);
			this_obj->srs_src_fctx = NULL;
		}
	} while (0);

	free(this_obj);
	this_obj = NULL;

	return this_obj;


}


unsigned int  get_ff_decoder_obj_src_width(ff_decoder_obj  *obj)
{
	unsigned int value = 0;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		value = this_obj->srs_src_width;
	}
	return value;

}

unsigned int  get_ff_decoder_obj_src_height(ff_decoder_obj *obj)
{
	unsigned int value = 0;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		value = this_obj->srs_src_height;
	}
	return value;
}

unsigned int  get_ff_decoder_obj_dec_width(ff_decoder_obj  *obj)
{
	unsigned int value = 0;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		value = this_obj->srs_dec_width;
	}
	return value;
}

unsigned int  get_ff_decoder_obj_dec_height(ff_decoder_obj *obj)
{
	unsigned int value = 0;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		value = this_obj->srs_dec_height;
	}
	return value;
}

unsigned int  get_ff_decoder_obj_src_fps(ff_decoder_obj *obj)
{
	unsigned int  value = 0;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		value = this_obj->srs_src_fps;
	}
	return value;
}

char *get_ff_decoder_obj_src_filename(ff_decoder_obj *obj)
{
	char *value = NULL ;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		value = this_obj->srs_src_filename;
	}
	return value;

}

void set_ff_decoder_obj_dec_width(ff_decoder_obj  *obj, unsigned int value)
{
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		this_obj->srs_dec_width = value;
	}
}

void set_ff_decoder_obj_dec_height(ff_decoder_obj  *obj, unsigned int value)
{
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		this_obj->srs_dec_height = value;
	}
}

void set_ff_decoder_obj_src_filename(ff_decoder_obj *obj, const char *value)
{
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL != this_obj){
		memset(this_obj->srs_src_filename, 0, sizeof(char)*1024);
		memcpy(this_obj->srs_src_filename,value,strlen(value));
	}
}

unsigned char *ff_decoder_img_getbuf(ff_decoder_obj *obj)
{
	unsigned char *srs_dec_data = NULL;
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	do 
	{
		unsigned int pkt_dataLen = this_obj->srs_src_cdctx->width * this_obj->srs_src_cdctx->height * 3;
		av_new_packet(&this_obj->srs_src_pkt, pkt_dataLen);

		if (NULL == this_obj){
			break;
		}

		while (1){
			int ret = av_read_frame(this_obj->srs_src_fctx, &(this_obj->srs_src_pkt));
			if (ret < 0){
				printf("data of av_read_frame is null. \n");
				break;
			}
			if ((this_obj->srs_src_pkt.stream_index) == this_obj->srs_src_video_index){
				int got_picture = -1;
				int ret = avcodec_decode_video2(this_obj->srs_src_cdctx, this_obj->srs_src_frame, &got_picture, &(this_obj->srs_src_pkt));
				if (ret < 0){
					printf("decode error.\n");
					break;
				}
				if (1 == got_picture){
					int retheight = sws_scale(
						this_obj->srs_src_sws_ctx,
						(const uint8_t* const*)this_obj->srs_src_frame->data,
						this_obj->srs_src_frame->linesize,
						0,
						this_obj->srs_src_height,
						this_obj->srs_dec_frame->data,
						this_obj->srs_dec_frame->linesize
						);
					if (retheight == this_obj->srs_dec_height){
						static unsigned int Image_index_store = 0;
						if (0 == Image_index_store++){
							unsigned int i = 0;
							for (i= 0; i < IMG_FILE_SIZE; i++)
							{
								unsigned int img_type   = this_obj->srs_img_store[i].srs_img_type;
								unsigned int img_width  = this_obj->srs_img_store[i].srs_img_width;
								unsigned int img_height = this_obj->srs_img_store[i].srs_img_height;
								char *img_path_name = this_obj->srs_img_store[i].srs_img_path_name;
								if ((0 == img_width) || (0 == img_height) || (0 == strlen(img_path_name)))
								{
									continue;
								}
								if (0 == img_type){
									save_yuv_data_to_jpg_from_yuvframe(this_obj->srs_dec_frame, img_width, img_height, img_path_name);
								}
								if (1 == img_type){
									save_yuv_data_to_bmp_from_yuvframe(this_obj->srs_dec_frame, img_width, img_height, 24, img_path_name);
								}
							}
						}
						srs_dec_data = this_obj->srs_dec_buf;
						break;
					}
				}
			}
		}	
	} while (0);

	av_free_packet(&this_obj->srs_src_pkt);
	
	return srs_dec_data;

}


void set_ff_decoder_obj_img_store(ff_decoder_obj *obj, unsigned int  index, unsigned int img_width, unsigned int img_height, char *img_path_name, unsigned int img_type)
{
	srs_ffmpeg_stream_decoder *this_obj = (srs_ffmpeg_stream_decoder*)obj;
	if (NULL == this_obj){
		return;
	}
	if (NULL == img_path_name){
		return;
	}
	if (0 == img_width){
		return;
	}
	if (0 == img_height){
		return;
	}
	if (index > IMG_FILE_SIZE){
		return;
	}

	this_obj->srs_img_store[index].srs_img_type   = img_type;
	this_obj->srs_img_store[index].srs_img_width  = img_width;
	this_obj->srs_img_store[index].srs_img_height = img_height;
	memset(this_obj->srs_img_store[index].srs_img_path_name, 0, sizeof(char)*IMG_FILE_NAME);
	memcpy(this_obj->srs_img_store[index].srs_img_path_name, img_path_name, strlen(img_path_name));

}





void CHANGE_ENDIAN_24(unsigned char *data)
{
	char temp2 = data[2];
	data[2] = data[0];
	data[0] = temp2;
}

//change endian of a pixel (32bit)  
void CHANGE_ENDIAN_32(unsigned char *data)
{
	char temp3, temp2;
	temp3 = data[3];
	temp2 = data[2];
	data[3] = data[0];
	data[2] = data[1];
	data[0] = temp3;
	data[1] = temp2;
}

//Change endian of a picture  
void CHANGE_ENDIAN_PIC(unsigned char *image, int w, int h, int bpp)
{
	unsigned char *pixeldata = NULL;
	int i = 0;
	int j = 0;
	for (i = 0; i < h; i++)
	for (j = 0; j < w; j++){
		pixeldata = image + (i*w + j)*bpp / 8;
		if (bpp == 32){
			CHANGE_ENDIAN_32(pixeldata);
		}
		else if (bpp == 24){
			CHANGE_ENDIAN_24(pixeldata);
		}
	}
}

int save_yuv_data_to_bmp_from_yuvframe(AVFrame *yuvframe, const int width, const int height, const int bpp, const char *filename)
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
		unsigned int srcwidth  = yuvframe->width;
		unsigned int srcheight = yuvframe->height;
		unsigned int rgbwidth  = width;
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
		srcframe->data[0] = srcbuf;  // 좋똑Y
		srcframe->data[1] = srcbuf + src_y_size;  // U 
		srcframe->data[2] = srcbuf + src_y_size * 5 / 4; // V
		srcframe->width   = srcwidth;
		srcframe->height  = srcheight;
		srcframe->format  = AV_PIX_FMT_YUV420P;
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

			bmpinfo.biSize   = sizeof(BITMAPINFOHEADER);
			bmpinfo.biWidth  = rgbwidth;
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


int save_yuv_data_to_jpg_from_yuvframe(AVFrame *yuvframe, const int width, const int height, const char *filename)
{
	AVFormatContext *av_fmtctx    = NULL;
	AVStream        *av_stream    = NULL;
	AVCodecContext  *av_codec_ctx = NULL;
	AVCodec         *av_codec     = NULL;

	AVFrame *enc_frame       = NULL;
	unsigned char *enc_buf   = NULL;
	struct SwsContext *enc_sws_ctx = NULL;

	int flag = 0;
	do
	{
		if (NULL == yuvframe){
			break;
		}
		unsigned int src_width  = yuvframe->width ;
		unsigned int src_height = yuvframe->height;
		unsigned int enc_width  = width;
		unsigned int enc_height = height;

		enc_frame = av_frame_alloc();
		if (NULL == enc_frame){
			break;
		}
		unsigned int enc_buf_size = avpicture_get_size(AV_PIX_FMT_YUV420P,enc_width,enc_height);
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
		enc_frame->data[0] = enc_buf;  // 좋똑Y
		enc_frame->data[1] = enc_buf + src_y_size;  // U 
		enc_frame->data[2] = enc_buf + src_y_size * 5 / 4; // V
		enc_frame->width   = enc_width;
		enc_frame->height  = enc_height;
		enc_frame->format  = AV_PIX_FMT_YUV420P;
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
		av_codec_ctx->codec_id   = av_fmtctx->oformat->video_codec;
		av_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
		av_codec_ctx->pix_fmt    = PIX_FMT_YUVJ420P;
		av_codec_ctx->width  = enc_width;
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



