/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_HANDSHAKE_H
#define KELD_HANDSHAKE_H

#include <net/genetlink.h>

enum {
	HANDSHAKE_A_UNSPEC,
	HANDSHAKE_A_STATUS_CODE,
	HANDSHAKE_A_SYS_SERVICE_TGID,
	__HANDSHAKE_A_MAX,
};
#define HANDSHAKE_A_MAX (__HANDSHAKE_A_MAX - 1)

inline void tgid_init(pid_t pid);
int keld_heartbeat_check(struct genl_info *info);
int keld_user_check(struct genl_info *info);
bool keld_trusted_proccess(void);

#endif
