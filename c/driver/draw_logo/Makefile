
#KERNEL = /mnt/hgfs/ubuntu_share/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-psp-first/
KERNEL = /home/richard/work_file/system_update/kernel/linux-dvr-rdk
obj-m +=draw_logo.o
all:
	make -C  $(KERNEL)  M=`pwd` modules
	cp draw_logo.ko  /opt/rfs_814x
	
clean:
	make -C  $(KERNEL)  M=`pwd` modules  clean
	rm -rf Module.symvers
