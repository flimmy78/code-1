include ../../../../Rules.make

DEMO_NAME = audio_demo
#DEMO_NAME = audio_in

ifeq ($(PLATFORM), AMBA)
LIBPREBUILD = -L $(SDK_PATH)/prebuild/third-party/alsa/lib -lasound
else
#LIBPREBUILD = -L $(LIB_PREBUILD_PATH) -lasound
LIBPREBUILD = -L $(LIB_PREBUILD_PATH) 
endif

HISI_INC_PATH = $(MODULES_SRC_DIR)/video/hisi/hi3518/video_api/inc/hi3518
ifeq ($(PLATFORM), HISI)
ifeq ($(CHIP), HI3518)
INC_PATH += -I $(HISI_INC_PATH)
endif
endif
export AUDIO_LIBA := $(MODULES_LIB_DIR)/libaudio_out.a  \
        $(MODULES_LIB_DIR)/libVoiceEngine.a \
		$(MODULES_LIB_DIR)/libaec.a	\
		$(MODULES_LIB_DIR)/libresampler.a \
		$(MODULES_LIB_DIR)/libanr.a 
#LIBMODULES = -L $(MODULES_LIB_DIR) -laudio_in -laudio_out
LIBMODULES =  -L $(MODULES_LIB_DIR) -laudio_in -laudio_out -lmpi -lVoiceEngine  -lresampler -lanr -laec
MPI_LIB := $(MODULES_LIB_DIR)/libmpi.a

LFLAGS += -lpthread -lm  

DEMO_SRC = *.c
DEMO_TARGET = $(DEMO_NAME) 
    
.PHONY:clean demo

demo:
	$(CC) -o $(DEMO_TARGET) $(CFLAGS) $(INC_PATH) $(DEMO_SRC) $(LFLAGS) $(LIBMODULES)

clean:  
	rm -f $(DEMO_TARGET)
install:
	$(INSTALL) $(LIBA) $(MODULES_LIB_DIR)

