
#include "srs_ffmpeg_stream_encoder.h"


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>  
#include <libavutil/pixfmt.h>





//interface for encoder
typedef struct tag_ffmpeg_stream_encoder{
	unsigned int  srs_src_width;
	unsigned int  srs_src_height;
	unsigned int  srs_enc_width;
	unsigned int  srs_enc_height;
	AVFrame       *srs_src_frame;
	AVFrame       *srs_enc_frame;
	unsigned char *srs_src_frame_buf;
	unsigned char *srs_enc_frame_buf;
	unsigned int  srs_src_frame_buf_len;
	unsigned int  srs_enc_frame_buf_len;
	unsigned char *srs_enc_data;
	unsigned int  srs_enc_data_len;
	unsigned int  srs_enc_fps;
	unsigned int  srs_enc_preset;
	unsigned int  srs_enc_profile;
	unsigned int  srs_enc_tune;

	unsigned int  srs_enc_keyframeinterval;
	unsigned int  srs_enc_bitrate;
	int          srs_enc_iskeyframe;

	AVCodecContext    *srs_enc_codecctx;
	AVCodec           *srs_enc_codec;
	struct SwsContext *srs_enc_swscontext;

} srs_ffmpeg_stream_encoder;



ffmpeg_encoder_obj *ff_encoder_obj_create()
{
	srs_ffmpeg_stream_encoder *obj = (srs_ffmpeg_stream_encoder *)malloc(sizeof(srs_ffmpeg_stream_encoder));
	memset(obj, 0, sizeof(srs_ffmpeg_stream_encoder));
	obj->srs_src_width = 1280;
	obj->srs_src_height = 1080;
	obj->srs_src_width = obj->srs_src_width;
	obj->srs_src_height = obj->srs_src_height;
	obj->srs_src_frame = NULL;
	obj->srs_enc_frame = NULL;
	obj->srs_src_frame_buf = NULL;
	obj->srs_enc_frame_buf = NULL;
	obj->srs_src_frame_buf_len = 0;
	obj->srs_enc_frame_buf_len = 0;
	obj->srs_enc_data_len = 0;
	obj->srs_enc_data = NULL;
	obj->srs_enc_fps = 25;
	obj->srs_enc_preset = 6;
	obj->srs_enc_profile = 2;
	obj->srs_enc_tune = 7;
	obj->srs_enc_keyframeinterval = 5;
	obj->srs_enc_bitrate = 6000000; //600kb;
	obj->srs_enc_iskeyframe = 0;
	obj->srs_enc_codecctx = NULL;
	obj->srs_enc_codec = NULL;
	obj->srs_enc_swscontext = NULL;

	return obj;

}

