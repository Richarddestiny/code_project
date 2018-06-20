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
	{ 0x7a0cf9e5, "module_layout" },
	{ 0xd8f795ca, "del_timer" },
	{ 0x7d11c268, "jiffies" },
	{ 0x8cfc414e, "add_timer" },
	{ 0x74c86cc0, "init_timer_key" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xea147363, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "A27BA6049B0639AFC7D3FF0");
