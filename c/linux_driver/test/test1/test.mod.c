#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x9e03639, "module_layout" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x9b98d1c5, "device_create" },
	{ 0x543712b0, "__class_create" },
	{ 0xeebd13c, "__register_chrdev" },
	{ 0xf9a482f9, "msleep" },
	{ 0x15331242, "omap_iounmap" },
	{ 0xf20dabd8, "free_irq" },
	{ 0xb03e36e0, "class_destroy" },
	{ 0x97ad189a, "device_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xea147363, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "6FDA809FA2055D84281D3F8");