ffmpeg_encoder_obj  *ff_encoder_enc_create(ffmpeg_encoder_obj  *obj)
{
	av_register_all();
	avformat_network_init();
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	int flag = 0;
	do
	{
		if (NULL == this_obj){
			break;
		}
		this_obj->srs_enc_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		if (NULL == this_obj->srs_enc_codec){
			break;
		}
		this_obj->srs_enc_codecctx = avcodec_alloc_context3(this_obj->srs_enc_codec);
		if (NULL == this_obj->srs_enc_codecctx){
			break;
		}
		//set codecctx
		avcodec_get_context_defaults3(this_obj->srs_enc_codecctx, this_obj->srs_enc_codec);
		this_obj->srs_enc_codecctx->codec_id = AV_CODEC_ID_H264;
		this_obj->srs_enc_codecctx->codec_type = AVMEDIA_TYPE_VIDEO;
		this_obj->srs_enc_codecctx->pix_fmt = AV_PIX_FMT_YUV420P;
		this_obj->srs_enc_codecctx->width = this_obj->srs_enc_width;
		this_obj->srs_enc_codecctx->height = this_obj->srs_enc_height;
		this_obj->srs_enc_codecctx->time_base.num = 1;
		this_obj->srs_enc_codecctx->time_base.den = this_obj->srs_enc_fps;
		this_obj->srs_enc_codecctx->bit_rate = this_obj->srs_enc_bitrate; //600kb
		this_obj->srs_enc_codecctx->gop_size = this_obj->srs_enc_keyframeinterval;// 250;gop_size 为1时，输入8帧即可出图像，10时，输入20帧出图像
		this_obj->srs_enc_codecctx->me_range = 16;
		this_obj->srs_enc_codecctx->max_qdiff = 4;
		this_obj->srs_enc_codecctx->qmin = 10;
		this_obj->srs_enc_codecctx->qmax = 51;
		this_obj->srs_enc_codecctx->qcompress = 0.6;

		//ffmpeg中CBR（固定码率控制）的设置：
		this_obj->srs_enc_codecctx->bit_rate = this_obj->srs_enc_bitrate;
		this_obj->srs_enc_codecctx->rc_min_rate = this_obj->srs_enc_bitrate;
		this_obj->srs_enc_codecctx->rc_max_rate = this_obj->srs_enc_bitrate;
		this_obj->srs_enc_codecctx->bit_rate_tolerance = this_obj->srs_enc_bitrate;
		this_obj->srs_enc_codecctx->rc_buffer_size = this_obj->srs_enc_bitrate;
		this_obj->srs_enc_codecctx->rc_initial_buffer_occupancy = this_obj->srs_enc_codecctx->rc_buffer_size * 3 / 4;
		//this_obj->srs_enc_codecctx->rc_buffer_aggressivity = (float)1.0;
		//this_obj->srs_enc_codecctx->rc_initial_cplx = 0.5;
		/*
		//ffmpeg中VBR（可变率控制）的设置：
		this_obj->srs_enc_codecctx->flags |= CODEC_FLAG_QSCALE;
		this_obj->srs_enc_codecctx->rc_min_rate = this_obj->bitrate;
		this_obj->srs_enc_codecctx->rc_max_rate = this_obj->bitrate;
		this_obj->srs_enc_codecctx->bit_rate    = this_obj->bitrate;
		*/
		//取值0.01-255，越小质量越好
		this_obj->srs_enc_codecctx->global_quality = 0.01;
		if (this_obj->srs_enc_codecctx->flags & AVFMT_GLOBALHEADER) {
			this_obj->srs_enc_codecctx->flags |= CODEC_FLAG_GLOBAL_HEADER; //注意：pps sps
		}
		AVDictionary *param = NULL;
		if (this_obj->srs_enc_codecctx->codec_id == AV_CODEC_ID_H264) {
			char *x264_preset_names[] = {
				"ultrafast",
				"superfast",
				"veryfast",
				"faster",
				"fast",
				"medium",
				"slow",
				"slower",
				"veryslow",
				"placebo",
				0
			};
			char *x264_profile_names[] = {
				"baseline",
				"main",
				"high",
				"high10",
				"high422",
				"high444",
				0
			};
			char *x264_tune_names[] = {
				"film",
				"animation",
				"grain",
				"stillimage",
				"psnr",
				"ssim",
				"fastdecode",
				"zerolatency",
				0
			};
			char *pLevel = x264_preset_names[this_obj->srs_enc_preset];
			char *pProfile = x264_profile_names[this_obj->srs_enc_profile];
			char *pTune = x264_tune_names[this_obj->srs_enc_tune];
			av_dict_set(&param, "preset", pLevel, 0);
			//av_dict_set(&param, "tune", "zerolatency", 0);
			av_dict_set(&param, "tune", pTune, 0);
			av_dict_set(&param, "profile", pProfile, 0);
		}
		//H.265
		if (this_obj->srs_enc_codecctx->codec_id == AV_CODEC_ID_H265){
			av_dict_set(&param, "preset", "ultrafast", 0);
			av_dict_set(&param, "tune", "zero-latency", 0);
		}
		if (avcodec_open2(this_obj->srs_enc_codecctx, this_obj->srs_enc_codec, &param) < 0){
			break;
		}
		av_dict_free(&param);

		//for src frame
		this_obj->srs_src_frame = av_frame_alloc();
		if (NULL == this_obj->srs_src_frame){
			break;
		}
		this_obj->srs_src_frame_buf_len = avpicture_get_size(
			AV_PIX_FMT_YUV420P,
			this_obj->srs_src_width,
			this_obj->srs_src_height
			);

		this_obj->srs_src_frame_buf = (unsigned char *)av_malloc(this_obj->srs_src_frame_buf_len);
		if (NULL == this_obj->srs_src_frame_buf){
			break;
		}
		avpicture_fill(
			(AVPicture *)this_obj->srs_src_frame,
			this_obj->srs_src_frame_buf,
			AV_PIX_FMT_YUV420P,
			this_obj->srs_src_width,
			this_obj->srs_src_height
			);
		unsigned int y_src_size = this_obj->srs_src_width * this_obj->srs_src_height;
		this_obj->srs_src_frame->data[0] = this_obj->srs_src_frame_buf;  // 亮度Y
		this_obj->srs_src_frame->data[1] = this_obj->srs_src_frame_buf + y_src_size;  // U 
		this_obj->srs_src_frame->data[2] = this_obj->srs_src_frame_buf + y_src_size * 5 / 4; // V
		this_obj->srs_src_frame->width   = this_obj->srs_src_width;
		this_obj->srs_src_frame->height  = this_obj->srs_src_height;
		this_obj->srs_src_frame->format  = AV_PIX_FMT_YUV420P;

		//for convert frame 
		this_obj->srs_enc_frame = av_frame_alloc();
		if (NULL == this_obj->srs_enc_frame){
			break;
		}
		this_obj->srs_enc_frame_buf_len = avpicture_get_size(
			AV_PIX_FMT_YUV420P,
			this_obj->srs_enc_width,
			this_obj->srs_enc_height);
		this_obj->srs_enc_data_len = this_obj->srs_enc_frame_buf_len;
		this_obj->srs_enc_frame_buf = (unsigned char *)av_malloc(this_obj->srs_enc_frame_buf_len);
		if (NULL == this_obj->srs_enc_frame_buf){
			break;
		}
		avpicture_fill(
			(AVPicture *)this_obj->srs_enc_frame,
			this_obj->srs_enc_frame_buf,
			AV_PIX_FMT_YUV420P,
			this_obj->srs_enc_width,
			this_obj->srs_enc_height);
		unsigned int y_enc_size = this_obj->srs_enc_width * this_obj->srs_enc_height;
		this_obj->srs_enc_frame->data[0] = this_obj->srs_enc_frame_buf;  // 亮度Y
		this_obj->srs_enc_frame->data[1] = this_obj->srs_enc_frame_buf + y_enc_size;  // U 
		this_obj->srs_enc_frame->data[2] = this_obj->srs_enc_frame_buf + y_enc_size * 5 / 4; // V
		this_obj->srs_enc_frame->width   = this_obj->srs_enc_width;
		this_obj->srs_enc_frame->height  = this_obj->srs_enc_height;
		this_obj->srs_enc_frame->format  = AV_PIX_FMT_YUV420P;

		//begin  create  convert
		this_obj->srs_enc_swscontext = sws_getContext(
			this_obj->srs_src_width,
			this_obj->srs_src_height,
			AV_PIX_FMT_YUV420P,
			this_obj->srs_enc_width,
			this_obj->srs_enc_height,
			AV_PIX_FMT_YUV420P,
			SWS_BICUBIC,
			NULL,
			NULL,
			NULL
			);
		if (NULL == this_obj->srs_enc_swscontext){
			break;
		}

		this_obj->srs_enc_data_len = this_obj->srs_enc_data_len * 2;
		this_obj->srs_enc_data = (unsigned char*)av_malloc(this_obj->srs_enc_data_len);
		memset(this_obj->srs_enc_data, 0, this_obj->srs_enc_data_len);
		if (NULL == this_obj->srs_enc_data){
			break;
		}
		flag = 1;

	} while (0);


	return obj;

}

