/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_PROCESS_UTILS_H
#define KELD_PROCESS_UTILS_H

#include <linux/kernel.h>

char *parse_argv_alloc(const char __user *const __user *argv);
char *get_pwd_path_alloc(void);

#endif
