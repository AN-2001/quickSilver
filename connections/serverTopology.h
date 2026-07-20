
#pragma once

#include "connections/metricsEvents.h"
#include "jobBuilder/jobPipeline.h"
#include "spmcQueue.h"
#include "mpscQueue.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include <array>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdio>
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
#include "connections/metricsCollector.h"

namespace Connections {

    static volatile std::sig_atomic_t g_serverShouldStop = 0;

    inline void pinThreadToCpu(std::size_t threadId)
    {
        unsigned int cpuCount = std::thread::hardware_concurrency();
        if (cpuCount == 0)
            return;

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        CPU_SET(threadId % cpuCount, &cpuset);

        sched_setaffinity(
                0,                    // current thread
                sizeof(cpu_set_t),
                &cpuset
                );
    }

    inline void handleSignal( int ) {
        printf( "RECEIVED STOP SIGNAL\n" );  
        g_serverShouldStop = true;
    }

    inline void workerFunction( std::size_t threadId, SpmcQueue<> &jobQueue, MpscQueue<> &eventQueue )
    {

        printf( "WORKER %lu STARTED\n", threadId );

        pinThreadToCpu( threadId );

        using namespace Utils;
        Utils::Arena arena{ 1_MB };
        while ( true ) {
            {
                auto now = std::chrono::steady_clock::now();

                Utils::Allocator allocator( arena );

                auto job = jobQueue.pop(); 
                if ( job.isStopJob() ) {
                    printf( "WORKER %lu IS EXITING\n", threadId );
                    return;
                }
                auto afterPop = std::chrono::steady_clock::now();
                double duration = std::chrono::duration< double > ( afterPop - now ).count();
                eventQueue.push( { MetricsEventType::QueuePopLatency, {}, duration } ); 

                JobTools::JobPipeline pipeline( job, allocator, &eventQueue );
                pipeline.execute();
            }
        }

    }

    inline void metricsFunction( std::size_t threadId, MpscQueue<> &eventQueue, SpmcQueue<> &jobQueue )
    {
        (void)(threadId);
        MetricsCollector collector( eventQueue, jobQueue );
        printf( "METRICS THREAD STARTED\n" );

        collector.collect();
        printf( "METRICS THREAD IS EXITING\n" );
    }

    template <std::size_t NumThreads = 16 >
        class ServerTopology {
            SpmcQueue<> m_queue;
            MpscQueue<> m_eventQueue;
            std::array< std::thread, NumThreads > pool;
            std::thread metricsThread;

            public:
            ServerTopology() {
                std::signal( SIGTERM, handleSignal );

                metricsThread = std::thread( metricsFunction, 0, std::ref( m_eventQueue ), std::ref( m_queue ) );
                for ( std::size_t i = 0; i < pool.size(); ++i )
                    pool[ i ] = std::thread( workerFunction, i, std::ref( m_queue ), std::ref( m_eventQueue ) );
            }

            ~ServerTopology() {
                for ( std::size_t i = 0; i < pool.size(); ++i )
                    pool[ i ].join();
                metricsThread.join();
                printf( "SERVER IS EXITING\n" );  
            }

            int start() noexcept
            {
                struct sockaddr_in server_addr;

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

                if ( ::listen( listen_fd, 128 ) < 0 ) {
                    perror( "listen" );
                    close( listen_fd );
                    return EXIT_FAILURE;
                }

                ::printf( "Server is listening on port %d...\n", 8080 );



                while ( !g_serverShouldStop ) {
                    fd_set readfds;
                    FD_ZERO(&readfds);
                    FD_SET(listen_fd, &readfds);

                    timeval timeout{};
                    timeout.tv_sec = 1;

                    int ret = select(listen_fd + 1, &readfds, nullptr, nullptr, &timeout);
                    if ( ret < 0 ) {
                        perror( "select" );
                        continue;
                    }

                    if (ret > 0 && FD_ISSET(listen_fd, &readfds)) {
                        int client_fd = accept(listen_fd, nullptr, nullptr);
                        if ( client_fd < 0 ) {
                            perror( "accept" );
                            continue;
                        }

                        Utils::Job job{ Utils::OwnedFd( client_fd ), Utils::BorrowedFd( client_fd ) };
                        m_queue.push( std::move( job ) );
                    }

                }

                for ( int i = 0; i < 100; ++i )
                    m_queue.push( Utils::Job() );
                m_eventQueue.push( { MetricsEventType::ShutDownMetricsThread, {}, 0 } );
                ::close( listen_fd );
                return 0;
            }
        };
}
