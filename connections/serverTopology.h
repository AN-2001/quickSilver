
#pragma once

#include "jobBuilder/jobPipeline.h"
#include "spmcQueue.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include <array>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 10


namespace Connections {

    inline void workerFunction( SpmcQueue<> &queue )
    {
        while ( true ) {
            auto job = queue.pop();
            JobTools::JobPipeline pipeline( job );
            pipeline.execute();
        }
    }

    template <std::size_t NumThreads = 16 >
        class ServerTopology {
            SpmcQueue<> m_queue;
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

            void start() noexcept
            {

                int listen_fd, client_fd;
                struct sockaddr_in server_addr, client_addr;
                socklen_t client_len = sizeof(client_addr);

                // Create socket
                listen_fd = socket(AF_INET, SOCK_STREAM, 0);
                if (listen_fd < 0) {
                    perror("socket");
                    exit(EXIT_FAILURE);
                }

                // Allow quick restart
                int opt = 1;
                setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

                ::memset(&server_addr, 0, sizeof(server_addr));
                server_addr.sin_family = AF_INET;
                server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                server_addr.sin_port = htons(PORT);

                // Bind
                if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                    perror("bind");
                    close(listen_fd);
                    exit(EXIT_FAILURE);
                }

                // Listen
                if (listen(listen_fd, BACKLOG) < 0) {
                    perror("listen");
                    close(listen_fd);
                    exit(EXIT_FAILURE);
                }

                printf("Listening on port %d...\n", PORT);

                // Accept loop
                while (1) {
                    client_fd = accept(
                            listen_fd,
                            (struct sockaddr *)&client_addr,
                            &client_len);

                    if (client_fd < 0) {
                        perror("accept");
                        continue;
                    }

                    printf("Accepted connection from %s:%d\n",
                            inet_ntoa(client_addr.sin_addr),
                            ntohs(client_addr.sin_port));
                    Utils::Job job{ Utils::OwnedFd( client_fd ), Utils::BorrowedFd( client_fd ) };
                    m_queue.push( std::move( job ) );


                    // Handle the client here...
                    client_len = sizeof(client_addr); // Reset before next accept()
                }

                close(listen_fd);


            }
        };
}
