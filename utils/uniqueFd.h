/******************************************************************************\
*  uniqueFd.h                                                                  *
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

namespace GraphToys {

    class UniqueFd;

    class FdView {
        private:
            int fd;

            friend UniqueFd;
            FdView( int fd ) noexcept : fd( fd ) {}
            FdView( const FdView &other ) noexcept = delete;
            FdView &operator=( const FdView &other ) noexcept = delete;

        public:
            int get() const noexcept
            {
                return fd;
            }

            ~FdView() noexcept {}

    };

    class UniqueFd {
        private:
            static constexpr int INVALID_FD = -1;
            int fd;
        public:
            explicit UniqueFd( int fd ) noexcept : fd( fd ) {}
            UniqueFd( UniqueFd &&other ) noexcept
                : fd( std::exchange( other.fd, INVALID_FD ) ) {}

            UniqueFd &operator=( UniqueFd &&other ) noexcept
            {
                if ( this == &other )
                    return *this;

                while ( fd != INVALID_FD && close( fd ) == -1 && errno == EINTR );

                fd = std::exchange( other.fd, INVALID_FD );
                return *this;
            }

            ~UniqueFd() noexcept
            {
                while ( fd != INVALID_FD && close( fd ) == -1 && errno == EINTR );
            }

            const int get() const noexcept
            {
                return fd;
            }

            FdView getView() const noexcept
            {
                return FdView( fd );
            }

            UniqueFd( const UniqueFd &other ) = delete;
            UniqueFd &operator=( const UniqueFd &other ) = delete;
    };
};
