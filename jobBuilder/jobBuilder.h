#pragma once

#include "jobParser/parserEvents.h"
#include "jobParser/token.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
namespace JobTools {

    static constexpr std::size_t MAX_INPUT_SIZE = 32;

    enum class JobType {
        Compute,
        Metrics
    };

    enum class AlgorithmType {
        BFS,
        DFS
    };

    struct GraphCsr {
        std::size_t numVertices;
        std::vector< uint16_t > adj;
        std::vector< int > offsets;
        std::vector< uint16_t > labels;
    };

    struct JobState {
        JobType type;
        AlgorithmType algorithm;
        std::size_t numInputs;
        std::array< uint16_t, MAX_INPUT_SIZE > inputs;
        GraphCsr graph;
        std::vector< std::string > strings;
    };


    template <typename T>
    class Builder {
        private:
            T &m_container;

            struct JobAuxState {
                std::size_t currentInput;
                int currentSrcVertex;
            };

        public:
            Builder( T &container ) 
                : m_container( container ) 
            {}

            void processEvent( JobState &job, JobAuxState &aux, const Json::ParserEvent &event ) const noexcept
            {
                switch( event.m_type ) {
                    case Json::ParserEventType::SetJobType:
                        if ( event.m_ident0 == std::to_underlying( Json::Token::Compute ) )
                            job.type = JobType::Compute;
                        else
                            job.type = JobType::Metrics;
                        break;
                    case Json::ParserEventType::SetAlgorithm:
                        if ( event.m_ident0 == std::to_underlying( Json::Token::Bfs ) )
                            job.algorithm = AlgorithmType::BFS;
                        else if ( event.m_ident0 == std::to_underlying( Json::Token::Dfs ) )
                            job.algorithm = AlgorithmType::DFS;
                        break;
                    case Json::ParserEventType::SetInputCount:
                        job.numInputs = event.m_ident0;
                        break;
                    case Json::ParserEventType::AddInput:
                        job.inputs[ aux.currentInput++ ] = event.m_ident0;
                        break;
                    case Json::ParserEventType::SetVertexCount:
                        job.graph.numVertices = event.m_ident0;
                        job.graph.offsets = std::vector( job.graph.numVertices + 1, 0 );
                        break;
                    case Json::ParserEventType::SetEdgeCount:
                        job.graph.adj.reserve( event.m_ident0 );
                        break;
                    case Json::ParserEventType::AddEdge:
                        for ( ; aux.currentSrcVertex != event.m_ident0; ++aux.currentSrcVertex )
                            job.graph.offsets[ static_cast< std::size_t >( aux.currentSrcVertex + 1 ) ] = static_cast<uint16_t>( job.graph.adj.size() );
                        job.graph.adj.push_back( event.m_ident1 );
                        break;
                    case Json::ParserEventType::SetLabelCount:
                        job.graph.labels.reserve( event.m_ident0 );
                        break;
                    case Json::ParserEventType::AddLabel:
                        job.graph.labels.push_back( event.m_ident0 );
                        break;
                    case Json::ParserEventType::EmptyEvent:
                    case Json::ParserEventType::NumEvents:
                    case Json::ParserEventType::Finish:
                        for ( ; static_cast< std::size_t > ( aux.currentSrcVertex ) != job.graph.numVertices; ++aux.currentSrcVertex )
                            job.graph.offsets[ static_cast< std::size_t >( aux.currentSrcVertex + 1 ) ] = static_cast<uint16_t>( job.graph.adj.size() );
                        break;

                    default:
                        break;
                }

            }

            [[nodiscard]] JobState build() const noexcept
            {
                JobState ret{};
                JobAuxState aux{};
                aux.currentSrcVertex = -1;
                for ( const auto &event : m_container )
                    processEvent( ret, aux, event );
                ret.strings = m_container.releaseStrings();
                return ret;
            }
    };

}
