#pragma once

#include <cstddef>
#include <sys/mman.h>
#include <unistd.h>

namespace Utils {

    static const std::size_t PAGE_SIZE = static_cast< std::size_t > ( sysconf( _SC_PAGESIZE ) );

    consteval std::size_t operator""_KB(unsigned long long n) {
        return n * 1024ULL;
    }

    consteval std::size_t operator""_MB(unsigned long long n) {
        return n * 1024_KB;
    }

    consteval std::size_t operator""_GB(unsigned long long n) {
        return n * 1024_MB;
    }

    /* Assumption: data used in this arena is assumed to be PODs. */
    class Arena {
        std::size_t m_size{};
        void *m_data{};
        bool m_hadError{};

        public:

        Arena() = delete;

        Arena( std::size_t size ) noexcept
            : m_size( size ),
              m_data( ::mmap( nullptr,
                      size + PAGE_SIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 ) ),
              m_hadError( m_data == MAP_FAILED )
        {
            if ( m_hadError )
                return;
            mprotect(
                static_cast<char*>( m_data ) + m_size,
                PAGE_SIZE,
                PROT_NONE
            );
        }

        Arena( const Arena &other ) = delete;
        Arena( Arena &&other ) = delete;
        void operator=( const Arena & other ) = delete;
        void operator=( Arena && other ) = delete;

        ~Arena() noexcept
        {
            if ( !m_hadError )
                ::munmap( m_data, m_size );
        }

        [[nodiscard]] std::size_t size() const noexcept
        {
            return m_size;
        }

        [[nodiscard]] void * data() const noexcept 
        {
            return m_data;
        }
    };

}
