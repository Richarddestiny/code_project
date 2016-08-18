
#KERNEL = /mnt/hgfs/ubuntu_share/ti-ezsdk_dm814x-evm_5_05_01_04/board-support/linux-2.6.37-psp-first/
KERNEL = /home/richard/work/kernel/mini_kernel
obj-m +=pcie_gpio.o
all:
	make -C  $(KERNEL)  M=`pwd` modules
	
clean:
	make -C  $(KERNEL)  M=`pwd` modules  clean
	rm -rf Module.symvers
