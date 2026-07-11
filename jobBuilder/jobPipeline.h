#pragma once

#include "utils/job.h"
namespace JobTools {
    class JobPipeline {
        Utils::Job &m_job;

        public:
            JobPipeline( Utils::Job &job ) 
                : m_job( job )
            {}
        void execute() noexcept;
    };

}
