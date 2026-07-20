#pragma once

#include "jobParser/parserEvents.h"
#include "jobParser/token.h"
#include "utils/allocator.h"
#include "utils/arrayView.h"
#include "utils/jobState.h"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace JobTools {

    template <typename T>
        class Builder {
            private:
                T &m_container;
                Utils::Allocator &m_allocator;

                struct JobAuxState {
                    std::size_t currentInput;
                    std::size_t currentLabel;
                    std::size_t currentEdge;
                    std::size_t currentSrcVertex;
                };

                Utils::JobState &m_job;
                JobAuxState m_aux{};

            public:
                Builder( T &container,
                        Utils::Allocator &allocator,
                        Utils::JobState &job ) 
                    : m_container( container ),
                    m_allocator( allocator ),
                    m_job( job )
                {}

                void build( const Json::ParserEvent &event ) noexcept
                {

                    switch( event.m_type ) {
                        case Json::ParserEventType::SetJobType:
                            if ( event.m_ident0 == std::to_underlying( Json::Token::Compute ) )
                                m_job.type = Utils::JobType::Compute;
                            else
                                m_job.type = Utils::JobType::Metrics;
                            break;
                        case Json::ParserEventType::SetAlgorithm:
                            if ( event.m_ident0 == std::to_underlying( Json::Token::Bfs ) )
                                m_job.algorithm = Utils::AlgorithmType::BFS;
                            else if ( event.m_ident0 == std::to_underlying( Json::Token::Dfs ) )
                                m_job.algorithm = Utils::AlgorithmType::DFS;
                            break;
                        case Json::ParserEventType::SetInputCount:
                            m_job.numInputs = event.m_ident0;
                            break;
                        case Json::ParserEventType::AddInput:
                            m_job.inputs[ m_aux.currentInput++ ] = event.m_ident0;
                            break;
                        case Json::ParserEventType::SetVertexCount:
                            m_job.graph.numVertices = event.m_ident0;
                            m_job.graph.offsets = Utils::makeArrayView<std::uint16_t>( m_allocator, event.m_ident0 + 1 );
                            break;
                        case Json::ParserEventType::SetEdgeCount:
                            m_job.graph.adj = Utils::makeArrayView<std::uint16_t>( m_allocator, event.m_ident0 );
                            break;
                        case Json::ParserEventType::AddEdge:
                            for ( ; m_aux.currentSrcVertex <= event.m_ident0; ++m_aux.currentSrcVertex )
                                m_job.graph.offsets[ static_cast< std::size_t >( m_aux.currentSrcVertex ) ] = static_cast<uint16_t>( m_aux.currentEdge );
                            m_job.graph.adj[ m_aux.currentEdge++ ] = event.m_ident1;
                            break;
                        case Json::ParserEventType::SetLabelCount:
                            m_job.graph.labels = Utils::makeArrayView<std::uint16_t>( m_allocator, event.m_ident0 );
                            break;
                        case Json::ParserEventType::AddLabel:
                            m_job.graph.labels[ m_aux.currentLabel++ ] = event.m_ident0;
                            break;
                        case Json::ParserEventType::EmptyEvent:
                        case Json::ParserEventType::NumEvents:
                        case Json::ParserEventType::Finish:
                            if ( m_job.type == Utils::JobType::Metrics )
                                break;

                            for ( ; static_cast< std::size_t > ( m_aux.currentSrcVertex ) <= m_job.graph.numVertices; ++m_aux.currentSrcVertex )
                                m_job.graph.offsets[ static_cast< std::size_t >( m_aux.currentSrcVertex ) ] = static_cast<uint16_t>( m_aux.currentEdge );
                            m_job.strings = m_container.releaseStrings();
                            break;

                        default:
                            break;
                    }
                }
        };

}
