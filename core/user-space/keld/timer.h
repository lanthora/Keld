/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef KELD_TIMER_H
#define KELD_TIMER_H

#include "keld/util.h"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

namespace keld {

namespace timer {

struct event {
    std::chrono::time_point<std::chrono::system_clock> time_point;
    std::function<void()> func;
};

struct compare {
    bool operator()(event a, event b) {
        return a.time_point > b.time_point;
    }
};

class timer {

public:
    int insert(const event &element);
    int start();
    int stop();

private:
    std::priority_queue<event, std::vector<event>, compare> queue_;
    bool running_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    static timer &global();
};

} // namespace timer

int start_timer();
void stop_timer();

}; // namespace keld

#endif
