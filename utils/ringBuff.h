#pragma once


#include <cassert>
#include <cstddef>
#include <span>

namespace Utils {

    template <typename T>
    struct RingBuffer {
        std::size_t m_head{}, m_tail{};
        std::span< T > m_storage;

        public:
            RingBuffer( std::span< T > span ) noexcept
                :
                    m_storage( span )
            {}

            ~RingBuffer() = default;


            void push( const T &elem ) noexcept
            {
                std::size_t next = ( m_head + 1 ) % m_storage.size();
                if ( next == m_tail )
                    return;
                m_storage[ m_head ] = elem;
                m_head = next;
            }

            void pop() noexcept
            {
                if ( m_tail == m_head )
                    return;
                std::size_t next = ( m_tail + 1 ) % m_storage.size();
                m_tail = next;
            }

            [[nodiscard]] T& front() noexcept
            {
                assert( !empty() );
                return m_storage[ m_tail ];
            }

            [[nodiscard]] bool empty() noexcept
            {
                return m_tail == m_head;
            }

            [[nodiscard]] bool full() noexcept
            {
                std::size_t next = ( m_head + 1 ) % m_storage.size();
                return next == m_tail;
            }

            [[nodiscard]] std::size_t size() noexcept
            {
                if ( m_head >= m_tail ) 
                    return m_head - m_tail;
                return ( m_storage.size() - m_tail + m_head );
            }
    };


};
