#pragma once

#include <cstddef>
#include <cstdint>
#include "arena.h"

namespace Utils {

    class Allocator {
        std::uintptr_t m_curr;
        std::uintptr_t m_end;

        public:

        Allocator() = delete;

        Allocator( const Utils::Arena &arena ) noexcept 
            : m_curr( reinterpret_cast< std::uintptr_t >( arena.data() ) ),
              m_end( m_curr + arena.size() )
        {}

        ~Allocator() = default;

        std::uintptr_t allocate( std::size_t size, std::size_t alignment ) noexcept {
            std::uintptr_t aligned =
                ( m_curr + alignment - 1 ) & ~( alignment - 1 );
            std::uintptr_t next = aligned + size; /* I don't care if we run out of memory :) */
            m_curr = next;
            return aligned;
        }

        Allocator( const Allocator &other ) = delete;
        Allocator( Allocator &&other ) = delete;
        void operator=( const Allocator &other ) = delete;
        void operator=( Allocator &&other ) = delete;
    };
}
