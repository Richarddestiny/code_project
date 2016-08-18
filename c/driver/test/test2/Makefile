
#KERNEL = /mnt/hgfs/ubuntu_share/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-psp-first/
KERNEL = /home/zz/work_file/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-for-work
obj-m +=led.o
all:
	make -C  $(KERNEL)  M=`pwd` modules
	rm /opt/rfs_814x/led.ko
	cp led.ko  /opt/rfs_814x
	
clean:
	make -C  $(KERNEL)  M=`pwd` modules  clean
	rm -rf Module.symvers
