#include "jobBuilder/jobValidator.h"
#include "jobBuilder/jobBuilder.h"
#include "jobParser/lexer.h"
#include "jobParser/parser.h"
#include "algorithms/bfs.h"
#include "utils/job.h"
#include "utils/managedFd.h"
#include <cstdio>
#include <print>

int main()
{
    Utils::Job job( Utils::BorrowedFd{ 0 }, Utils::BorrowedFd{ 1 } );
    Json::Lexer lexer( job );
    Json::Parser parser( lexer );
    JobTools::Validator validator( parser );
    std::println( "Error was: {}", Json::errorToString( parser.parse() ) );

    if ( !validator.validate() )
        return 1;

    JobTools::Builder builder( parser );
    job.m_jobState = builder.build();

    Algorithms::Bfs bfs( job.m_jobState.graph, job.m_jobState.inputs, job.m_jobState.numInputs );
    bfs.run();
    bfs.serialize( job );
    std::puts("");
}
