#pragma once

#include "utils/ringBuff.h"
#include "utils/serializer.h"
#include <algorithm>
#include <array>
#include <numeric>
#include <string>
#include <string_view>
namespace Connections {

    template <std::size_t NumSamples = 1024 >
    struct MetricsCollector {
        std::array< double, NumSamples > m_ringBuffStorage{};
        Utils::RingBuffer< double > m_ringBuff{ m_ringBuffStorage };
        std::size_t m_count{};

        void addLatency( double latency ) noexcept {
            if ( m_ringBuff.full() )
                (void)m_ringBuff.pop();
            (void)m_ringBuff.push( latency );
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
};
