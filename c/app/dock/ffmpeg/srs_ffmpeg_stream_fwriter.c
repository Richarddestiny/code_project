


#include "srs_ffmpeg_stream_fwriter.h"



#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>  
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>




#define USE_H264BSF 1
#define USE_AACBSF  1

#define MUX_FILE_TYPE_SIZE 100
#define MUX_FILE_NAME_SIZE 512

typedef struct tag_ffmpeg_stream_fwriter{
	AVFormatContext *srs_mux_ftmctx ; //
	AVStream        *srs_mux_audioST;
	AVStream        *srs_mux_videoST;

	AVBitStreamFilterContext *srs_mux_h264bsfc;
	AVBitStreamFilterContext *srs_mux_faacbsfc;

	unsigned int   srs_mux_width;
	unsigned int   srs_mux_height;
	unsigned int   srs_mux_fps;
	unsigned int   srs_mux_keyframeinterval;
	unsigned int   srs_mux_bitrate;
	unsigned int   srs_mux_samplerate;
	unsigned int   srs_mux_samplebit;
	unsigned int   srs_mux_soundchanel;

	long  srs_mux_video_pts;
	long  srs_mux_audio_pts;

	char srs_mux_file_type[MUX_FILE_TYPE_SIZE];
	char srs_mux_file_name[MUX_FILE_NAME_SIZE];

} srs_ffmpeg_stream_fwriter;


static void  flush_encoder_for_audio(AVFormatContext *fmt_ctx, unsigned int stream_index);
static void  flush_encoder_for_video(AVFormatContext *fmt_ctx, unsigned int stream_index);



ffmpeg_fwriter_obj   *ff_fwriter_obj_create()
{
	srs_ffmpeg_stream_fwriter *obj = (srs_ffmpeg_stream_fwriter*)malloc(sizeof(srs_ffmpeg_stream_fwriter));
	memset(obj, 0, sizeof(srs_ffmpeg_stream_fwriter));

	return obj;

}

