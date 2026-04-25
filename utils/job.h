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

#include <utility>
#include <unistd.h>
#include <memory>

namespace GraphToys {
    class JsonObject;
    class Job {
        private:
            int inFd;
            int outFd;
            bool closeInFd;
            std::unique_ptr< JsonObject > json;

        public:
            Job( int fd ) noexcept
                : inFd( fd ),
                  outFd( fd ),
                  closeInFd( true ),
                  json( nullptr )
            {}

            Job( Job &&other ) noexcept
                : inFd( std::exchange( other.inFd, -1 ) ),
                  outFd( std::exchange( other.outFd, -1 ) ),
                  closeInFd( std::exchange( other.closeInFd, false ) ),
                  json( std::exchange( other.json, nullptr ) )
            {}


            ~Job() noexcept {
                if ( closeInFd && inFd != -1 )
                    close( inFd );
            }

            Job &operator=( Job &&other ) noexcept
            {
                if ( this == &other )
                    return *this;

                std::swap( other.inFd, inFd );
                std::swap( other.outFd, outFd );
                std::swap( other.closeInFd, closeInFd );
                std::swap( other.json, json );

                return *this;
            }

            Job( const Job &other ) = delete;
            Job &operator=( const Job &other ) = delete;
    };

};
