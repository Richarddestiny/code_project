
#ifndef FF_DECODER_OBJ_H
#define FF_DECODER_OBJ_H

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

typedef  void ff_decoder_obj;

//interface for decoder
ff_decoder_obj  *ff_decoder_obj_create();
ff_decoder_obj  *ff_decoder_dec_create(ff_decoder_obj  *obj);
ff_decoder_obj  *ff_decoder_obj_destory(ff_decoder_obj *obj);
unsigned char   *ff_decoder_img_getbuf(ff_decoder_obj *obj);

unsigned int  get_ff_decoder_obj_src_width(ff_decoder_obj  *obj);
unsigned int  get_ff_decoder_obj_src_height(ff_decoder_obj *obj);
unsigned int  get_ff_decoder_obj_dec_width(ff_decoder_obj  *obj);
unsigned int  get_ff_decoder_obj_dec_height(ff_decoder_obj *obj);
unsigned int  get_ff_decoder_obj_src_fps(ff_decoder_obj *obj);
char         *get_ff_decoder_obj_src_filename(ff_decoder_obj *obj);

void set_ff_decoder_obj_dec_width(ff_decoder_obj  *obj,unsigned int value);
void set_ff_decoder_obj_dec_height(ff_decoder_obj  *obj, unsigned int value);
void set_ff_decoder_obj_src_filename(ff_decoder_obj *obj, const char *value);
void set_ff_decoder_obj_img_store(ff_decoder_obj *obj,unsigned int index,unsigned int img_width, unsigned int img_height, char *img_path_name,unsigned int img_type);


/*
#ifdef __cplusplus
}
#endif
*/
#endif /* FF_DECODER_OBJ_H */




