/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_NETLINK_H
#define KELD_NETLINK_H

#include <net/genetlink.h>

#define KELD_FAMLY_NAME "KELD"
#define KELD_FAMLY_VERSION 1

enum {
	KELD_C_UNSPEC,
	KELD_C_HANDSHAKE,
	KELD_C_PROCESS_PROTECT,
	KELD_C_FILE_PROTECT,
	KELD_C_NET_PROTECT,
	__KELD_C_MAX,
};
#define KELD_C_MAX (__KELD_C_MAX - 1)

void netlink_kernel_start(void);
void netlink_kernel_stop(void);

extern u32 keld_portid;
extern struct net *keld_net;

int handshake_handler(struct sk_buff *skb, struct genl_info *info);

#endif
