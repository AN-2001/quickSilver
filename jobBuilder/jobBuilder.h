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

        public:
            Builder( T &container,
                     Utils::Allocator &allocator ) 
                : m_container( container ),
                  m_allocator( allocator )
            {}

            void processEvent( Utils::JobState &job, JobAuxState &aux, const Json::ParserEvent &event ) const noexcept
            {
                switch( event.m_type ) {
                    case Json::ParserEventType::SetJobType:
                        if ( event.m_ident0 == std::to_underlying( Json::Token::Compute ) )
                            job.type = Utils::JobType::Compute;
                        else
                            job.type = Utils::JobType::Metrics;
                        break;
                    case Json::ParserEventType::SetAlgorithm:
                        if ( event.m_ident0 == std::to_underlying( Json::Token::Bfs ) )
                            job.algorithm = Utils::AlgorithmType::BFS;
                        else if ( event.m_ident0 == std::to_underlying( Json::Token::Dfs ) )
                            job.algorithm = Utils::AlgorithmType::DFS;
                        break;
                    case Json::ParserEventType::SetInputCount:
                        job.numInputs = event.m_ident0;
                        break;
                    case Json::ParserEventType::AddInput:
                        job.inputs[ aux.currentInput++ ] = event.m_ident0;
                        break;
                    case Json::ParserEventType::SetVertexCount:
                        job.graph.numVertices = event.m_ident0;
                        job.graph.offsets = Utils::makeArrayView<std::uint16_t>( m_allocator, event.m_ident0 + 1 );
                        break;
                    case Json::ParserEventType::SetEdgeCount:
                        job.graph.adj = Utils::makeArrayView<std::uint16_t>( m_allocator, event.m_ident0 );
                        break;
                    case Json::ParserEventType::AddEdge:
                        for ( ; aux.currentSrcVertex <= event.m_ident0; ++aux.currentSrcVertex )
                            job.graph.offsets[ static_cast< std::size_t >( aux.currentSrcVertex ) ] = static_cast<uint16_t>( aux.currentEdge );
                        job.graph.adj[ aux.currentEdge++ ] = event.m_ident1;
                        break;
                    case Json::ParserEventType::SetLabelCount:
                        job.graph.labels = Utils::makeArrayView<std::uint16_t>( m_allocator, event.m_ident0 );
                        break;
                    case Json::ParserEventType::AddLabel:
                        job.graph.labels[ aux.currentLabel++ ] = event.m_ident0;
                        break;
                    case Json::ParserEventType::EmptyEvent:
                    case Json::ParserEventType::NumEvents:
                    case Json::ParserEventType::Finish:
                        for ( ; static_cast< std::size_t > ( aux.currentSrcVertex ) <= job.graph.numVertices; ++aux.currentSrcVertex )
                            job.graph.offsets[ static_cast< std::size_t >( aux.currentSrcVertex ) ] = static_cast<uint16_t>( aux.currentEdge );
                        break;

                    default:
                        break;
                }

            }

            [[nodiscard]] Utils::JobState build() const noexcept
            {
                Utils::JobState ret{};
                JobAuxState aux{};
                for ( const auto &event : m_container ) {
                    processEvent( ret, aux, event );
                    if ( event.m_type == Json::ParserEventType::SetJobType &&
                            event.m_ident0 == std::to_underlying( Json::Token::Metrics ) )
                        return ret;
                }
                ret.strings = m_container.releaseStrings();
                return ret;
            }
    };

}
