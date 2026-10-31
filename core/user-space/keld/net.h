/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_NET_H
#define KELD_NET_H

#include "keld/util.h"
#include "net/define.h"
#include <netlink/genl/mngt.h>

namespace keld {

typedef uint32_t net_addr;
typedef uint16_t net_port;
typedef uint8_t net_protocol;
typedef uint32_t net_response;
typedef uint32_t net_policy_id;
typedef int8_t net_priority;

#define NET_POLICY_DROP 0
#define NET_POLICY_ACCEPT 1

enum {
    NET_PROTECT_UNSPEC,
    NET_PROTECT_ENABLE,
    NET_PROTECT_DISABLE,
    NET_PROTECT_INSERT,
    NET_PROTECT_DELETE,
    NET_PROTECT_CLEAR,
    NET_PROTECT_REPORT,
};

int handle_genl_net_protection(struct nl_cache_ops *unused, struct genl_cmd *genl_cmd, struct genl_info *genl_info,
                               void *arg);

// For equal priorities, newer policies match first. Policies may share an ID.
// Delete policies in batches by ID. All values use host byte order.
struct net_policy {
    net_policy_id id;
    net_priority priority;

    struct {
        struct {
            net_addr begin;
            net_addr end;
        } src;
        struct {
            net_addr begin;
            net_addr end;
        } dst;
    } addr;

    struct {
        struct {
            net_port begin;
            net_port end;
        } src;
        struct {
            net_port begin;
            net_port end;
        } dst;
    } port;

    struct {
        net_protocol begin;
        net_protocol end;
    } protocol;

    net_response response;
    int flags;
};

int enable_net_protection(int32_t session);
int disable_net_protection(int32_t session);
int insert_net_policy(int32_t session, const struct net_policy *policy);
int delete_net_policy(int32_t session, net_policy_id id);
int clear_net_policy(int32_t session);

#define FLAG_NET_INBOUND (0b00000001)
#define FLAG_NET_OUTBOUND (0b00000010)
#define FLAG_NET_ONLY_CHECK_NEW_TCP (0b00000100)
#define FLAG_NET_ONLY_ALLOW_TCP_HEADER (0b000001000)

}; // namespace keld

#endif
