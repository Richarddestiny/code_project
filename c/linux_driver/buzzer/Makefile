
#KERNEL = /mnt/hgfs/ubuntu_share/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-psp-first/
KERNEL = /home/zz/work_file/linux-2.6.37-for-work
obj-m +=buzzer.o
all:
	make -C  $(KERNEL)  M=`pwd` modules
	cp buzzer.ko  /opt/filesystem
	
clean:
	make -C  $(KERNEL)  M=`pwd` modules  clean
	rm -rf Module.symvers
