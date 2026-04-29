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

#include <cstdint>
#include "jsonParser/object.h"
#include <memory>
#include "ownedFd.h"
#include <unistd.h>
#include <utility>

namespace Utils {
    class Job {
        private:
            OwnedFd readFd;
            int writeFd;

        public:
            std::unique_ptr< Json::Object > json;

            Job( int readFd, int writeFd, bool ownsReadFd ) noexcept
                : readFd( readFd, ownsReadFd  ),
                  writeFd( writeFd ),
                  json( nullptr )
            {}

            Job( Job &&other ) noexcept
                : readFd( std::move( other.readFd ) ),
                  writeFd( std::exchange( other.writeFd, -1 ) ),
                  json( std::move( other.json ) )
            {}


            ~Job() noexcept = default;

            Job &operator=( Job &&other ) noexcept
            {
                if ( this == &other )
                    return *this;

                readFd = std::move( other.readFd );
                writeFd = std::exchange( other.writeFd, -1 );
                json = std::move( other.json );

                return *this;
            }

            Job( const Job &other ) = delete;
            Job &operator=( const Job &other ) = delete;

            int read( void *buff, size_t size ) noexcept
            {
                return ::read( readFd.get(), buff, size );
            }

            int write( const void *buff, size_t size ) noexcept
            {
                return ::write( writeFd, buff, size );
            }
    };

};
