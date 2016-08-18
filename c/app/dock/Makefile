CC := gcc
LD := gcc

HPATH := -I$(PWD)/include
HPATH += -mfloat-abi=hard -I$(CUDA_PATH)/include
HPATH += -I$(PWD)/codec/inc/core/include -I$(PWD)/codec/inc/cuda/include -I$(PWD)/codec/inc/graphics/2d/include -I$(PWD)/codec/inc/multimedia/include \
           -I$(PWD)/codec/inc/camera/core_v3/include -I$(PWD)/codec/inc/camera-partner/imager/include -I$(PWD)/codec/inc/lib_nv_camera_hal \
           -I$(PWD)/codec/inc/camera-partner/image_enc/include -I$(PWD)/codec/inc/multimedia-partner/openmax/include/openmax/il -I$(PWD)/codec/inc

TEGRA_LIB_DIR = /usr/lib/arm-linux-gnueabihf/tegra
		   
#lib for codec
LIB := -L$(TEGRA_LIB_DIR) \
       -L/usr/local/lib \
       -L/usr/lib/arm-linux-gnueabihf \
       -lcuda \
       -lrt \
       -lz \
       -ldl \
       -lnvomx \
       -lnvos \
       -lnvddk_2d_v2 \
       -lpthread \
       -lnvrm \

#lib for stitch	   
LIB += -lVisionSystem \
       -lStitching \
       -lOptimizePath \
       -llsdslam
	   
#lib for ffmpeg
LIB += -lavcodec    \
       -lavformat   \
       -lavutil     \
       -lswresample \
       -lswscale    \
       -lavdevice   \
       -lavfilter   \
       -lpostproc 
		   
	   
TARGET := dock

#########################################
# file compile area
#########################################

OBJS := main.o \
        dbg.o \
        mcu.o \
        app.o \
        ball.o \
        decode.o \
        stitch.o \
        encode.o \
        ffmpeg.o \
        preview.o \
	photo.o \
	video.o

OBJS += codec/nv_buffer.o \
        codec/util.o \
        codec/nv_omx_dec.o \
        codec/nv_omx_dec_1.o \
        codec/nv_omx_dec_2.o \
        codec/nv_omx_dec_3.o \
        codec/nv_omx_enc.o \
        ffmpeg/srs_ffmpeg_stream_fwriter.o \
        ffmpeg/srs_ffmpeg_stream_picture.o
		 
HEADERS := include/main.h \
           include/dbg.h \
           include/mcu.h \
           include/app.h \
           include/ball.h \
           include/decode.h \
           include/stitch.h \
           include/encode.h \
           include/ffmpeg.h \
           include/preview.h \
           include/photo.h \
           include/video.h

#########################################
#     Main Entry             
# Usage: make all
#########################################
all: $(OBJS)
	@echo "building $(TARGET)"
	$(LD)  $(OBJS) -o $(TARGET) $(LIB)

#============================================
#     规则定义区
#============================================
$(OBJS): %.o:%.c $(HEADERS)
	@echo "CC $<"
	@$(CC) -c $(CFLAGS) $(HPATH)  $< -o $@  

CLEAN_FILES :=  $(OBJS) \
                $(TARGET)

.PHONY : clean
clean: 
	@$(quiet_cmd_rmfiles)
	@$(cmd_rmfiles)

rm-files := $(CLEAN_FILES)
quiet_cmd_rmfiles = $(if $(wildcard $(rm-files)),echo "CLEAN   $(wildcard $(rm-files))")
      cmd_rmfiles = for i in $(rm-files);\
		    do \
			    if [ -f  $$i ]; then \
				    rm $$i;\
			    fi;\
			    if [ -d  $$i ]; then \
				    rm -rf $$i;\
			    fi;\
		    done
			
run:
	./$(TARGET)

