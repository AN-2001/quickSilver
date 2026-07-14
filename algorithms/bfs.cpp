#include "bfs.h"
#include "utils/job.h"
#include "utils/serializer.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include "utils/ringBuff.h"

void Algorithms::Bfs::run() noexcept 
{
    if ( m_err != Algorithms::BfsErrorType::NoError )
        return;
    uint16_t source = static_cast< uint16_t > ( m_source );
    uint16_t distance = 0;
    Utils::RingBuffer< uint16_t > bfsQueue( m_allocator, m_graph.numVertices );
    Utils::ArrayView< uint16_t > visited( Utils::makeArrayView<uint16_t>( m_allocator, m_graph.numVertices, 0 ) );

    if ( !bfsQueue.push( source ) ) 
        return;
    
    visited[ source ] = 1;
    m_distances[ source ] = distance;
    m_parents[ source ] = source;
    distance++;

    while ( !bfsQueue.empty() ) {
        std::size_t size = bfsQueue.size();

        for ( std::size_t k = 0; k < size; ++k ) {
            auto v = bfsQueue.front();
            if ( !v )
                return;
            for ( uint16_t i = m_graph.offsets[ *v ]; i < m_graph.offsets[ *v + 1 ]; ++i ) {
                uint16_t n = m_graph.adj[ i ];
                if ( !visited[ n ] ) {
                    visited[ n ] = 1;
                    if ( !bfsQueue.push( n ) )
                        return;

                    m_distances[ n ] = distance;
                    m_parents[ n ] = *v;
                }
            }
            (void)bfsQueue.pop();
        }

        distance++;
    }
    
}

void Algorithms::Bfs::serialize( Utils::Job &job ) noexcept 
{
    Utils::Serializer serialize( job );

    if ( m_err != BfsErrorType::NoError ) {
        serialize << R"JSON({"status":"BFS error","error":")JSON";
        serialize << m_errorStrings.get( m_err );
        serialize << R"JSON("})JSON";
        return;
    }

    serialize << R"JSON({"status":"ok","distance":[)JSON";

    bool first = true;
    for ( const auto n : m_distances ) {
        if ( !first )
            serialize << ",";
        first = false;
        if ( n != std::numeric_limits<uint16_t>::max() ) 
            serialize << std::to_string( n );
        else
            serialize << "\"INF\"";
    }

    serialize << R"JSON(],"parent":[)JSON";
    first = true;
    for ( std::size_t i = 0; i < m_parents.size(); ++i ) {
        if ( !first )
            serialize << ",";
        first = false;
        if ( m_parents[ i ] != std::numeric_limits<uint16_t>::max() )
            serialize << std::to_string( m_parents[ i ] );
        else
            serialize << "-1";
    }

    serialize << "]}";
}
