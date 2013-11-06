-include $(TOPDIR)/config.mk

LIB =  libdriver.a
SRC := serial_pl01x.o
ALL:$(LIB)
	@echo "\t\n Compile drivers ... \n"

$(LIB):$(SRC)
	@$(AR) -cr $@ $(SRC)
clean:
	@rm *.o *.a -f

