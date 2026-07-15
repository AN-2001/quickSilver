#pragma once

#include "metricsEvents.h"

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace Connections {

template <std::size_t N = 1024>
struct MpscQueue {
    static_assert((N & (N - 1)) == 0, "N must be a power of two");

    std::array<MetricsEvent, N> buff;

    std::size_t head = 0;
    std::size_t tail = 0;

    MetricsEvent slot;

    std::mutex mutex;
    std::condition_variable cv;

    void push(MetricsEvent&& event) noexcept
    {
        {
            std::lock_guard lock(mutex);

            const auto next = (head + 1) & (N - 1);

            // Full queue: preserve non-blocking producer behavior
            if (next == tail)
                return;

            buff[head] = std::move(event);
            head = next;
        }

        cv.notify_one();
    }

    [[nodiscard]] MetricsEvent&& pop() noexcept
    {
        std::unique_lock lock(mutex);

        cv.wait(lock, [&] {
            return tail != head;
        });

        slot = std::move(buff[tail]);
        tail = (tail + 1) & (N - 1);

        return std::move(slot);
    }
};

}
