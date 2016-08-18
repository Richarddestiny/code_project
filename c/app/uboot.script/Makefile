all:

	./mkimage  -A ARM -O linux -T script  -C none  -a 0x80900000 -e 0x80900000  -n "boot script"  -d ./boot.script  boot.scr
	cp boot.scr /tftpboot

