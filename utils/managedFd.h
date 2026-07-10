/******************************************************************************\
*  managedFd.h                                                                 *
*                                                                              *
*  RAII wrapper for a file-descriptor.                                         *
*  Enforces unique ownership with support for weak semantics.                  *
*                                                                              *
*              Written by A.N.                                  23-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once
#include <utility>
#include <unistd.h>

namespace Utils {

    template < bool tag >
    struct TaggedFd {
        int num;
    };

    using OwnedFd = TaggedFd<true>;
    using BorrowedFd = TaggedFd<false>;

    class ManagedFd {

            static constexpr int g_invalidFd = -1;
            int m_fd;
            bool m_isOwned;
        public:

            explicit ManagedFd( OwnedFd fd ) noexcept
                : m_fd( fd.num ), m_isOwned( true ) {}

            explicit ManagedFd( BorrowedFd fd ) noexcept
                : m_fd( fd.num ), m_isOwned( false ) {}

            ManagedFd( ManagedFd &&other ) noexcept
                : m_fd( std::exchange( other.m_fd, g_invalidFd ) ),
                  m_isOwned( std::exchange( other.m_isOwned, false ) )
                {}

            ManagedFd &operator=( ManagedFd &&other ) noexcept
            {
                if ( this == &other )
                    return *this;
                if ( m_isOwned && m_fd != g_invalidFd )
                    ::close( m_fd );

                m_fd = std::exchange( other.m_fd, g_invalidFd );
                m_isOwned = std::exchange( other.m_isOwned, false );
                return *this;
            }

            ~ManagedFd() noexcept
            {
                if ( m_isOwned && m_fd != g_invalidFd )
                    ::close( m_fd );
            }

            int get() const noexcept
            {
                return m_fd;
            }

            ManagedFd( const ManagedFd &other ) = delete;
            ManagedFd &operator=( const ManagedFd &other ) = delete;
    };

};
