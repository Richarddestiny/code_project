

#ifndef FF_FWRITER_OBJ_H
#define FF_FWRITER_OBJ_H

/*
#ifdef __cplusplus
extern "C" {
#endif
*/


typedef  void ffmpeg_fwriter_obj;


ffmpeg_fwriter_obj   *ff_fwriter_obj_create();
ffmpeg_fwriter_obj   *ff_fwriter_mux_create(ffmpeg_fwriter_obj *obj);
ffmpeg_fwriter_obj   *ff_fwriter_obj_destory(ffmpeg_fwriter_obj *obj);


void ff_fwriter_mux_width(ffmpeg_fwriter_obj *obj,unsigned int value);
void ff_fwriter_mux_height(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_fps(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_keyframeinterval(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_bitrate(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_samplerate(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_samplebit(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_soundchanel(ffmpeg_fwriter_obj *obj, unsigned int value);
void ff_fwriter_mux_file_type(ffmpeg_fwriter_obj *obj, char *value);
void ff_fwriter_mux_file_name(ffmpeg_fwriter_obj *obj, char *value);


void  ff_fwriter_mux_finish(ffmpeg_fwriter_obj *obj);
int  ff_fwriter_mux_write_audio(ffmpeg_fwriter_obj *obj, unsigned char *pdata, unsigned int dataLen, double timeamp);
int  ff_fwriter_mux_write_video(ffmpeg_fwriter_obj *obj, unsigned char *pdata, unsigned int dataLen, double timeamp, int iskeyframe);

/*
#ifdef __cplusplus
}
#endif
*/
#endif /* FF_FWRITER_OBJ_H */







