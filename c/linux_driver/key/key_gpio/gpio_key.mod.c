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
	{ 0x1f1e12ae, "platform_driver_unregister" },
	{ 0xba540b82, "platform_driver_register" },
	{ 0x7d11c268, "jiffies" },
	{ 0xb9e52429, "__wake_up" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x9a6221c5, "mod_timer" },
	{ 0x3bd1b1f6, "msecs_to_jiffies" },
	{ 0xc27487dd, "__bug" },
	{ 0xf6288e02, "__init_waitqueue_head" },
	{ 0x82f776b7, "gpio_export" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x7ffc8718, "gpio_set_debounce" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x74c86cc0, "init_timer_key" },
	{ 0xe34e93da, "dev_set_drvdata" },
	{ 0xdc798d37, "__mutex_init" },
	{ 0x42064352, "dev_err" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x9b98d1c5, "device_create" },
	{ 0x543712b0, "__class_create" },
	{ 0x7408ca98, "__register_chrdev" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0xf9a482f9, "msleep" },
	{ 0x8893fa5d, "finish_wait" },
	{ 0x1000e51, "schedule" },
	{ 0x75a17bed, "prepare_to_wait" },
	{ 0x5f754e5a, "memset" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0xea147363, "printk" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xb03e36e0, "class_destroy" },
	{ 0x97ad189a, "device_destroy" },
	{ 0x37a0cba, "kfree" },
	{ 0xfe990052, "gpio_free" },
	{ 0xd8f795ca, "del_timer" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
	{ 0x1d99db95, "dev_get_drvdata" },
	{ 0x15331242, "omap_iounmap" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

