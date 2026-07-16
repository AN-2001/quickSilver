#pragma once

#include "connections/mpscQueue.h"
#include "connections/spmcQueue.h"
#include "utils/ringBuff.h"
#include "utils/serializer.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <numeric>
#include <string>
#include <string_view>
namespace Connections {

    template <std::size_t NumSamples = 1024 >
    struct ThroughputCollector
    {
        using TimeStamp = std::chrono::steady_clock::time_point;
        std::array< TimeStamp, NumSamples > m_ringBuffStorage{};
        Utils::RingBuffer< TimeStamp > m_ringBuff{ m_ringBuffStorage };
        std::size_t m_count{};

        void postJob() noexcept {
            auto now = std::chrono::steady_clock::now();
            if ( m_ringBuff.full() )
                m_ringBuff.pop();
            m_ringBuff.push( now );
            m_count++;
        }

        [[nodiscard]] auto inline getThroughput() noexcept {
            auto now = std::chrono::steady_clock::now();
            while ( !m_ringBuff.empty() ) {
                TimeStamp front = m_ringBuff.front();
                if (now - front > std::chrono::seconds(1))
                    m_ringBuff.pop();
                else
                    break;
            }
            return m_ringBuff.size();
        }

        void inline serialize( Utils::Serializer &serializer, std::string_view name ) noexcept {
            serializer << "\"" << name << "\"" << ":" << std::to_string( getThroughput() );
        }

    };

    template <std::size_t NumSamples = 1024 >
    struct LatencyCollector {
        std::array< double, NumSamples > m_ringBuffStorage{};
        Utils::RingBuffer< double > m_ringBuff{ m_ringBuffStorage };
        std::size_t m_count{};

        void addLatency( double latency ) noexcept {
            if ( m_ringBuff.full() )
                m_ringBuff.pop();
            m_ringBuff.push( latency );
            m_count++;
        }

        [[nodiscard]] auto inline getCount() const noexcept {
            return m_count;
        }

        [[nodiscard]] auto inline getAvgLatency() const noexcept {

            double sum = std::accumulate( begin( m_ringBuffStorage ),
                                          end( m_ringBuffStorage ),
                                          0.0 );
            return sum / NumSamples;
        }

        /* Only called once fully populated, will do partial later.. */
        [[nodiscard]] auto inline getPrecentiles() const noexcept {
            std::array< double, NumSamples > sorted = m_ringBuffStorage;
            std::sort( begin( sorted ), end( sorted ) );
            std::size_t p50 = static_cast< std::size_t > ( NumSamples * 0.5 );
            std::size_t p95 = static_cast< std::size_t > ( NumSamples * 0.95 );
            std::size_t p99 = static_cast< std::size_t > ( NumSamples * 0.99 );
            return std::tuple{ sorted[ p50 ], sorted[ p95 ], sorted[ p99 ] };
        }

        void inline serialize( Utils::Serializer &serializer, std::string_view name ) const noexcept {
            auto [ p50, p95, p99 ] = getPrecentiles();

            serializer << "\"" << name << "\"" << ":{";
            serializer << "\"avgLatency\":" << std::to_string( getAvgLatency() ) << ",";
            serializer << "\"P50\":" << std::to_string( p50 ) << ",";
            serializer << "\"P95\":" << std::to_string( p95 ) << ",";
            serializer << "\"P99\":" << std::to_string( p99 );
            serializer << "}";
        }

    };

    class MetricsCollector {
        Connections::MpscQueue<> &m_eventQueue;
        Connections::SpmcQueue<> &m_jobQueue;

        ThroughputCollector<> throughputCollector{};
        LatencyCollector<> popCollector{};
        LatencyCollector<> schedCollector{};
        LatencyCollector<> totalCollector{};
        LatencyCollector<> parseCollector{};
        LatencyCollector<> validateCollector{};
        LatencyCollector<> buildCollector{};
        LatencyCollector<> algoCollector{};

        public:
            MetricsCollector( Connections::MpscQueue<> &eventQueue, Connections::SpmcQueue<> &jobQueue ) :
                m_eventQueue( eventQueue ),
                m_jobQueue( jobQueue )
            {}


            void collect() noexcept {
                using namespace std::string_view_literals;
                while ( true ) {
                    MetricsEvent event = m_eventQueue.pop();
                    switch ( event.type ) {
                        case MetricsEventType::MetricsRequest: {
                            Utils::Serializer serializer( event.job );
                            serializer << R"JSON({"status":"OK","jobCount":)JSON"
                                << std::to_string( totalCollector.getCount() ) << ",";

                            throughputCollector.serialize( serializer, "throughput"sv );
                            serializer << ",";

                            serializer << "\"jobQueueSize\":" << std::to_string( m_jobQueue.size() );
                            serializer << ",";

                            popCollector.serialize( serializer, "pop"sv );
                            serializer << ",";

                            schedCollector.serialize( serializer, "sched"sv );
                            serializer << ",";

                            totalCollector.serialize( serializer, "total_execution"sv );
                            serializer << ",";

                            parseCollector.serialize( serializer, "parse"sv );
                            serializer << ",";

                            validateCollector.serialize( serializer, "validate"sv );
                            serializer << ",";

                            buildCollector.serialize( serializer, "build"sv );
                            serializer << ",";

                            algoCollector.serialize( serializer, "algo"sv );
                            serializer << R"JSON(})JSON";
                            break;
                        }

                        case MetricsEventType::QueuePopLatency:
                            popCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::SchedJobLatency:
                            schedCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::PostJobLatency:
                            throughputCollector.postJob();
                            totalCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::PostJobParseLatency:
                            parseCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::PostJobValidateLatency:
                            validateCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::PostJobBuildLatency:
                            buildCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::PostJobAlgoLatency:
                            algoCollector.addLatency( event.duration );
                            break;
                        case MetricsEventType::ShutDownMetricsThread:
                        default:
                            return;
                    }
                }
            }



    };


};