ffmpeg_encoder_obj *ff_encoder_obj_destory(ffmpeg_encoder_obj *obj)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		if (NULL != this_obj->srs_enc_codec){
			av_free(this_obj->srs_enc_codec);
			this_obj->srs_enc_codec = NULL;
		}
		if (NULL != this_obj->srs_enc_codecctx){
			avcodec_close(this_obj->srs_enc_codecctx);
			avcodec_free_context(&this_obj->srs_enc_codecctx);
			this_obj->srs_enc_codecctx = NULL;
		}
		if (NULL != this_obj->srs_src_frame){
			av_frame_free(&this_obj->srs_src_frame);
			this_obj->srs_src_frame = NULL;
		}
		if (NULL != this_obj->srs_enc_frame){
			av_frame_free(&this_obj->srs_enc_frame);
			this_obj->srs_enc_frame = NULL;
		}
		if (NULL != this_obj->srs_src_frame_buf){
			av_free(this_obj->srs_src_frame_buf);
			this_obj->srs_src_frame_buf = NULL;
		}
		if (NULL != this_obj->srs_enc_frame_buf){
			av_free(this_obj->srs_enc_frame_buf);
			this_obj->srs_enc_frame_buf = NULL;
		}
		if (NULL != this_obj->srs_enc_swscontext){
			sws_freeContext(this_obj->srs_enc_swscontext);
			this_obj->srs_enc_swscontext = NULL;
		}
		if (NULL != this_obj->srs_enc_data){
			av_free(this_obj->srs_enc_data);
			this_obj->srs_enc_data = NULL;
		}
		free(obj);
		obj = NULL;
	}

	return obj;

}

//set ffmpeg_stream_encoder_obj param
void set_ff_encoder_obj_src_width(ffmpeg_encoder_obj  *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_src_width = value;
	}
}

