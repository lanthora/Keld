/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_UTIL_H
#define KELD_UTIL_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

EXTERN_C_BEGIN

#define WARN(fmt, arg...)                                                                                              \
    do {                                                                                                               \
        time_t now = time(NULL);                                                                                       \
        struct tm *t = localtime(&now);                                                                                \
        fprintf(stdout, "[%04d-%02d-%02d %02d:%02d:%02d] [%s:%d] " fmt "\n", t->tm_year + 1900, t->tm_mon + 1,         \
                t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, __FILE__, __LINE__, ##arg);                              \
        fflush(stdout);                                                                                                \
    } while (0)

#define INFO WARN

#if defined(DEBUG)
#define DBG WARN
#else
#define DBG(fmt, arg...)
#endif

#define ERR(fmt, arg...)                                                                                               \
    do {                                                                                                               \
        time_t now = time(NULL);                                                                                       \
        struct tm *t = localtime(&now);                                                                                \
        fprintf(stderr, "[%04d-%02d-%02d %02d:%02d:%02d] [%s:%d] " fmt "\n", t->tm_year + 1900, t->tm_mon + 1,         \
                t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, __FILE__, __LINE__, ##arg);                              \
        fflush(stderr);                                                                                                \
    } while (0)

#define update_thread_name(name)                                                                                       \
    do {                                                                                                               \
        pthread_setname_np(pthread_self(), name);                                                                      \
    } while (0)

int insert_kernel_module(const char *filename);
int remove_kernel_module(const char *modulename);

enum {
    KELD_SUCCESS,
    KELD_SIG,
    KELD_NETLINK_INIT,
    KELD_HEARTBEAT,
    KELD_NETLINK_WAIT,
    KELD_UNIX_DOMAIN_SOCKET,
    KELD_BAD_AUDIENCE,
};

void shutdown_service(int code);

bool current_service_status();

EXTERN_C_END

#endif
