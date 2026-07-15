#pragma once

#include "utils/allocator.h"
#include "utils/job.h"
#include "connections/mpscQueue.h"

namespace JobTools {
    class JobPipeline {
        Utils::Job &m_job;
        Utils::Allocator &m_allocator;
        Connections::MpscQueue<> *m_eventQueue;

        public:
            JobPipeline( Utils::Job &job, Utils::Allocator &allocator ) 
                : m_job( job ),
                  m_allocator( allocator ),
                  m_eventQueue( nullptr )
            {}

            JobPipeline( Utils::Job &job, Utils::Allocator &allocator, Connections::MpscQueue<> *eventQueue ) 
                : m_job( job ),
                  m_allocator( allocator ),
                  m_eventQueue( eventQueue )
            {}
        void execute() noexcept;
    };

}
