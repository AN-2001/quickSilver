
#pragma once

#include "jobBuilder/jobPipeline.h"
#include "spmcQueue.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/mman.h>
#include "utils/arena.h"
#include "utils/allocator.h"

namespace Connections {

    inline void workerFunction( SpmcQueue< 4096 > &queue )
    {
        using namespace Utils;
        Utils::Arena arena{ 1_GB };
        while ( true ) {
            {
                Utils::Allocator allocator( arena );
                auto job = queue.pop();
                JobTools::JobPipeline pipeline( job, allocator );
                pipeline.execute();
            }
        }

    }

    template <std::size_t NumThreads = 16 >
        class ServerTopology {
            SpmcQueue< 4096 > m_queue;
            std::array< std::thread, NumThreads > pool;

            public:
            ServerTopology() {
                for ( std::size_t i = 0; i < pool.size(); ++i )
                    pool[ i ] = std::thread( workerFunction, std::ref( m_queue ) );
            }

            ~ServerTopology() {
                for ( std::size_t i = 0; i < pool.size(); ++i )
                    pool[ i ].join();
            }

            int start() noexcept
            {
                struct sockaddr_in server_addr, client_addr;
                socklen_t client_len = sizeof( client_addr );

                int listen_fd = socket( AF_INET, SOCK_STREAM, 0 );
                if ( listen_fd < 0 ) {
                    perror( "socket" );
                    return EXIT_FAILURE;
                }

                int opt = 1;
                setsockopt( listen_fd, SOL_SOCKET,
                            SO_REUSEADDR, &opt, sizeof( opt ) );

                ::memset(&server_addr, 0, sizeof(server_addr));
                server_addr.sin_family = AF_INET;
                server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
                server_addr.sin_port = htons( 8080 );

                if ( ::bind(listen_fd,
                            reinterpret_cast< struct sockaddr * >(
                                                    &server_addr ),
                            sizeof( server_addr ) ) < 0 ) {
                    perror( "bind" );
                    close( listen_fd );
                    return EXIT_FAILURE;
                }

                if ( ::listen( listen_fd, 1024 ) < 0 ) {
                    perror( "listen" );
                    close( listen_fd );
                    return EXIT_FAILURE;
                }

                ::printf( "Listening on port %d...\n", 8080 );
                while ( true ) {
                    client_len = sizeof( client_addr );
                    int client_fd = ::accept(
                            listen_fd,
                            reinterpret_cast< sockaddr* >( &client_addr ),
                            &client_len);

                    if ( client_fd < 0 ) {
                        ::perror( "accept" );
                        continue;
                    }

                    Utils::Job job{ Utils::OwnedFd( client_fd ), Utils::BorrowedFd( client_fd ) };
                    m_queue.push( std::move( job ) );
                }

                ::close( listen_fd );
            }
        };
}
