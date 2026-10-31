/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELDNL_NETLINK_H
#define KELDNL_NETLINK_H

#include "keld/util.h"
#include <netlink/msg.h>
#include <stdint.h>

EXTERN_C_BEGIN

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

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

void init_netlink_server(void);
int start_netlink(void);
int stop_netlink(void);

struct nl_msg *alloc_keld_nlmsg(uint8_t cmd);
int send_free_keld_nlmsg(struct nl_msg *message);

EXTERN_C_END

#endif
