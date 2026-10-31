/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_SYSCALL_H
#define KELD_SYSCALL_H

#include "keld/define.h"
#include "keld/log.h"

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

extern unsigned long *g_sys_call_table;
extern kallsyms_lookup_name_t keld_kallsyms_lookup_name;

void disable_wp(unsigned long addr);
void enable_wp(unsigned long addr);
void syscall_early_init(void);

#define KELDMAP1(cnt, m, t, a, ...) m(t, a, cnt)
#define KELDMAP2(cnt, m, t, a, ...) m(t, a, cnt), KELDMAP1(cnt##i, m, __VA_ARGS__)
#define KELDMAP3(cnt, m, t, a, ...) m(t, a, cnt), KELDMAP2(cnt##i, m, __VA_ARGS__)
#define KELDMAP4(cnt, m, t, a, ...) m(t, a, cnt), KELDMAP3(cnt##i, m, __VA_ARGS__)
#define KELDMAP5(cnt, m, t, a, ...) m(t, a, cnt), KELDMAP4(cnt##i, m, __VA_ARGS__)

#if CONFIG_SYSCALL_PTREGS
#define __HOOK_DECL(t, a, cnt) t a
#define __HOOK_ARGS(t, a, cnt) (t) SC_ARG_##cnt
#define DECL_MAP(n, ...) KELDMAP##n(i, __HOOK_DECL, __VA_ARGS__)
#define ARGS_MAP(n, ...) KELDMAP##n(i, __HOOK_ARGS, __VA_ARGS__)
#define DECL_MAP_RAW(n, ...) const struct pt_regs *regs
#define ARGS_MAP_RAW(n, ...) regs
#else
#define __HOOK_DECL(t, a, cnt) t a
#define __HOOK_ARGS(t, a, cnt) (t) a
#define DECL_MAP(n, ...) KELDMAP##n(i, __HOOK_DECL, __VA_ARGS__)
#define ARGS_MAP(n, ...) KELDMAP##n(i, __HOOK_ARGS, __VA_ARGS__)
#define DECL_MAP_RAW DECL_MAP
#define ARGS_MAP_RAW ARGS_MAP
#endif

#if KELD_USE_FTRACE
#include <linux/atomic.h>
#include <linux/ftrace.h>
#include <linux/wait.h>

struct keld_ftrace_hook {
	const char *symbol;
	void *replacement;
	unsigned long address;
	struct ftrace_ops ops;
	atomic_t active;
	wait_queue_head_t drained;
	bool registered;
};

int keld_ftrace_register(struct keld_ftrace_hook *hook, void **original);
int keld_ftrace_unregister(struct keld_ftrace_hook *hook);
void keld_ftrace_exit(struct keld_ftrace_hook *hook);

