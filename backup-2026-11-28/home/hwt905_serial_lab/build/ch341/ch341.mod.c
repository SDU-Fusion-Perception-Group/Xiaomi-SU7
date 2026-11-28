#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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



static const char ____versions[]
__used __section("__versions") =
	"\x20\x00\x00\x00\x0b\x05\xdb\x34"
	"_raw_spin_lock_irqsave\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x24\x00\x00\x00\xec\x32\x96\xde"
	"usb_serial_register_drivers\0"
	"\x24\x00\x00\x00\x91\x9e\x0c\x4b"
	"usb_serial_generic_close\0\0\0\0"
	"\x18\x00\x00\x00\xb4\x1d\x7d\xb8"
	"usb_kill_urb\0\0\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x28\x00\x00\x00\x36\x79\x56\x57"
	"usb_serial_deregister_drivers\0\0\0"
	"\x18\x00\x00\x00\x36\xbb\xc0\xdc"
	"usb_submit_urb\0\0"
	"\x14\x00\x00\x00\x8a\xb5\x2e\xa6"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\x44\x43\x96\xe2"
	"__wake_up\0\0\0"
	"\x1c\x00\x00\x00\x26\x34\x72\x99"
	"tty_port_tty_get\0\0\0\0"
	"\x28\x00\x00\x00\xed\x65\x61\xca"
	"usb_serial_handle_dcd_change\0\0\0\0"
	"\x18\x00\x00\x00\x1d\x1f\x4f\x73"
	"tty_kref_put\0\0\0\0"
	"\x20\x00\x00\x00\x5c\xa5\x2f\xa7"
	"usb_control_msg_recv\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x18\x00\x00\x00\xc1\x03\xaa\xdc"
	"usb_control_msg\0"
	"\x10\x00\x00\x00\xa6\x50\xba\x15"
	"jiffies\0"
	"\x28\x00\x00\x00\xb4\x96\x44\x05"
	"schedule_timeout_interruptible\0\0"
	"\x18\x00\x00\x00\xc2\x08\x4c\xf2"
	"tty_put_char\0\0\0\0"
	"\x24\x00\x00\x00\xb3\x83\xf9\x16"
	"usb_serial_generic_resume\0\0\0"
	"\x18\x00\x00\x00\xc7\xa2\xd5\x42"
	"kmalloc_caches\0\0"
	"\x18\x00\x00\x00\x4c\x72\x3c\xa2"
	"kmalloc_trace\0\0\0"
	"\x14\x00\x00\x00\x38\x3c\xe8\xdd"
	"_dev_info\0\0\0"
	"\x20\x00\x00\x00\xa3\x13\x9c\xcd"
	"tty_termios_hw_change\0\0\0"
	"\x20\x00\x00\x00\xd8\x94\xd3\x0b"
	"tty_termios_baud_rate\0\0\0"
	"\x1c\x00\x00\x00\x8f\x04\x16\xe8"
	"tty_termios_copy_hw\0"
	"\x20\x00\x00\x00\x28\x6d\x15\x94"
	"usb_serial_generic_open\0"
	"\x28\x00\x00\x00\x63\xf8\x7e\x9c"
	"usb_serial_generic_tiocmiwait\0\0\0"
	"\x18\x00\x00\x00\x8a\x10\xd2\xe6"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "usbserial");

MODULE_ALIAS("usb:v1A86p5523d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v1A86p7522d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v1A86p7523d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v2184p0057d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v4348p5523d*dc*dsc*dp*ic*isc*ip*in*");
MODULE_ALIAS("usb:v9986p7523d*dc*dsc*dp*ic*isc*ip*in*");
