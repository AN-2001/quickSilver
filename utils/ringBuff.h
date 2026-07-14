#pragma once


#include "utils/allocator.h"
#include "utils/arrayView.h"
#include <cstddef>
#include <utility>

namespace Utils {

    template <typename T>
    struct RingBuffer {
        Utils::Allocator &m_allocator;

        std::size_t m_size;
        std::size_t m_head, m_tail;
        Utils::ArrayView< T > m_storage;

        public:
            RingBuffer( Utils::Allocator &allocator, std::size_t size ) noexcept
                :
                    m_allocator( allocator ),
                    m_size( size + 1 ),
                    m_head( 0 ),
                    m_tail( 0 ),
                    m_storage( Utils::makeArrayView<T>( m_allocator, m_size ) )
            {}

            RingBuffer( const RingBuffer& other ) noexcept
                :
                    m_allocator( other.m_allocator ),
                    m_size( other.m_size ),
                    m_head( other.m_head ),
                    m_tail( other.m_tail ),
                    m_storage( Utils::makeArrayView<T>( m_allocator, m_size ) )
            {
                memcpy( m_storage.data(), other.m_storage.data(), m_storage.size() );
            }

            RingBuffer( RingBuffer &&other ) noexcept
                :
                    m_allocator( other.m_allocator ),
                    m_size( std::exchange( other.m_size, 0 ) ),
                    m_head( std::exchange( other.m_head, 0 ) ),
                    m_tail( std::exchange( other.m_tail, 0 ) ),
                    m_storage( std::exchange( other.m_storage, nullptr ) )
            {
            }

            RingBuffer &operator=( const RingBuffer &other ) noexcept
            {
                if ( this == &other )
                    return *this;
                m_size = other.m_size;
                m_head = other.m_head;
                m_tail = other.m_tail;
                m_storage = Utils::makeArrayView<T>( m_allocator, m_size );
                memcpy( m_storage.data(), other.m_storage.data(), m_storage.size() );
                return *this;
            }

            RingBuffer &operator=( RingBuffer &&other ) noexcept
            {
                if ( this == &other )
                    return *this;
                m_size = std::exchange( other.m_size, 0 );
                m_head = std::exchange( other.m_head, 0 );
                m_tail = std::exchange( other.m_tail, 0 );
                m_storage = std::exchange( other.m_storage, nullptr );
                return *this;
            }

            ~RingBuffer() = default;


            [[nodiscard]] bool push( const T &elem ) noexcept
            {
                std::size_t next = ( m_head + 1 ) % m_size;
                if ( next == m_tail )
                    return false;
                m_storage[ m_head ] = elem;
                m_head = next;
                return true;
            }

            [[nodiscard]] std::optional< T > pop() noexcept
            {
                if ( m_tail == m_head )
                    return std::nullopt;
                std::size_t next = ( m_tail + 1 ) % m_size;
                T &ret = m_storage[ m_tail ];
                m_tail = next;
                return ret;
            }

            [[nodiscard]] std::optional< T > front() noexcept
            {
                if ( m_tail == m_head )
                    return std::nullopt;
                return m_storage[ m_tail ];
            }

            [[nodiscard]] bool empty() noexcept
            {
                return m_tail == m_head;
            }

            [[nodiscard]] std::size_t size() noexcept
            {
                if ( m_head >= m_tail ) 
                    return m_head - m_tail;
                return ( m_size - m_tail + m_head );
            }
    };


};
