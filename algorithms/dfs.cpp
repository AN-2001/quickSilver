#include "dfs.h"
#include "utils/job.h"
#include "utils/serializer.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stack>
#include <vector>

void Algorithms::Dfs::run() noexcept 
{
    if ( m_err != Algorithms::DfsErrorType::NoError )
        return;
    uint16_t source = static_cast< uint16_t > ( m_source );
    std::stack< uint16_t > dfsStack;
    std::vector< uint8_t > visited( m_graph.numVertices, 0 );

    dfsStack.push( source );
    
    visited[ source ] = 1;
    m_parents[ source ] = source;

    while ( !dfsStack.empty() ) {
        auto v = dfsStack.top();
        dfsStack.pop();

        for ( std::size_t i = m_graph.offsets[ v ]; i < m_graph.offsets[ v + 1 ]; ++i ) {
            uint16_t n = m_graph.adj[ i ];
            if ( !visited[ n ] ) {
                visited[ n ] = 1;
                m_parents[ n ] = v;
                dfsStack.push( n );
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
        if ( m_parents[ i ] )
            serialize << std::to_string( *m_parents[ i ] );
        else
            serialize << "-1";
    }

    serialize << "]}";
}
