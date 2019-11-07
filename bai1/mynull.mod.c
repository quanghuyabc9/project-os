#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x5fe7d95d, "module_layout" },
	{ 0x52e04d4f, "cdev_del" },
	{ 0x758708d1, "device_destroy" },
	{ 0x1a52faba, "cdev_add" },
	{ 0x450351e, "cdev_init" },
	{ 0x4920100f, "class_destroy" },
	{ 0x77facd38, "device_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x4b9e70ce, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xb44ad4b3, "_copy_to_user" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x79aa04a2, "get_random_bytes" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

