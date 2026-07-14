#include "dfs.h"
#include "utils/arrayView.h"
#include "utils/job.h"
#include "utils/serializer.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

void Algorithms::Dfs::run() noexcept 
{
    if ( m_err != Algorithms::DfsErrorType::NoError )
        return;
    uint16_t source = static_cast< uint16_t > ( m_source );
    Utils::ArrayView< uint16_t > dfsStack( Utils::makeArrayView<uint16_t>( m_allocator, m_graph.numVertices ) );
    Utils::ArrayView< uint16_t > visited( Utils::makeArrayView<uint16_t>( m_allocator, m_graph.numVertices, 0 ) );
    std::size_t top = 0;

    dfsStack[ top++ ] = source;
    
    visited[ source ] = 1;
    m_parents[ source ] = source;

    while ( top ) {
        auto v = dfsStack[ --top ];

        for ( std::size_t i = m_graph.offsets[ v ]; i < m_graph.offsets[ v + 1 ]; ++i ) {
            uint16_t n = m_graph.adj[ i ];
            if ( !visited[ n ] ) {
                visited[ n ] = 1;
                m_parents[ n ] = v;
                dfsStack[ top++ ] = n;
            }
        }
    }
    
}

void Algorithms::Dfs::serialize( Utils::Job &job ) noexcept 
{
    Utils::Serializer serialize( job );

    if ( m_err != DfsErrorType::NoError ) {
        serialize << R"JSON({"status":"DFS error","error":")JSON";
        serialize << m_errorStrings.get( m_err );
        serialize << R"JSON("})JSON";
        return;
    }

    serialize << R"JSON({"status":"ok","parent":[)JSON";

    bool first = true;
    for ( std::size_t i = 0; i < m_parents.size(); ++i ) {
        if ( !first )
            serialize << ",";
        first = false;
        if ( m_parents[ i ] != std::numeric_limits< uint16_t >::max() )
            serialize << std::to_string( m_parents[ i ] );
        else
            serialize << "null";
    }

    serialize << "]}";
}
