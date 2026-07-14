#pragma once

#include "utils/job.h"
#include <atomic>
#include <cstddef>
#include <atomic>

namespace Connections {
    static constexpr int g_cacheLineSize = 64;

    template < std::size_t N = 256 >
    struct MpscQueue {
        alignas( g_cacheLineSize ) std::array< int, N > buff;
        alignas( g_cacheLineSize ) std::atomic< std::size_t > head{};
        alignas( g_cacheLineSize ) std::atomic< std::size_t > tail{};

        struct alignas( g_cacheLineSize ) slot 
        {
            int val;
        } m_slot;


        void push( int &&job ) noexcept {
            std::size_t oldValue, newValue, tailValue;

            do {
                oldValue = head.load( std::memory_order_acquire );
                tailValue = tail.load( std::memory_order_acquire );
                newValue = (oldValue + 1) & ( N - 1 );
            } while ( newValue == tailValue || !head.compare_exchange_weak( oldValue, newValue, std::memory_order_acq_rel ) );

            buff[ oldValue ] = std::move( job );
            head.store( newValue, std::memory_order_release );
        }

        [[nodiscard]] int &&pop() noexcept {
            std::size_t headValue, oldValue, newValue;
            do {
                oldValue = tail.load( std::memory_order_relaxed );
                headValue = head.load( std::memory_order_acquire );
                newValue = (oldValue + 1) & ( N - 1 );
            } while ( oldValue == headValue );
            m_slot.val = std::move( buff[ oldValue ] );
            tail.store( newValue, std::memory_order_release );
            return std::move( m_slot.val );
        }
    };
}
