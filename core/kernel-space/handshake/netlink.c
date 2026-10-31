/* SPDX-License-Identifier: GPL-2.0-only */
#include "keld/netlink.h"
#include "keld/handshake.h"
#include "keld/syscall.h"
#include <net/genetlink.h>

extern struct genl_family genl_family;
struct nla_policy handshake_policy[HANDSHAKE_A_MAX + 1] = {
	[HANDSHAKE_A_STATUS_CODE] = { .type = NLA_S32 },
	[HANDSHAKE_A_SYS_SERVICE_TGID] = { .type = NLA_S32 },
};

int handshake_handler(struct sk_buff *skb, struct genl_info *info)
{
	int error = 0;
	struct sk_buff *reply = NULL;
	void *head = NULL;
	int code = 0;

	if (!netlink_capable(skb, CAP_SYS_ADMIN)) {
		ERR("netlink_capable failed");
		return -EPERM;
	}

	if (keld_heartbeat_check(info)) {
		ERR("keld_heartbeat_check failed");
		return -EPERM;
	}

	if (!info->attrs[HANDSHAKE_A_SYS_SERVICE_TGID]) {
		code = -EINVAL;
		ERR("HANDSHAKE_A_SYS_SERVICE_TGID failed");
		goto response;
	}

	tgid_init(nla_get_s32(info->attrs[HANDSHAKE_A_SYS_SERVICE_TGID]));

response:
	reply = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (unlikely(!reply)) {
		ERR("genlmsg_new failed");
		goto out_free;
	}

	head = genlmsg_put_reply(reply, info, &genl_family, 0,
				 KELD_C_HANDSHAKE);
	if (unlikely(!head)) {
		ERR("genlmsg_put_reply failed");
		goto out_free;
	}

	error = nla_put_s32(reply, HANDSHAKE_A_STATUS_CODE, code);
	if (unlikely(error)) {
		ERR("nla_put_s32 failed");
		goto out_cancel;
	}

	genlmsg_end(reply, head);

	/**
	 * genlmsg_reply releases the memory referenced by reply.
	 * Calling nlmsg_free(reply) here would double-free and crash the kernel.
	 */
	error = genlmsg_reply(reply, info);
	if (unlikely(error))
		ERR("genlmsg_reply failed");

	return 0;

out_cancel:
	genlmsg_cancel(reply, head);
out_free:
	nlmsg_free(reply);
	return 0;
}
