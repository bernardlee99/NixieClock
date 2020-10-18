obj-m += ws2812b.o

all:
	make ARCH=arm CROSS_COMPILE=/home/bbd/Desktop/NUC970_Buildroot/output/host/usr/bin/arm-linux- -C /home/bbd/Desktop/NUC970_Buildroot/output/build/linux-master SUBDIRS=$(PWD) modules
clean:
	make -C /home/bbd/Desktop/NUC970_Buildroot/output/build/linux-master SUBDIRS=$(PWD) clean
