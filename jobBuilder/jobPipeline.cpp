#include "jobPipeline.h"
#include "jobBuilder/jobBuilder.h"
#include "jobBuilder/jobValidator.h"
#include "jobParser/errors.h"
#include "jobParser/lexer.h"
#include "jobParser/parser.h"
#include "algorithms/bfs.h"
#include "algorithms/dfs.h"
#include "utils/serializer.h"

void JobTools::JobPipeline::execute() noexcept
{
    Json::Lexer lexer( m_job );
    Json::Parser parser( lexer );
    JobTools::Validator validator( parser );
    JobTools::Builder builder( parser );

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
        serializer << R"JSON({"status":"Invalid job format"})JSON";
        return;
    }

    m_job.m_jobState = builder.build();

    if ( m_job.m_jobState.type == JobTools::JobType::Metrics ) {
        Utils::Serializer serializer( m_job );
        serializer << R"JSON({"status":"Metrics jobs aren't supported"})JSON";
        return;
    }

    switch ( m_job.m_jobState.algorithm ) {
        case JobTools::AlgorithmType::BFS: {
            Algorithms::Bfs bfs( m_job.m_jobState.graph,  m_job.m_jobState.inputs, m_job.m_jobState.numInputs );
            bfs.run();
            bfs.serialize( m_job );
            break;
        }
        case JobTools::AlgorithmType::DFS: {
            Algorithms::Dfs dfs( m_job.m_jobState.graph,  m_job.m_jobState.inputs, m_job.m_jobState.numInputs );
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
