#pragma once

#include "utils/job.h"
#include <atomic>
#include <cstddef>
#include <atomic>

namespace Connections {
    static constexpr int g_cacheLineSize = 64;

    template < std::size_t N = 256 >
    struct SpmcQueue {
        std::array< Utils::Job, N > buff;

        alignas( g_cacheLineSize ) std::atomic< std::size_t > head{};
        alignas( g_cacheLineSize ) std::atomic< std::size_t > tail{};


        void push( Utils::Job &&job ) noexcept {
            std::size_t newValue, tailValue, oldValue;
            do {
                oldValue = head.load( std::memory_order_relaxed );
                newValue = ( oldValue + 1 ) & ( N - 1 );
                tailValue = tail.load( std::memory_order_acquire );
            } while ( newValue == tailValue );
            buff[ static_cast< std::size_t >( oldValue ) ] = std::move( job );
            head.store( newValue, std::memory_order_release );
        }

        [[nodiscard]] Utils::Job &&pop() noexcept {
            std::size_t oldValue, newValue, headValue;

            do {
                oldValue = tail.load( std::memory_order_acquire );
                headValue = head.load( std::memory_order_acquire );
                newValue = (oldValue + 1) & ( N - 1 );
            } while ( oldValue == headValue || !tail.compare_exchange_weak( oldValue, newValue, std::memory_order_acq_rel ) );
            return std::move( buff[ oldValue ] );
        }
    };
}
