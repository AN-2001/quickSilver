#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include "arena.h"

namespace Utils {

    class Allocator {
        void *m_begin;
        void *m_end;
        void *m_curr;

        public:

        Allocator() = delete;

        Allocator( const Utils::Arena &arena ) noexcept 
            : m_begin( arena.data() ),
              m_end( reinterpret_cast<void*>( reinterpret_cast<uintptr_t>( arena.data() ) + arena.size() ) ),
              m_curr( m_begin )
        {}

        ~Allocator() noexcept
        {
            m_curr = m_begin;
        }

        void *allocate( std::size_t size, std::size_t alignment ) noexcept {
            std::uintptr_t aligned =
                ( reinterpret_cast< std::uintptr_t >( m_curr ) + alignment - 1 ) & ~( alignment - 1 );
            std::uintptr_t next = aligned + size; /* I don't care if we run out of memory :) */
            assert( next < reinterpret_cast<uintptr_t>( m_end ) );
            m_curr = reinterpret_cast< void* >( next );
            return reinterpret_cast< void *> ( aligned );
        }

        Allocator( const Allocator &other ) = delete;
        Allocator( Allocator &&other ) = delete;
        void operator=( const Allocator &other ) = delete;
        void operator=( Allocator &&other ) = delete;
    };
}
