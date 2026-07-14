#pragma once

#include "utils/allocator.h"
#include <cstddef>
#include <utility>
#include "utils/arrayView.h"

namespace Utils {

    template <typename T>
    class BumpArray 
    {
        T *m_base{};
        std::size_t m_count{};
        Utils::Allocator &m_allocator;

        public:

        BumpArray() = default;

        BumpArray( Utils::Allocator &allocator )
            : m_allocator( allocator )
        {}

        BumpArray( const BumpArray &other ) = delete;
        BumpArray( BumpArray &&other ) 
            : m_base( std::exchange( other.m_base, nullptr ) ),
              m_count( std::exchange( other.m_count, 0 ) ),
              m_allocator( other.m_allocator )
        {}

        void operator=( const BumpArray &other ) = delete;
        BumpArray &operator=( BumpArray &&other ) noexcept
        {
            m_base = std::exchange( other.m_base, nullptr );
            m_count = std::exchange( other.m_count, 0 );
            return *this;
        }

        /* Bump must be called in series with no other allocations in between. */
        /* This is a systems level guarantee, if it breaks we have no way of verifying... */
        void bump( const T& elem ) noexcept 
        {
            if ( !m_base )
                m_base = reinterpret_cast< T* >( m_allocator.allocate( sizeof( T ), alignof( T ) ) );
            m_base[ m_count++ ] = elem;
        }

        [[nodiscard]] T &operator[]( std::size_t idx ) const noexcept
        {
            return m_base[ idx ];
        }

        [[nodiscard]] std::size_t size() const noexcept 
        {
            return m_count;
        }

        [[nodiscard]] ArrayView<T> toView() const noexcept 
        {
            return { m_base, m_count };
        }

    };


};
