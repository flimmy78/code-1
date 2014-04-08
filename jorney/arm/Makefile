-include $(TOPDIR)/config.mk
ASRC := start.o
CSRC := $(wildcard lib/*.o)
LIB = libhi3518.a

all: $(LIB)
	@echo "\t\nCompile arch ... \n"

$(LIB):$(ASRC)
	$(AR) -cr $@ $(ASRC) $(CSRC)

start.o:start.S
	$(Q)$(CC) $(AFLAGS) $(CFLAGS) -o $@ $< -c
clean:
	$(Q)rm *.o *.a -f
