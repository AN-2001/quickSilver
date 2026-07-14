#include "utils/allocator.h"
#include "utils/arena.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include "jobBuilder/jobPipeline.h"

int main()
{
    using namespace Utils;
    Utils::Arena arena( 1_GB );
    if ( !arena.validate() )
        return 1;

    Utils::Allocator allocator( arena );

    Utils::Job job( Utils::BorrowedFd{ 0 }, Utils::BorrowedFd{ 1 } );
    JobTools::JobPipeline pipeline( job, allocator );
    pipeline.execute();
}