ffmpeg_fwriter_obj   *ff_fwriter_mux_create(ffmpeg_fwriter_obj *obj)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;

	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	int flag = 0;
	do
	{
		if (NULL == this_obj){
			break;
		}
		this_obj->srs_mux_audio_pts = 0;
		this_obj->srs_mux_video_pts = 0;
		int ret = avformat_alloc_output_context2(&this_obj->srs_mux_ftmctx, NULL, this_obj->srs_mux_file_type, NULL);
		if (ret < 0){
			break;
		}
		this_obj->srs_mux_ftmctx->audio_codec_id = AV_CODEC_ID_AAC;
		this_obj->srs_mux_ftmctx->video_codec_id = AV_CODEC_ID_H264;
		this_obj->srs_mux_ftmctx->oformat->audio_codec = AV_CODEC_ID_AAC;
		this_obj->srs_mux_ftmctx->oformat->video_codec = AV_CODEC_ID_H264;
		this_obj->srs_mux_ftmctx->flags = 0;
		this_obj->srs_mux_ftmctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
		AVCodec *avcodec_audio = avcodec_find_encoder(this_obj->srs_mux_ftmctx->oformat->audio_codec);
		AVCodec *avcodec_video = avcodec_find_encoder(this_obj->srs_mux_ftmctx->oformat->video_codec);
		if ((NULL == avcodec_video) || (NULL == avcodec_audio)){
			break;
		}
		
		this_obj->srs_mux_audioST = avformat_new_stream(this_obj->srs_mux_ftmctx, avcodec_audio);
		this_obj->srs_mux_videoST = avformat_new_stream(this_obj->srs_mux_ftmctx, avcodec_video);
		if ((NULL == this_obj->srs_mux_audioST) || (NULL == this_obj->srs_mux_videoST)){
			break;
		}
		//this_obj->srs_mux_videoST->id = this_obj->srs_mux_ftmctx->nb_streams - 1;
		//this_obj->srs_mux_audioST->id = this_obj->srs_mux_ftmctx->nb_streams - 2;
		this_obj->srs_mux_videoST->codec->codec_tag = 0;
		this_obj->srs_mux_audioST->codec->codec_tag = 0;
		if (this_obj->srs_mux_ftmctx->oformat->flags & AVFMT_GLOBALHEADER){
			this_obj->srs_mux_audioST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			this_obj->srs_mux_videoST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		this_obj->srs_mux_audioST->codec->codec_id   = AV_CODEC_ID_AAC;
		this_obj->srs_mux_audioST->codec->codec_type = AVMEDIA_TYPE_AUDIO;
		enum AVSampleFormat cursampleformat = AV_SAMPLE_FMT_S16;
		//int cursampleformat = AV_SAMPLE_FMT_S16;
		if (8 == this_obj->srs_mux_samplebit){
			cursampleformat = AV_SAMPLE_FMT_U8;
		}
		if (32 == this_obj->srs_mux_samplebit){
			cursampleformat = AV_SAMPLE_FMT_S32;
		}
		this_obj->srs_mux_audioST->codec->sample_fmt = cursampleformat;// AV_SAMPLE_FMT_S16;
		this_obj->srs_mux_audioST->codec->sample_rate = this_obj->srs_mux_samplerate;// 44100;
		this_obj->srs_mux_audioST->codec->channels = this_obj->srs_mux_soundchanel;
		this_obj->srs_mux_audioST->codec->channel_layout = AV_CH_LAYOUT_STEREO;
		this_obj->srs_mux_audioST->codec->bit_rate = 96000;//64000
		this_obj->srs_mux_audioST->codec->channels = av_get_channel_layout_nb_channels(this_obj->srs_mux_audioST->codec->channel_layout);
		this_obj->srs_mux_audioST->time_base.den = this_obj->srs_mux_audioST->codec->sample_rate;
		this_obj->srs_mux_audioST->time_base.den = this_obj->srs_mux_samplerate;
		this_obj->srs_mux_audioST->time_base.num = 1;
		this_obj->srs_mux_audioST->codec->time_base.num = 1;
		this_obj->srs_mux_audioST->codec->time_base.den = this_obj->srs_mux_samplerate;

		this_obj->srs_mux_videoST->codec->codec_id   = AV_CODEC_ID_H264  ;// this->FMPEG4avoutfmt->video_codec;
		this_obj->srs_mux_videoST->codec->codec_type = AVMEDIA_TYPE_VIDEO;
		this_obj->srs_mux_videoST->codec->pix_fmt    = AV_PIX_FMT_YUV420P;
		this_obj->srs_mux_videoST->codec->width = this_obj->srs_mux_width;
		this_obj->srs_mux_videoST->codec->height = this_obj->srs_mux_height;
		this_obj->srs_mux_videoST->codec->time_base.num = 1;
		this_obj->srs_mux_videoST->codec->time_base.den = this_obj->srs_mux_fps;
		this_obj->srs_mux_videoST->time_base.num = 1;
		this_obj->srs_mux_videoST->time_base.den    = this_obj->srs_mux_fps;
		this_obj->srs_mux_videoST->codec->bit_rate  = this_obj->srs_mux_bitrate; //600kb
		this_obj->srs_mux_videoST->codec->gop_size  = this_obj->srs_mux_keyframeinterval;
		this_obj->srs_mux_videoST->codec->me_range  = 16;
		this_obj->srs_mux_videoST->codec->max_qdiff = 4;
		this_obj->srs_mux_videoST->codec->qmin = 10;
		this_obj->srs_mux_videoST->codec->qmax = 51;
		this_obj->srs_mux_videoST->codec->qcompress = 0.6;
		this_obj->srs_mux_videoST->codec->refs = 3;

		AVDictionary *param = NULL;
		if (this_obj->srs_mux_videoST->codec->codec_id == AV_CODEC_ID_H264) {
			av_dict_set(&param, "preset", "slow", 0); //superfast;ultrafast;medium;
			av_dict_set(&param, "tune", "zerolatency", 0);//不用会拖尾,实时编码
			//av_dict_set(&param, "profile", "main", 0);
			//H.264 Baseline Level 3.0, Baseline Level 3.1, Main Level 3.1, and High Profile Level 4.1.
			//av_dict_set(&param, "profile", "baseline", 0);//默认为 High@L3.1
		}
		// some formats want stream headers to be separate  
		if (this_obj->srs_mux_videoST->codec->flags & AVFMT_GLOBALHEADER) {
			this_obj->srs_mux_videoST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			this_obj->srs_mux_audioST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		this_obj->srs_mux_videoST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		this_obj->srs_mux_audioST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

		if (this_obj->srs_mux_ftmctx->oformat->flags & AVFMT_GLOBALHEADER) {
			this_obj->srs_mux_ftmctx->oformat->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		// some formats want stream headers to be separate
		if (!strcmp(this_obj->srs_mux_ftmctx->oformat->name, "mp4") ||
			!strcmp(this_obj->srs_mux_ftmctx->oformat->name, "mov") ||
			!strcmp(this_obj->srs_mux_ftmctx->oformat->name, "3gp")){
			this_obj->srs_mux_videoST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			this_obj->srs_mux_audioST->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		if (avcodec_open2(this_obj->srs_mux_audioST->codec, avcodec_audio, NULL) < 0){
			break;
		}
		if (avcodec_open2(this_obj->srs_mux_videoST->codec, avcodec_video, &param) < 0){
			break;
		}
		av_dict_free(&param);
		this_obj->srs_mux_ftmctx->flags = 0;
		this_obj->srs_mux_ftmctx->flags |= CODEC_FLAG_GLOBAL_HEADER;
		if (avio_open(&this_obj->srs_mux_ftmctx->pb, this_obj->srs_mux_file_name, AVIO_FLAG_WRITE) < 0){
			break;
		}
		av_dump_format(this_obj->srs_mux_ftmctx, 0, this_obj->srs_mux_file_name, 1);
		ret = avformat_write_header(this_obj->srs_mux_ftmctx, NULL);
		if (0 != ret){
			break;
		}	
		#if USE_H264BSF
				this_obj->srs_mux_h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
				if (NULL == this_obj->srs_mux_h264bsfc){
					break;
				}
		#endif
		#if USE_AACBSF
				this_obj->srs_mux_faacbsfc = av_bitstream_filter_init("aac_adtstoasc");
				if (NULL == this_obj->srs_mux_faacbsfc){
					break;
				}
		#endif

		flag = 1;

	} while (0);

	if (flag){
		return this_obj;
	}else{
		return NULL;
	}
	
}

ffmpeg_fwriter_obj   *ff_fwriter_obj_destory(ffmpeg_fwriter_obj *obj)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	do
	{
		if (NULL == this_obj){
			break;
		}
		this_obj->srs_mux_audio_pts = 0;
		this_obj->srs_mux_video_pts = 0;
		avcodec_close(this_obj->srs_mux_audioST->codec);
		if (NULL != this_obj->srs_mux_audioST->codec){
			avcodec_free_context(&this_obj->srs_mux_audioST->codec);
		}
		this_obj->srs_mux_audioST = NULL;
		avcodec_close(this_obj->srs_mux_videoST->codec);
		if (NULL != this_obj->srs_mux_videoST->codec){
			avcodec_free_context(&this_obj->srs_mux_videoST->codec);
		}
		this_obj->srs_mux_videoST = NULL;
		avio_close(this_obj->srs_mux_ftmctx->pb);
		if (NULL != this_obj->srs_mux_ftmctx){
			//avformat_free_context(this_obj->srs_mux_ftmctx);
		}
		this_obj->srs_mux_ftmctx = NULL;
		#if USE_H264BSF
			if (NULL != this_obj->srs_mux_h264bsfc){
				av_bitstream_filter_close(this_obj->srs_mux_h264bsfc);
				this_obj->srs_mux_h264bsfc = NULL;
			}
		#endif
		#if USE_AACBSF
			if (NULL != this_obj->srs_mux_faacbsfc){
				av_bitstream_filter_close(this_obj->srs_mux_faacbsfc);
				this_obj->srs_mux_faacbsfc = NULL;
			}
		#endif	

	} while (0);

	 free(this_obj);
	this_obj = NULL;

	return this_obj;

}

void ff_fwriter_mux_width(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_width = value;
	}

}

void ff_fwriter_mux_height(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_height = value;
	}

}

