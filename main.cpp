#include <iostream>
#include <memory>
#include "jsonParser/object.h"
#include "jsonParser/parser.h"
#include "utils/job.h"
#include <queue>
#include <unistd.h>

int main()
{
    using BfsState = std::pair< int, int >;
    std::queue< BfsState > bfs;
    Utils::Job job( 0, 1, false );
    Json::Parser parser( job );
    parser.parse();
    Json::Object res;

    int n = (*job.json)[ "numVertices" ].toNumber();
    int s = (*job.json)[ "source" ].toNumber();
    const auto &adj = (*job.json)[ "edges" ];
    std::vector< int > visited( n, 0 );

    res = Json::Object::JsonArray();
    res.set_array_size( n );

    bfs.emplace( s, 0 );
    visited[ s ] = 1;

    while ( !bfs.empty() ) {
        auto v = bfs.front();
        bfs.pop();

        auto asString = std::to_string( v.first );
        res.set_array_at_index( v.first, v.second );

        for ( const auto &n : adj[ v.first ].toArray() ) {
            if ( !visited[ n.toNumber() ] ) {
                visited[ n.toNumber() ] = 1;
                bfs.emplace( n.toNumber(), v.second + 1 );
            }
        }
    }

    std::string out = res.serialize();
    int r = write( STDOUT_FILENO, out.data(), out.size() );
    (void) r;

    return 0;
}
