#include "jobPipeline.h"
#include "jobBuilder/jobBuilder.h"
#include "jobBuilder/jobValidator.h"
#include "jobParser/errors.h"
#include "jobParser/lexer.h"
#include "jobParser/parser.h"
#include "utils/jobState.h"
#include "algorithms/bfs.h"
#include "algorithms/dfs.h"
#include "utils/serializer.h"

void JobTools::JobPipeline::execute() noexcept
{
    Json::Lexer lexer( m_job );
    Json::Parser parser( lexer, m_allocator );
    JobTools::Validator validator( parser );
    JobTools::Builder builder( parser, m_allocator );

    auto err = parser.parse();
    if ( err != Json::Error::NoError ) {
        Utils::Serializer serializer( m_job );
        serializer << R"JSON({"status":"syntax error","error":")JSON";
        serializer << Json::errorToString( err );
        serializer << R"JSON("})JSON";
        return;
    }

    if ( !validator.validate() ) {
        Utils::Serializer serializer( m_job );
        serializer << R"JSON({"status":"Job error","error":"Invalid job format"})JSON";
        return;
    }

    m_job.m_jobState = builder.build();

    if ( m_job.m_jobState.type == Utils::JobType::Metrics ) {
        Utils::Serializer serializer( m_job );
        serializer << R"JSON({"status":"not supported","error":"Metrics jobs aren't supported"})JSON";
        return;
    }

    switch ( m_job.m_jobState.algorithm ) {
        case Utils::AlgorithmType::BFS: {
            Algorithms::Bfs bfs( m_allocator, m_job.m_jobState.graph,  m_job.m_jobState.inputs, m_job.m_jobState.numInputs );
            bfs.run();
            bfs.serialize( m_job );
            break;
        }
        case Utils::AlgorithmType::DFS: {
            Algorithms::Dfs dfs( m_allocator, m_job.m_jobState.graph,  m_job.m_jobState.inputs, m_job.m_jobState.numInputs );
            dfs.run();
            dfs.serialize( m_job );
            break;
        }
        default: {
            Utils::Serializer serializer( m_job );
            serializer << R"JSON({"status":"Unrecognized algorithm"})JSON";
        }
    }

}