void ff_fwriter_mux_fps(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_fps = value;
	}

}

void ff_fwriter_mux_keyframeinterval(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_keyframeinterval = value;
	}

}

void ff_fwriter_mux_bitrate(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_bitrate = value;
	}

}

void ff_fwriter_mux_samplerate(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_samplerate = value;
	}

}

void ff_fwriter_mux_samplebit(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_samplebit = value;
	}

}

void ff_fwriter_mux_soundchanel(ffmpeg_fwriter_obj *obj, unsigned int value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		this_obj->srs_mux_soundchanel = value;
	}

}

void ff_fwriter_mux_file_type(ffmpeg_fwriter_obj *obj, char *value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		memset(this_obj->srs_mux_file_type, 0, sizeof(char)*MUX_FILE_TYPE_SIZE);
		memcpy(this_obj->srs_mux_file_type,value,MUX_FILE_TYPE_SIZE);
	}

}

void ff_fwriter_mux_file_name(ffmpeg_fwriter_obj *obj, char *value)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	if (NULL != this_obj){
		memset(this_obj->srs_mux_file_name,0,MUX_FILE_NAME_SIZE);
		memcpy(this_obj->srs_mux_file_name,value,MUX_FILE_NAME_SIZE);
	}
}




void  ff_fwriter_mux_finish(ffmpeg_fwriter_obj *obj)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;
	do
	{
		if (NULL != this_obj)
		{
			if (NULL != this_obj->srs_mux_ftmctx){
				flush_encoder_for_audio(this_obj->srs_mux_ftmctx, this_obj->srs_mux_audioST->index);
				flush_encoder_for_video(this_obj->srs_mux_ftmctx, this_obj->srs_mux_videoST->index);
				av_write_trailer(this_obj->srs_mux_ftmctx);
			}
		}
	} while (0);

}


