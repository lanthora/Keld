/* SPDX-License-Identifier: GPL-2.0-only */
#include "keld/handshake.h"
#include "keld/syscall.h"
#include "keld/watchdog.h"

pid_t keld_tgid;
extern struct net *keld_net;
extern u32 keld_portid;

int keld_user_check(struct genl_info *info)
{
	if (info->snd_portid != keld_portid)
		return -EPERM;

	if (genl_info_net(info) != keld_net)
		return -EPERM;
	return 0;
}

int keld_heartbeat_check(struct genl_info *info)
{
	if (!conn_check_living())
		goto update;

	if (keld_user_check(info))
		return -EPERM;

update:
	keld_net = genl_info_net(info);
	keld_portid = info->snd_portid;
	conn_check_set_alive();
	return 0;
}

bool keld_trusted_proccess(void)
{
	return current->tgid == keld_tgid;
}

inline void tgid_init(pid_t pid)
{
	keld_tgid = pid;
}
