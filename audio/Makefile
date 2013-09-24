include ../../../Rules.make

ifeq ($(PLATFORM), HISI)
SUB_DIRS = 
else
SUB_DIRS = audio_in audio_out audio_demo
endif

.PHONY:all clean

all:
	@for i in $(SUB_DIRS); do $(MAKE) -C $$i; done
clean:
	@for i in $(SUB_DIRS); do $(MAKE) clean -C $$i ; done
install:
