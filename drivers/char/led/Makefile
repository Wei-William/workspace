KERNELDIR:=/home/linux/code/iTop_kernel
PWD:=$(shell pwd)
all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
obj-m:=led.o
