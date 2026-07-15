#pragma once

#include "connections/metricsEvents.h"
#include "connections/mpscQueue.h"
#include <chrono>
namespace JobTools {

    class Timer {
        Connections::MpscQueue<> *m_eventQueue;
        Connections::MetricsEventType m_event;
        std::chrono::steady_clock::time_point m_now;

        public:
            Timer( Connections::MpscQueue<> *queue, Connections::MetricsEventType event ) 
                :   m_eventQueue( queue ),
                    m_event( event ),
                    m_now( std::chrono::steady_clock::now() )
            {}

            ~Timer() 
            {
                auto now = std::chrono::steady_clock::now();
                double duration = std::chrono::duration< double >( now - m_now ).count();
                m_eventQueue -> push( { m_event, {}, duration } );
            }


    };

}
