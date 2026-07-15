
#pragma once

#include "connections/metricsEvents.h"
#include "jobBuilder/jobPipeline.h"
#include "spmcQueue.h"
#include "mpscQueue.h"
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
#include "utils/serializer.h"
#include "connections/metricsCollector.h"

namespace Connections {

    inline void workerFunction( std::size_t threadId, SpmcQueue<> &jobQueue, MpscQueue<> &eventQueue )
    {
        using namespace Utils;

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET( ( threadId % 4 ) , &cpuset);

        sched_setaffinity(0, sizeof(cpuset), &cpuset);

        Utils::Arena arena{ 1_MB };
        while ( true ) {
            {
                Utils::Allocator allocator( arena );
                auto job = jobQueue.pop();
                JobTools::JobPipeline pipeline( job, allocator, &eventQueue );
                pipeline.execute();
            }
        }

    }

    inline void metricsFunction( std::size_t threadId, MpscQueue<> &queue )
    {
        MetricsCollector<> totalCollector;
        MetricsCollector<> parseCollector;
        MetricsCollector<> validateCollector;
        MetricsCollector<> buildCollector;
        MetricsCollector<> algoCollector;

        using namespace Utils;

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET( ( threadId % 4 ) , &cpuset);
        sched_setaffinity(0, sizeof(cpuset), &cpuset);

        while ( true ) {
            MetricsEvent event = queue.pop();
            switch ( event.type ) {
                case MetricsEventType::MetricsRequest: {
                    Utils::Serializer serializer( event.job );
                    serializer << R"JSON({"status":"OK","jobCount":)JSON"
                        << std::to_string( totalCollector.getCount() ) << ",";

                    totalCollector.serialize( serializer, "total" );
                    serializer << ",";

                    parseCollector.serialize( serializer, "parse" );
                    serializer << ",";

                    validateCollector.serialize( serializer, "validate" );
                    serializer << ",";

                    buildCollector.serialize( serializer, "build" );
                    serializer << ",";

                    algoCollector.serialize( serializer, "algo" );
                    serializer << R"JSON(})JSON";
                    break;
                }

                case MetricsEventType::PostJobLatency:
                    totalCollector.addLatency( event.duration );
                    break;
                case MetricsEventType::PostJobParseLatency:
                    parseCollector.addLatency( event.duration );
                    break;
                case MetricsEventType::PostJobValidateLatency:
                    validateCollector.addLatency( event.duration );
                    break;
                case MetricsEventType::PostJobBuildLatency:
                    buildCollector.addLatency( event.duration );
                    break;
                case MetricsEventType::PostJobAlgoLatency:
                    algoCollector.addLatency( event.duration );
                    break;
                case MetricsEventType::ShutDownMetricsThread:
                default:
                    return;
            }
        }

    }

    template <std::size_t NumThreads = 16 >
        class ServerTopology {
            SpmcQueue<> m_queue;
            MpscQueue<> m_eventQueue;
            std::array< std::thread, NumThreads > pool;
            std::thread metricsThread;

            public:
            ServerTopology() {
                metricsThread = std::thread( metricsFunction, 0, std::ref( m_eventQueue ) );
                for ( std::size_t i = 0; i < pool.size(); ++i )
                    pool[ i ] = std::thread( workerFunction, i, std::ref( m_queue ), std::ref( m_eventQueue ) );
            }

            ~ServerTopology() {
                for ( std::size_t i = 0; i < pool.size(); ++i )
                    pool[ i ].join();
                metricsThread.join();
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

                if ( ::listen( listen_fd, 128 ) < 0 ) {
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
