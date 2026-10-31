/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef PROCESS_AUDIT_H
#define PROCESS_AUDIT_H

#include "keld/broadcaster.h"
#include "keld/lru.h"
#include "keld/process.h"
#include <chrono>
#include <functional>
#include <shared_mutex>
#include <string>
#include <unordered_set>

namespace keld {

int start_process_protector();

struct process_cmd_ctx {
    std::string workdir;
    std::string binary;
    std::string argv;
};
}; // namespace keld

namespace std {
template <> struct hash<keld::process_cmd_ctx> {
    std::size_t operator()(const keld::process_cmd_ctx &ctx) const {
        std::size_t a = std::hash<std::string>()(ctx.workdir);
        std::size_t b = std::hash<std::string>()(ctx.binary) << 1;
        std::size_t c = std::hash<std::string>()(ctx.argv) >> 1;
        return (a) ^ (b) ^ (c);
    }
};
}; // namespace std

namespace keld {

class process_protector {

public:
    proc_perm handle_new_cmd(const process_cmd_ctx &cmd);
    int init();
    int start();

private:
    int report(const process_cmd_ctx &cmd);
    int insert_trusted_cmd(const process_cmd_ctx &cmd);
    int delete_trusted_cmd(const process_cmd_ctx &cmd);
    int clear_trusted_cmd();
    bool is_trusted(const process_cmd_ctx &cmd);
    bool handle_process_msg(const std::string &msg);

public:
    static process_protector &global();

private:
    std::unordered_set<process_cmd_ctx> trusted_;
    std::shared_mutex mutex_;
    proc_perm judge_ = PROCESS_ACCEPT;
    bool enabled_ = false;
    std::shared_ptr<audience> audience_ = nullptr;
};

}; // namespace keld

#endif
