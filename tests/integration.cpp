#include "utils/job.h"
#include "utils/managedFd.h"
#include "jobBuilder/jobPipeline.h"

int main()
{
    Utils::Job job( Utils::BorrowedFd{ 0 }, Utils::BorrowedFd{ 1 } );
    JobTools::JobPipeline pipeline( job );
    pipeline.execute();
}
