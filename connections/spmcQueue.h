#pragma once

#include "utils/job.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <utility>

namespace Connections {

static constexpr std::size_t g_cacheLineSize = 64;

template <std::size_t N = 4096>
struct SpmcQueue {
    static_assert((N & (N - 1)) == 0);

    static constexpr std::size_t MASK = N - 1;

    alignas(g_cacheLineSize)
    std::array<Utils::Job, N> buff;

    // Producer only writes head
    alignas(g_cacheLineSize)
    std::atomic<std::size_t> head{0};

    // Consumers only write tail
    alignas(g_cacheLineSize)
    std::atomic<std::size_t> tail{0};


    void push(Utils::Job&& job) noexcept {
        std::size_t headValue;
        std::size_t next;

        for (;;) {
            headValue = head.load(std::memory_order_relaxed);
            next = (headValue + 1) & MASK;

            if (next != tail.load(std::memory_order_acquire))
                break;
        }

        buff[headValue] = std::move(job);

        head.store(next, std::memory_order_release);
    }


    [[nodiscard]] Utils::Job&& pop() noexcept {
        std::size_t tailValue;
        std::size_t next;

        for (;;) {
            tailValue = tail.load(std::memory_order_relaxed);
            next = (tailValue + 1) & MASK;

            if (tailValue != head.load(std::memory_order_acquire) &&
                tail.compare_exchange_weak(
                    tailValue,
                    next,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed))
            {
                break;
            }

            // spin
        }

        return std::move(buff[tailValue]);
    }
};

}
