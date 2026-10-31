/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_THREADS_H
#define KELD_THREADS_H

#include <functional>
#include <list>
#include <thread>

namespace keld {

class thread_manager {

public:
    void wait_thread_exit();
    void create_thread(std::function<void(void)> &&t);

public:
    static thread_manager &global();

private:
    std::list<std::thread> threads_;
};

void wait_thread_exit();
void create_thread(std::function<void(void)> &&t);

} // namespace keld

#endif
