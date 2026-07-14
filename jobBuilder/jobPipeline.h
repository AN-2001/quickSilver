#pragma once

#include "utils/allocator.h"
#include "utils/job.h"
namespace JobTools {
    class JobPipeline {
        Utils::Job &m_job;
        Utils::Allocator &m_allocator;

        public:
            JobPipeline( Utils::Job &job, Utils::Allocator &allocator ) 
                : m_job( job ),
                  m_allocator( allocator )
            {}
        void execute() noexcept;
    };

}
