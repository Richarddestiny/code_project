
#KERNEL = /mnt/hgfs/ubuntu_share/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-psp-first/
KERNEL = /home/zz/work_file/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-for-work
obj-m +=gprs_c5300.o
all:
	make -C  $(KERNEL)  M=`pwd` modules
	rm /opt/filesystem/gprs_c5300.ko
	cp gprs_c5300.ko  /opt/filesystem
	
clean:
	make -C  $(KERNEL)  M=`pwd` modules  clean
	rm -rf Module.symvers