int  ff_fwriter_mux_write_audio(ffmpeg_fwriter_obj *obj, unsigned char *pdata, unsigned int dataLen, double timeamp)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;

	int flag = 0;
	AVPacket pkt;
	do
	{
		if (NULL == this_obj){
			break;
		}
		if (NULL == pdata){
			break;
		}
		if (0 == dataLen){
			break;
		}
		av_new_packet(&pkt, dataLen);
		memcpy(pkt.data, pdata, dataLen);
		#if USE_AACBSF
		av_bitstream_filter_filter(this_obj->srs_mux_faacbsfc, this_obj->srs_mux_audioST->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		#endif

		pkt.pts = timeamp;
		pkt.pts = (this_obj->srs_mux_audio_pts++) * (this_obj->srs_mux_audioST->codec->frame_size * 1000 / this_obj->srs_mux_audioST->codec->sample_rate);
		//pkt.pts = (this_obj->srs_mux_audio_pts++) * (1024 * 1000.00 / this_obj->srs_mux_samplerate);
		pkt.dts = pkt.pts;
		AVRational base_time = { 1, 1000 };
		pkt.pts = av_rescale_q(pkt.pts, base_time, this_obj->srs_mux_audioST->time_base);
		pkt.dts = av_rescale_q(pkt.dts, base_time, this_obj->srs_mux_audioST->time_base);
		pkt.duration = this_obj->srs_mux_audioST->codec->frame_size;
		pkt.duration = av_rescale_q(pkt.duration, base_time, this_obj->srs_mux_audioST->time_base);

		pkt.pos = -1;
		pkt.stream_index = this_obj->srs_mux_audioST->index;
		int ret = av_write_frame(this_obj->srs_mux_ftmctx, &pkt);
		if (ret < 0){
			break;
		}
		flag = 1;
	} while (0);

	av_free_packet(&pkt);

	return flag;


}


