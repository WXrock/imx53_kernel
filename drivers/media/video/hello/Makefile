
obj-m := hello.o
modules-objs := myhello.o

KDIR :=/home/heu/i.MX53-QSB-Android-Gingerbread-Release4.2/src/kernel_imx
PWD :=$(shell pwd)
default:
	$(MAKE) -C $(KDIR) M= $(PWD) modules
clean: 
	rm-rf*.o.*.cmd*.ko*.mod.c.tmp_versions
