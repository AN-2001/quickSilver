/******************************************************************************\
*  job.h                                                                       *
*                                                                              *
*  Contains the definition for a job class.                                    *
*  A job is our main resource in this project, this file specifies the job ty- *
*  pe and what it expects.                                                     *
*                                                                              *
*              Written by A.N.                                  24-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once

#include "utils/managedFd.h"
#include <sys/types.h>
#include <unistd.h>
#include <utility>

namespace Utils {
    class Job {
        private:
            ManagedFd m_readFd;
            int m_writeFd;

        public:

            Job( OwnedFd readFd, int writeFd ) noexcept
                : m_readFd( readFd  ),
                  m_writeFd( writeFd )
            {}

            Job( BorrowedFd readFd, int writeFd ) noexcept
                : m_readFd( readFd  ),
                  m_writeFd( writeFd )
            {}

            Job( Job &&other ) noexcept
                : m_readFd( std::move( other.m_readFd ) ),
                  m_writeFd( std::exchange( other.m_writeFd, -1 ) )
            {}


            ~Job() noexcept = default;

            Job &operator=( Job &&other ) noexcept
            {
                if ( this == &other )
                    return *this;

                m_readFd = std::move( other.m_readFd );
                m_writeFd = std::exchange( other.m_writeFd, -1 );

                return *this;
            }

            Job( const Job &other ) = delete;
            Job &operator=( const Job &other ) = delete;

            [[nodiscard]] ssize_t read( void *buff, const size_t size ) noexcept
            {
                return ::read( m_readFd.get(), buff, size );
            }

            [[nodiscard]] ssize_t write( const void *buff, const size_t size ) noexcept
            {
                return ::write( m_writeFd, buff, size );
            }
    };

};