int  ff_fwriter_mux_write_video(ffmpeg_fwriter_obj *obj, unsigned char *pdata, unsigned int dataLen, double timeamp, int iskeyframe)
{
	srs_ffmpeg_stream_fwriter *this_obj = (srs_ffmpeg_stream_fwriter*)obj;

	int flag = 0;
	AVPacket pkt;
	do
	{
		if ((NULL == this_obj) || (NULL == pdata) || (0 == dataLen)){
			break;
		}
		av_new_packet(&pkt, dataLen);
		memcpy(pkt.data, pdata, dataLen);
		pkt.flags = iskeyframe;
		#if USE_H264BSF
		if (this_obj->srs_mux_h264bsfc){
			av_bitstream_filter_filter(this_obj->srs_mux_h264bsfc, this_obj->srs_mux_videoST->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		}
		#endif
		pkt.pts = timeamp;
		//pkt.pts = this_obj->srs_mux_video_pts++ * (this_obj->srs_mux_videoST->time_base.num * 1000 / this_obj->srs_mux_videoST->time_base.den);
		pkt.pts = this_obj->srs_mux_video_pts++ * (1000 / this_obj->srs_mux_fps);
		pkt.dts = pkt.pts;
		pkt.dts = pkt.pts;
		AVRational base_time = { 1, 1000 };
		pkt.pts = av_rescale_q(pkt.pts, base_time, this_obj->srs_mux_videoST->time_base);
		pkt.dts = av_rescale_q(pkt.dts, base_time, this_obj->srs_mux_videoST->time_base);
		pkt.duration = base_time.den / this_obj->srs_mux_fps;
		pkt.duration = av_rescale_q(pkt.duration, base_time, this_obj->srs_mux_videoST->time_base);
		pkt.pos = -1;
		pkt.stream_index = this_obj->srs_mux_videoST->index;
		pkt.flags = iskeyframe;
		if (av_dup_packet(&pkt) < 0) {
			av_free_packet(&pkt);
		}
		int ret = av_write_frame(this_obj->srs_mux_ftmctx, &pkt);
		if (ret < 0){
			break;
		}
		flag = 1;

	} while (0);

	av_free_packet(&pkt);

	return flag;


}


void  flush_encoder_for_audio(AVFormatContext *fmt_ctx, unsigned int stream_index)
{	
	do 
	{	
		if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &CODEC_CAP_DELAY)){
			break;
		}
		AVPacket enc_pkt;
		while (1) {
			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			av_init_packet(&enc_pkt);
			int got_frame = -1;
			int ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,NULL, &got_frame);
			av_frame_free(NULL);
			if (ret < 0){
				break;
			}	
			if (!got_frame){
				break;
			}
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
			/* mux encoded frame */
			ret = av_write_frame(fmt_ctx, &enc_pkt);
			if (ret < 0){
				break;
			}		
		}
	} while (0);
	

}

void flush_encoder_for_video(AVFormatContext *fmt_ctx, unsigned int stream_index)
{
	do
	{
		if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &CODEC_CAP_DELAY)){
			break;
		}
		AVPacket enc_pkt;
		while (1) {
			enc_pkt.data = NULL;
			enc_pkt.size = 0;
			av_init_packet(&enc_pkt);
			//static long enc_pkt_pts = enc_pkt.pts;
			//enc_pkt.pts = enc_pkt.pts + 500;
			int got_frame = -1;
			int ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt, NULL, &got_frame);
			av_frame_free(NULL);
			if (ret < 0){
				break;
			}
			if (!got_frame){
				break;
			}
			printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
			/* mux encoded frame */
			ret = av_write_frame(fmt_ctx, &enc_pkt);
			if (ret < 0){
				break;
			}
		}
	} while (0);

}








