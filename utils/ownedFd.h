/******************************************************************************\
*  OwnedFd.h                                                                   *
*                                                                              *
*  RAII wrapper for a file-descriptor.                                         *
*  Enforces unique ownership with support for weak semantics.                  *
*                                                                              *
*              Written by A.N.                                  23-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once
#include <utility>
#include <errno.h>
#include <unistd.h>

namespace Utils {
    constexpr int INVALID_FD = -1;

    class OwnedFd {
        private:
            int fd;
            bool isOwned;
        public:
            explicit OwnedFd( int fd, bool isOwned = true ) noexcept
                : fd( fd ), isOwned( isOwned ) {}

            OwnedFd( OwnedFd &&other ) noexcept
                : fd( std::exchange( other.fd, INVALID_FD ) ),
                  isOwned( std::exchange( other.isOwned, false ) )
                {}

            OwnedFd &operator=( OwnedFd &&other ) noexcept
            {
                if ( this == &other )
                    return *this;
                if ( isOwned && fd != INVALID_FD )
                    ::close( fd );

                fd = std::exchange( other.fd, -1 );
                isOwned = std::exchange( isOwned, false );
                return *this;
            }

            ~OwnedFd() noexcept
            {
                if ( isOwned && fd != INVALID_FD )
                    ::close( fd );
            }

            int get() const noexcept
            {
                return fd;
            }

            OwnedFd( const OwnedFd &other ) = delete;
            OwnedFd &operator=( const OwnedFd &other ) = delete;
    };
};
