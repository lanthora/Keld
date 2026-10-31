/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_LOG_H
#define KELD_LOG_H

#define ERR(fmt, arg...)                                                       \
	do {                                                                   \
		printk(KERN_ERR "keld: %s:%d " fmt "\n", __FILE__,        \
		       __LINE__, ##arg);                                       \
	} while (0)

#define INFO(fmt, arg...)                                                      \
	do {                                                                   \
		printk(KERN_INFO "keld: %s:%d " fmt "\n", __FILE__,       \
		       __LINE__, ##arg);                                       \
	} while (0)

#ifdef DEBUG
#define DBG(fmt, arg...)                                                       \
	do {                                                                   \
		printk(KERN_DEBUG "keld: %s:%d " fmt "\n", __FILE__,      \
		       __LINE__, ##arg);                                       \
	} while (0)
#else
#define DBG(fmt, arg...) do { } while (0)
#endif

#endif
