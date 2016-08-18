
#ifndef FF_ENCODER_OBJ_H
#define FF_ENCODER_OBJ_H

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

typedef  void ffmpeg_encoder_obj;


//interface for encoder
ffmpeg_encoder_obj  *ff_encoder_obj_create();
ffmpeg_encoder_obj  *ff_encoder_enc_create(ffmpeg_encoder_obj  *obj);
ffmpeg_encoder_obj  *ff_encoder_obj_destory(ffmpeg_encoder_obj *obj);

//set ffmpeg_stream_encoder_obj param
void set_ff_encoder_obj_src_width(ffmpeg_encoder_obj  *obj, unsigned int  value);
void set_ff_encoder_obj_src_height(ffmpeg_encoder_obj *obj, unsigned int  value);
void set_ff_encoder_obj_enc_width(ffmpeg_encoder_obj  *obj, unsigned int  value);
void set_ff_encoder_obj_enc_height(ffmpeg_encoder_obj *obj, unsigned int  value);
void set_ff_encoder_obj_enc_fps(ffmpeg_encoder_obj *obj, unsigned int  value);

void set_ff_encoder_obj_enc_preset(ffmpeg_encoder_obj *obj, unsigned int  value);
void set_ff_encoder_obj_enc_profile(ffmpeg_encoder_obj *obj, unsigned int  value);
void set_ff_encoder_obj_enc_tune(ffmpeg_encoder_obj *obj, unsigned int  value);
void set_ff_encoder_obj_enc_bitrate(ffmpeg_encoder_obj *obj, unsigned int  value);
void set_ff_encoder_obj_enc_keyframeinterval(ffmpeg_encoder_obj *obj, unsigned int value);

//get param from ffmpeg_stream_encoder_obj
int           get_ff_encoder_obj_enc_iskeyframe(ffmpeg_encoder_obj *obj);
unsigned int  get_ff_encoder_obj_enc_data_len(ffmpeg_encoder_obj *obj);
unsigned char *get_ff_encoder_obj_enc_data(ffmpeg_encoder_obj *obj);

//encode data
unsigned char *ff_encoder_encode(ffmpeg_encoder_obj *obj, unsigned char *srcdata, unsigned int srcdatalen, unsigned int *encdatalen, unsigned int *iskeyframe);


/*
#ifdef __cplusplus
}
#endif
*/
#endif /* FF_ENCODER_OBJ_H */