void set_ff_encoder_obj_src_height(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_src_height = value;
	}
}

void set_ff_encoder_obj_enc_width(ffmpeg_encoder_obj  *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_width = value;
	}
}

void set_ff_encoder_obj_enc_height(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_height = value;
	}
}

void set_ff_encoder_obj_enc_fps(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_fps = value;
	}
}

void set_ff_encoder_obj_enc_preset(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_preset = value;
	}
}

void set_ff_encoder_obj_enc_profile(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_profile = value;
	}
}

void set_ff_encoder_obj_enc_tune(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_tune = value;
	}
}

void set_ff_encoder_obj_enc_bitrate(ffmpeg_encoder_obj *obj, unsigned int  value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_bitrate = value;
	}
}

void set_ff_encoder_obj_enc_keyframeinterval(ffmpeg_encoder_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		this_obj->srs_enc_keyframeinterval = value;
	}
}


//get param from ffmpeg_stream_encoder_obj
int   get_ff_encoder_obj_enc_iskeyframe(ffmpeg_encoder_obj *obj)
{
	int is_key_frame = 0;
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		is_key_frame = this_obj->srs_enc_iskeyframe;
	}
	return is_key_frame;
}

unsigned int get_ff_encoder_obj_enc_data_len(ffmpeg_encoder_obj *obj)
{
	unsigned int enc_data_len = 0;
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		enc_data_len = this_obj->srs_enc_data_len;
	}
	return enc_data_len;
}

unsigned char *get_ff_encoder_obj_enc_data(ffmpeg_encoder_obj *obj)
{
	unsigned char *enc_data = NULL;
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;
	if (NULL != this_obj){
		enc_data = this_obj->srs_enc_data;
	}
	return enc_data;
}



unsigned char *ff_encoder_encode(ffmpeg_encoder_obj *obj, unsigned char *srcdata, unsigned int srcdatalen, unsigned int *encdatalen, unsigned int *iskeyframe)
{
	srs_ffmpeg_stream_encoder *this_obj = (srs_ffmpeg_stream_encoder *)obj;

	int flag = 0;
	int datalen = 0;
	AVPacket pkt;
	do
	{
		if (NULL == this_obj){
			break;
		}
		if (NULL == this_obj->srs_enc_codec){
			break;
		}
		if (NULL == this_obj->srs_enc_codecctx){
			break;
		}
		if (NULL == this_obj->srs_enc_swscontext){
			break;
		}
		if (NULL == this_obj->srs_src_frame){
			break;
		}
		if (NULL == srcdata){
			break;
		}
		if (0 == srcdatalen){
			break;
		}
		if (srcdatalen > this_obj->srs_src_frame_buf_len){
			break;
		}

		memset(this_obj->srs_src_frame_buf, 0, this_obj->srs_src_frame_buf_len);
		memcpy(this_obj->srs_src_frame_buf, srcdata, srcdatalen/* this_obj->srs_src_frame_buf_len*/);
		int ret = sws_scale(
			this_obj->srs_enc_swscontext,
			(const uint8_t* const*)this_obj->srs_src_frame->data,
			this_obj->srs_src_frame->linesize,
			0,
			this_obj->srs_src_height,
			this_obj->srs_enc_frame->data,
			this_obj->srs_enc_frame->linesize
			);
		if (ret != this_obj->srs_enc_height){
			break;
		}

		unsigned int y_enc_size = this_obj->srs_enc_width * this_obj->srs_enc_height;
		av_new_packet(&pkt, y_enc_size * 3 / 2);
		int got_packet_ptr = 0;
		int u_size = avcodec_encode_video2(this_obj->srs_enc_codecctx, &pkt, this_obj->srs_enc_frame, &got_packet_ptr);
		if ((0 == u_size) && (1 == got_packet_ptr) && (NULL != pkt.data) && (pkt.size > 0)){
			if (this_obj->srs_enc_data){
				memset(this_obj->srs_enc_data, 0, sizeof(char)* this_obj->srs_enc_data_len);
				memcpy(this_obj->srs_enc_data, pkt.data, pkt.size);
				datalen = pkt.size;
			}
			this_obj->srs_enc_iskeyframe = (int)pkt.flags;
			*iskeyframe = pkt.flags;
			flag = 1;
		}

	} while (0);

	av_free_packet(&pkt);

	*encdatalen = 0;
	if (datalen >= 0){
		*encdatalen = datalen;
	}
	if ((1 == flag) && (datalen > 0)){
		return this_obj->srs_enc_data;
	}
	else{
		return NULL;
	}

}






