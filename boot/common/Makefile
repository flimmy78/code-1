-include $(TOPDIR)/config.mk


LIB =  libcommon.a
SRC := vsprintf.o div64.o debug.o
ALL:$(LIB)
	@echo "\t\n Compile drivers ... \n"

$(LIB):$(SRC)
	@$(AR) -cr $@ $(SRC)
clean:
	@rm *.o *.a -f
