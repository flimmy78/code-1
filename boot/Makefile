-include config.mk
ALIB += arm/libhi3518.a
DLIBS += driver/libdriver.a
TOPDIR := $(PWD)

SYSTEM_MAP = $(NM) $1| grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)'|LC_ALL=C sort


all: LIBS
	@$(LD) $(LDFLAGS) $(ALIB) $(DLIBS) lib/eabi_compat.o $(PLATFORM_LIBS) -Map u-boot.map -o u-boot
	$(Q)$(call SYSTEM_MAP ,u-boot) > System.map
	@$(COPY) --gap-fill=0xff -O binary u-boot u-boot.bin
	$(Q)dd if=u-boot.bin of=./fb1 bs=1 count=64
	$(Q)dd if=ddr/reg_info_hi3518a.bin of=./fb2 bs=4096 conv=sync
	$(Q)dd if=u-boot.bin of=./fb3 bs=1 skip=4160
	$(Q)cat fb1 fb2 fb3 > uboot-2ne1.bin
	$(Q)rm -f fb1 fb2 fb3
	#@sudo cp uboot-2ne1.bin ~/tftp/

LIBS:
	@make -C driver
	@make -C arm/lib
	@make -C arm
	@make -C lib

clean:
	@make -C driver clean
	@make -C arm/lib clean
	@make -C arm clean
	@make -C lib clean
	$(Q)rm *.o *.a *.map -f
export TOPDIR
