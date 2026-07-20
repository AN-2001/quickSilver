#include "jobPipeline.h"
#include "connections/metricsEvents.h"
#include "jobBuilder/jobBuilder.h"
#include "jobBuilder/jobValidator.h"
#include "jobParser/errors.h"
#include "jobParser/lexer.h"
#include "jobParser/parser.h"
#include "utils/jobState.h"
#include "algorithms/bfs.h"
#include "algorithms/dfs.h"
#include "utils/serializer.h"
#include "jobBuilder/timer.h"
#include <chrono>

void JobTools::JobPipeline::execute() noexcept
{
    if ( m_eventQueue ) {
        auto schedTime = std::chrono::steady_clock::now();
        double schedLatency = std::chrono::duration< double >( schedTime - m_job.m_acceptTime ).count();
        m_eventQueue -> push( { Connections::MetricsEventType::SchedJobLatency, {}, schedLatency }  );
    }

    JobTools::Timer jobTimer( m_eventQueue, Connections::MetricsEventType::PostJobLatency );
    Json::Lexer lexer( m_job );
    Json::Parser parser( lexer, m_allocator );
    JobTools::Validator validator( parser );
    JobTools::Builder builder( parser, m_allocator );


    {
        JobTools::Timer parseTimer( m_eventQueue, Connections::MetricsEventType::PostJobParseLatency );
        if ( !parser.parse() ) {
            Utils::Serializer serializer( m_job );
            serializer << R"JSON({"status":"syntax error","error":")JSON";
            serializer << Json::errorToString( parser.error() );
            serializer << R"JSON("})JSON";
            return;
        }
    }

    {
        JobTools::Timer validateTimer( m_eventQueue, Connections::MetricsEventType::PostJobValidateLatency );
        if ( !validator.validate() ) {
            Utils::Serializer serializer( m_job );
            serializer << R"JSON({"status":"Job error","error":"Invalid job format"})JSON";
            return;
        }
    }

    {
        JobTools::Timer buildTimer( m_eventQueue, Connections::MetricsEventType::PostJobBuildLatency );
        m_job.m_jobState = builder.build();
    }

    if ( m_job.m_jobState.type == Utils::JobType::Metrics ) {
        if ( !m_eventQueue ) {
            Utils::Serializer serializer( m_job );
            serializer << R"JSON({"status":"Not supported","error":"Metrics jobs aren't supported."})JSON";
            return;
        }

        m_eventQueue -> push( { Connections::MetricsEventType::MetricsRequest, std::move( m_job ), 0 } );
        return;
    }

    {
        JobTools::Timer algoTimer( m_eventQueue, Connections::MetricsEventType::PostJobAlgoLatency );
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
}