#define REG_DEFINE(name)                                                       \
	static struct keld_ftrace_hook keld_ftrace_##name = {                      \
		.symbol = "__x64_sys_" #name,                                 \
		.replacement = sys_##name##_hook,                             \
	};                                                                     \
	static int __hook_##name(void)                                         \
	{                                                                      \
		if (KELD_NR_##name == KELD_NR_UNDEFINED)                         \
			return -ENOSYS;                                        \
		return keld_ftrace_register(&keld_ftrace_##name,                  \
					 (void **)&keld_sys_##name);              \
	}

#define UNREG_DEFINE(name)                                                     \
	static int __unhook_##name(void)                                       \
	{                                                                      \
		return keld_ftrace_unregister(&keld_ftrace_##name);                \
	}

#define HOOK_LEAVE(name) keld_ftrace_exit(&keld_ftrace_##name)
#else
#define HOOK_LEAVE(name) do { } while (0)

#define SYSCALL_UPDATE(nr, func) g_sys_call_table[nr] = (unsigned long)func
#define SYSCALL_BACKUP(func, nr) func = (void *)g_sys_call_table[nr]

#define REG_DEFINE(name)                                                       \
	static bool syscall_##name##_registered;                              \
	static int __hook_##name(void)                                         \
	{                                                                      \
		unsigned long flags;                                           \
		if (syscall_##name##_registered)                               \
			return 0;                                              \
		if (KELD_NR_##name == KELD_NR_UNDEFINED) {                         \
			ERR("undefined system call: " STR(name));              \
			return -ENOSYS;                                        \
		}                                                              \
		if (!g_sys_call_table) {                                       \
			ERR("g_sys_call_table is not initialized");            \
			return -EPERM;                                         \
		}                                                              \
                                                                               \
		if (!keld_sys_##name) {                                          \
			SYSCALL_BACKUP(keld_sys_##name, KELD_NR_##name);           \
		}                                                              \
                                                                               \
		local_irq_save(flags);                                         \
		disable_wp((unsigned long)(g_sys_call_table + KELD_NR_##name));  \
		SYSCALL_UPDATE(KELD_NR_##name, &sys_##name##_hook);              \
		wmb();                                                         \
		enable_wp((unsigned long)(g_sys_call_table + KELD_NR_##name));   \
		local_irq_restore(flags);                                      \
		syscall_##name##_registered = true;                            \
		return 0;                                                      \
	}

#define UNREG_DEFINE(name)                                                     \
	static int __unhook_##name(void)                                       \
	{                                                                      \
		unsigned long flags;                                           \
		if (!syscall_##name##_registered)                              \
			return 0;                                              \
		if (KELD_NR_##name == KELD_NR_UNDEFINED) {                         \
			return -ENOSYS;                                        \
		}                                                              \
		if (!g_sys_call_table) {                                       \
			return -EPERM;                                         \
		}                                                              \
                                                                               \
		if (!keld_sys_##name) {                                          \
			return -EPERM;                                         \
		}                                                              \
                                                                               \
		local_irq_save(flags);                                         \
		disable_wp((unsigned long)(g_sys_call_table + KELD_NR_##name));  \
		SYSCALL_UPDATE(KELD_NR_##name, keld_sys_##name);                   \
		wmb();                                                         \
		enable_wp((unsigned long)(g_sys_call_table + KELD_NR_##name));   \
		local_irq_restore(flags);                                      \
		syscall_##name##_registered = false;                           \
		return 0;                                                      \
	}
#endif /* KELD_USE_FTRACE */

#define HOOK_DEFINEx(x, name, ...)                                             \
	static long __sys_##name##_hook(DECL_MAP(x, __VA_ARGS__));             \
	noinline long sys_##name##_hook(DECL_MAP_RAW(x, __VA_ARGS__));         \
	long (*keld_sys_##name)(DECL_MAP_RAW(x, __VA_ARGS__));                   \
	REG_DEFINE(name)                                                       \
	UNREG_DEFINE(name)                                                     \
	noinline long sys_##name##_hook(DECL_MAP_RAW(x, __VA_ARGS__))          \
	{                                                                      \
		long retval;                                                   \
		retval = __sys_##name##_hook(ARGS_MAP(x, __VA_ARGS__));        \
		if (!retval)                                                   \
			retval = keld_sys_##name(ARGS_MAP_RAW(x, __VA_ARGS__));  \
		HOOK_LEAVE(name);                                              \
		return retval;                                                 \
	}                                                                      \
	static long __sys_##name##_hook(DECL_MAP(x, __VA_ARGS__))

#define HOOK_DEFINE1(name, ...) HOOK_DEFINEx(1, name, __VA_ARGS__)
#define HOOK_DEFINE2(name, ...) HOOK_DEFINEx(2, name, __VA_ARGS__)
#define HOOK_DEFINE3(name, ...) HOOK_DEFINEx(3, name, __VA_ARGS__)
#define HOOK_DEFINE4(name, ...) HOOK_DEFINEx(4, name, __VA_ARGS__)
#define HOOK_DEFINE5(name, ...) HOOK_DEFINEx(5, name, __VA_ARGS__)

#ifndef STR
#define STR(x) #x
#endif

#ifndef REG_HOOK
#define REG_HOOK(name)                                                         \
	do {                                                                   \
		int hook_error = __hook_##name();                              \
		if (hook_error)                                                \
			ERR("hook register name=[%s] code=[%d]", STR(name),      \
			    hook_error);                                       \
		else                                                           \
			DBG("hook register name=[%s] code=[0]", STR(name));      \
	} while (0)
#endif

#ifndef UNREG_HOOK
#define UNREG_HOOK(name)                                                       \
	do {                                                                   \
		int hook_error = __unhook_##name();                            \
		if (hook_error)                                                \
			ERR("hook unregister name=[%s] code=[%d]", STR(name),    \
			    hook_error);                                       \
		else                                                           \
			DBG("hook unregister name=[%s] code=[0]", STR(name));    \
	} while (0)
#endif

#if defined(CONFIG_X86_64)
#define SC_ARG_i (regs->di)
#define SC_ARG_ii (regs->si)
#define SC_ARG_iii (regs->dx)
#define SC_ARG_iiii (regs->r10)
#define SC_ARG_iiiii (regs->r8)
#endif

#if defined(CONFIG_ARM64)
#define SC_ARG_i (regs->regs[0])
#define SC_ARG_ii (regs->regs[1])
#define SC_ARG_iii (regs->regs[2])
#define SC_ARG_iiii (regs->regs[3])
#define SC_ARG_iiiii (regs->regs[4])
#endif

#endif
