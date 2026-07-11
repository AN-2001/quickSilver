#pragma once

#include "jobBuilder/jobBuilder.h"
#include "utils/job.h"
#include "utils/fastMap.h"
#include <array>
#include <cstdint>
#include <utility>

namespace Algorithms {
    using namespace std::string_view_literals;

    enum class BfsErrorType : uint8_t {
        NoError = 0,
        InvalidInputCount,
        NegativeSource,
        SourceTooBig,
        NumErrors,
    };

    static consteval auto buildErrorMapping() 
    {
        constexpr std::size_t NUM_ERRORS = std::to_underlying( BfsErrorType::NumErrors );
        Utils::FastMap< BfsErrorType , std::string_view, NUM_ERRORS > map{};

        map.add( BfsErrorType::NoError, "No error"sv );
        map.add( BfsErrorType::InvalidInputCount, "Invalid input count"sv );
        map.add( BfsErrorType::NegativeSource, "Source is negative"sv );
        map.add( BfsErrorType::SourceTooBig, "Source is too big to be a vertex"sv );
        return map;
    }

    class Bfs {
        static constexpr auto m_errorStrings = buildErrorMapping();

        const JobTools::GraphCsr &m_graph;
        int m_source;
        BfsErrorType m_err = BfsErrorType::NoError;
        std::vector< std::optional< uint16_t > > m_distances;
        std::vector< std::optional< uint16_t > > m_parents;

        public:
        Bfs( const JobTools::GraphCsr &graph,
             const std::array< uint16_t, JobTools::MAX_INPUT_SIZE > input,
             std::size_t numInput )
        : m_graph( graph ),
          m_distances( graph.numVertices, std::nullopt ),
          m_parents( graph.numVertices, std::nullopt )
        {
            if ( numInput != 1 )
                m_err = BfsErrorType::InvalidInputCount;
            m_source = input[ 0 ];
            if ( m_source < 0 )
                m_err = BfsErrorType::NegativeSource;
            if ( static_cast< std::size_t >( m_source ) >= graph.numVertices )
                m_err = BfsErrorType::SourceTooBig;
        }

        void run() noexcept;
        void serialize( Utils::Job &job ) noexcept;
    };

}
