/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_PROCESS_H
#define KELD_PROCESS_H

#include "keld/util.h"
#include "process/define.h"
#include <netlink/genl/mngt.h>
#include <string>

namespace keld {

typedef int proc_perm_id;
typedef int32_t proc_perm;

enum { PROCESS_PROTECT_UNSPEC, PROCESS_PROTECT_REPORT, PROCESS_PROTECT_ENABLE, PROCESS_PROTECT_DISABLE };

#define PROCESS_INVAILD -1
#define PROCESS_WATT 0
#define PROCESS_ACCEPT 1
#define PROCESS_REJECT 2

int handle_genl_process_protection(struct nl_cache_ops *unused, struct genl_cmd *genl_cmd, struct genl_info *genl_info,
                                   void *arg);

int enable_process_protection(int32_t session);
int disable_process_protection(int32_t session);

proc_perm check_process_permission(const std::string &workdir, const std::string &binary, const std::string &argv);
int reply_process_permission(proc_perm_id id, proc_perm perm);

}; // namespace keld

#endif
