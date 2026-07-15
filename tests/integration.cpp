#include "connections/metricsEvents.h"
#include "connections/mpscQueue.h"
#include "utils/allocator.h"
#include "utils/arena.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include "jobBuilder/jobPipeline.h"

static void metricsFunc( Connections::MpscQueue<> &eventQueue )
{
    while ( true ) {
        Connections::MetricsEvent event = eventQueue.pop();
        if ( event.type == Connections::MetricsEventType::ShutDownMetricsThread )
            break;
    }

}

int main()
{
    Connections::MpscQueue<> eventQueue;
    std::thread metricsThread{ metricsFunc, std::ref( eventQueue ) };

    using namespace Utils;
    Utils::Arena arena( 1_GB );
    Utils::Allocator allocator( arena );

    Utils::Job job( Utils::BorrowedFd{ 0 }, Utils::BorrowedFd{ 1 } );
    JobTools::JobPipeline pipeline( job, allocator, &eventQueue );
    pipeline.execute();

    eventQueue.push( { Connections::MetricsEventType::ShutDownMetricsThread, {}, 0 } );
    metricsThread.join();
}
