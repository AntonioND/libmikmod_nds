
export TARGET		:=	$(shell basename $(CURDIR))
export TOPDIR		:=	$(CURDIR)

all:
	make -C arm7
	make -C arm9

clean:
	make -C arm7 clean
	make -C arm9 clean
