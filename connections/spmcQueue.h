#pragma once

#include "utils/job.h"
#include <atomic>
#include <chrono>
#include <cstddef>
#include <atomic>

namespace Connections {

    template < std::size_t N = 1024 >
    struct SpmcQueue {
        static constexpr int g_cacheLineSize = 64;

        alignas( g_cacheLineSize ) std::array< Utils::Job, N > buff;
        alignas( g_cacheLineSize ) std::atomic< std::size_t > head{};
        alignas( g_cacheLineSize ) std::atomic< std::size_t > tail{};

        [[nodiscard]] std::size_t size() const noexcept {
            std::size_t h = head.load( std::memory_order_relaxed );
            std::size_t t = head.load( std::memory_order_relaxed );
            return ( h - t ) & ( N - 1 );
        }

        void push( Utils::Job &&job ) noexcept {
            std::size_t newValue, tailValue, oldValue;
            do {
                oldValue = head.load( std::memory_order_relaxed );
                newValue = ( oldValue + 1 ) & ( N - 1 );
                tailValue = tail.load( std::memory_order_acquire );
            } while ( newValue == tailValue );
            job.m_acceptTime = std::chrono::steady_clock::now();
            buff[ oldValue ] = std::move( job );
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
