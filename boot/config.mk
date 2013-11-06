PWD := /home/jorney/develop/boot
CROSS_COMPILE := arm-hisiv100nptl-linux-
CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld
NM:= $(CROSS_COMPILE)nm
COPY := $(CROSS_COMPILE)objcopy

TEXT_BASE := 0x80800000
AFLAGS := -D__ASSEMBLY__
LDFLAGS := -Bstatic -T u-boot.lds -Ttext $(TEXT_BASE)
Q :=@
CFLAGS := -I$(PWD)/include

PLATFORM_LIBS := -L /opt/hisi-linux-nptl/arm-hisiv100-linux/bin/../lib/gcc/arm-hisiv100-linux-uclibcgnueabi/4.4.1 -lgcc

%.o:%c
	$(Q)$(CC) $(CFLAGS) -o $@ $< -c

export CC AR LD NM COPY Q CFLAGS PWD
