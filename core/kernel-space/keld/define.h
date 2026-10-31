/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_DEFINE_H
#define KELD_DEFINE_H

#include <generated/autoconf.h>
#include <linux/syscalls.h>
#include <linux/version.h>

/*
 * x86 stopped dispatching through sys_call_table in v6.9. The change was
 * also backported to these stable branches. Distributions with different
 * backports can override this at build time with -DKELD_USE_FTRACE=0/1.
 * ARM and ARM64 retain the existing syscall-table backend.
 */
#ifndef KELD_USE_FTRACE
#if defined(CONFIG_X86_64) &&                                            \
	((LINUX_VERSION_CODE >= KERNEL_VERSION(6, 9, 0)) ||               \
	 (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 5) &&                \
	  LINUX_VERSION_CODE < KERNEL_VERSION(6, 9, 0)) ||                \
	 (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 26) &&               \
	  LINUX_VERSION_CODE < KERNEL_VERSION(6, 7, 0)) ||                \
	 (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 85) &&               \
	  LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)) ||                \
	 (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 154) &&             \
	  LINUX_VERSION_CODE < KERNEL_VERSION(5, 16, 0)))
#define KELD_USE_FTRACE 1
#else
#define KELD_USE_FTRACE 0
#endif
#endif

#if KELD_USE_FTRACE && !defined(CONFIG_X86_64)
#error "The ftrace syscall backend currently supports only x86_64"
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
#define CONFIG_NLA_STRSCPY 1
#endif

/**
 * CONFIG_SYSCALL_PTREGS indicates whether syscall arguments use a register wrapper.
 * Set the macro to 1 on systems known to use wrapped arguments.
 */
#if defined(CONFIG_X86_64) || defined(CONFIG_ARM64)
#define CONFIG_SYSCALL_PTREGS 1
#endif

#ifndef CONFIG_NLA_STRSCPY
#define CONFIG_NLA_STRSCPY 0
#endif

#ifndef CONFIG_SYSCALL_PTREGS
#define CONFIG_SYSCALL_PTREGS 0
#endif

#define KELD_NR_UNDEFINED -1

#if defined(__NR_open)
#define KELD_NR_open __NR_open
#else
#define KELD_NR_open KELD_NR_UNDEFINED
#endif

#if defined(__NR_openat)
#define KELD_NR_openat __NR_openat
#else
#define KELD_NR_openat KELD_NR_UNDEFINED
#endif

#if defined(__NR_openat)
#define KELD_NR_openat __NR_openat
#else
#define KELD_NR_openat KELD_NR_UNDEFINED
#endif

#if defined(__NR_rename)
#define KELD_NR_rename __NR_rename
#else
#define KELD_NR_rename KELD_NR_UNDEFINED
#endif

#if defined(__NR_renameat)
#define KELD_NR_renameat __NR_renameat
#else
#define KELD_NR_renameat KELD_NR_UNDEFINED
#endif

#if defined(__NR_renameat2)
#define KELD_NR_renameat2 __NR_renameat2
#else
#define KELD_NR_renameat2 KELD_NR_UNDEFINED
#endif

#if defined(__NR_mkdir)
#define KELD_NR_mkdir __NR_mkdir
#else
#define KELD_NR_mkdir KELD_NR_UNDEFINED
#endif

#if defined(__NR_mkdirat)
#define KELD_NR_mkdirat __NR_mkdirat
#else
#define KELD_NR_mkdirat KELD_NR_UNDEFINED
#endif

#if defined(__NR_rmdir)
#define KELD_NR_rmdir __NR_rmdir
#else
#define KELD_NR_rmdir KELD_NR_UNDEFINED
#endif

#if defined(__NR_link)
#define KELD_NR_link __NR_link
#else
#define KELD_NR_link KELD_NR_UNDEFINED
#endif

#if defined(__NR_linkat)
#define KELD_NR_linkat __NR_linkat
#else
#define KELD_NR_linkat KELD_NR_UNDEFINED
#endif

#if defined(__NR_unlink)
#define KELD_NR_unlink __NR_unlink
#else
#define KELD_NR_unlink KELD_NR_UNDEFINED
#endif

#if defined(__NR_unlinkat)
#define KELD_NR_unlinkat __NR_unlinkat
#else
#define KELD_NR_unlinkat KELD_NR_UNDEFINED
#endif

#if defined(__NR_symlink)
#define KELD_NR_symlink __NR_symlink
#else
#define KELD_NR_symlink KELD_NR_UNDEFINED
#endif

#if defined(__NR_symlinkat)
#define KELD_NR_symlinkat __NR_symlinkat
#else
#define KELD_NR_symlinkat KELD_NR_UNDEFINED
#endif

#if defined(__NR_mknod)
#define KELD_NR_mknod __NR_mknod
#else
#define KELD_NR_mknod KELD_NR_UNDEFINED
#endif

#if defined(__NR_mknodat)
#define KELD_NR_mknodat __NR_mknodat
#else
#define KELD_NR_mknodat KELD_NR_UNDEFINED
#endif

#if defined(__NR_execve)
#define KELD_NR_execve __NR_execve
#else
#define KELD_NR_execve KELD_NR_UNDEFINED
#endif

#if defined(__NR_execveat)
#define KELD_NR_execveat __NR_execveat
#else
#define KELD_NR_execveat KELD_NR_UNDEFINED
#endif

#if defined(__NR_kill)
#define KELD_NR_kill __NR_kill
#else
#define KELD_NR_kill KELD_NR_UNDEFINED
#endif

#if defined(__NR_delete_module)
#define KELD_NR_delete_module __NR_delete_module
#else
#define KELD_NR_delete_module KELD_NR_UNDEFINED
#endif

#endif
