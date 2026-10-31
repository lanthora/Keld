/* SPDX-License-Identifier: GPL-2.0-only */
#include "keld/file.h"
#include "keld/log.h"
#include "keld/net.h"
#include "keld/netlink.h"
#include "keld/process.h"
#include "keld/syscall.h"
#include "keld/watchdog.h"
#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("Keld host intrusion detection and prevention system");

static int init(void)
{
	INFO("keld 0.1.0 loaded");
	syscall_early_init();
	process_protect_init();
	file_protect_init();
	net_protect_init();
	conn_check_init();
	netlink_kernel_start();
	return 0;
}

static void cleanup(void)
{
	netlink_kernel_stop();
	conn_check_destory();
	process_protect_destory();
	file_protect_destory();
	net_protect_destory();
	INFO("keld exit");
	return;
}

module_init(init);
module_exit(cleanup);
