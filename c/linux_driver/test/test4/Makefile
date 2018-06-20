
#KERNEL = /mnt/hgfs/ubuntu_share/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-psp-first/
KERNEL = /home/richard/work_file/kernel/mini_kernel
obj-m +=reset.o
all:
	make -C  $(KERNEL)  M=`pwd` modules
	cp reset.ko  /opt/rfs_814x
	
clean:
	make -C  $(KERNEL)  M=`pwd` modules  clean
	rm -rf Module.symvers
