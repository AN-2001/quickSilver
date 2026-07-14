#pragma once

#include <cstddef>
#include <cstdint>
#include "arena.h"

namespace Utils {

    class Allocator {
        std::byte *m_curr;
        std::byte *m_end;

        public:

        Allocator() = delete;

        Allocator( const Utils::Arena &arena ) noexcept 
            : m_curr( static_cast< std::byte * >( arena.data() ) ),
              m_end( m_curr + arena.size() )
        {}

        ~Allocator() = default;

        std::byte *allocate( std::size_t size, std::size_t alignment ) noexcept {
            auto curr = reinterpret_cast< std::uintptr_t >( m_curr );
            auto aligned =
                ( curr + alignment - 1 ) & ~( alignment - 1 );
            auto next = aligned + size; /* I don't care if we run out of memory :) */
            m_curr = reinterpret_cast<std::byte *>( next );
            return reinterpret_cast<std::byte *>( aligned );
        }

        Allocator( const Allocator &other ) = delete;
        Allocator( Allocator &&other ) = delete;
        void operator=( const Allocator &other ) = delete;
        void operator=( Allocator &&other ) = delete;
    };
}
