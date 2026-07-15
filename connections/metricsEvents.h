#pragma once

#include "utils/job.h"
namespace Connections {

    enum class MetricsEventType {
        MetricsRequest,
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
