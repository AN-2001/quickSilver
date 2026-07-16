#pragma once

#include "utils/job.h"
namespace Connections {

    enum class MetricsEventType {
        MetricsRequest,
        SchedJobLatency,
        PostJobLatency,
        PostJobParseLatency,
        PostJobValidateLatency,
        PostJobBuildLatency,
        PostJobAlgoLatency,
        ShutDownMetricsThread, /* Only used internally */
    };

    struct MetricsEvent {
        MetricsEventType type;
        Utils::Job job;
        double duration;
    };

}
