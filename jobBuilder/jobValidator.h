#pragma once

#include "jobParser/parserEvents.h"
#include "jobParser/token.h"
#include <array>
#include <cstdint>
#include <utility>


namespace JobTools {

    enum class ValidatorState : uint8_t {
        Init,
        SetJobType,
        SetAlgorithm,
        SetInputCount,
        AddInput,
        SetGraphVertexCount,
        SetGraphEdgeCount,
        AddGraphEdge,
        SetGraphLabelCount,
        AddGraphLabel,
        UnexpectedState,
        Finish,
        NumStates
    };

    static constexpr std::size_t NUM_STATES = std::to_underlying( ValidatorState::NumStates );
    static constexpr std::size_t NUM_TRANSITIONS = std::to_underlying( Json::ParserEventType::NumEvents );
    using TransitionTable = std::array< std::array< ValidatorState, NUM_TRANSITIONS >, NUM_STATES >;

    static consteval TransitionTable buildMetricsTransitionTable()
    {
        TransitionTable table{};
        auto addTransition = [ &table ]( ValidatorState src, Json::ParserEventType event, ValidatorState dst ) 
        {
            std::size_t srcIndex = std::to_underlying( src );
            std::size_t eventIndex = std::to_underlying( event );
            table[ srcIndex ][ eventIndex ] = dst;
        };

        for ( auto &state : table )
            for ( auto &transition : state )
                transition = ValidatorState::UnexpectedState;

        addTransition( ValidatorState::Init,
					   Json::ParserEventType::SetJobType,
					   ValidatorState::SetJobType );

        addTransition( ValidatorState::SetJobType,
					   Json::ParserEventType::Finish,
					   ValidatorState::Finish );

        return table;
    }

    static consteval TransitionTable buildComputeTransitionTable()
    {
        TransitionTable table{};
        auto addTransition = [ &table ]( ValidatorState src, Json::ParserEventType event, ValidatorState dst ) 
        {
            std::size_t srcIndex = std::to_underlying( src );
            std::size_t eventIndex = std::to_underlying( event );
            table[ srcIndex ][ eventIndex ] = dst;
        };

        for ( auto &state : table )
            for ( auto &transition : state )
                transition = ValidatorState::UnexpectedState;

        addTransition( ValidatorState::Init,
					   Json::ParserEventType::SetJobType,
					   ValidatorState::SetJobType );

        addTransition( ValidatorState::SetJobType,
					   Json::ParserEventType::SetAlgorithm,
					   ValidatorState::SetAlgorithm );

        addTransition( ValidatorState::SetAlgorithm,
					   Json::ParserEventType::SetInputCount,
					   ValidatorState::SetInputCount );

        addTransition( ValidatorState::SetInputCount,
					   Json::ParserEventType::AddInput,
					   ValidatorState::AddInput );

        addTransition( ValidatorState::AddInput,
					   Json::ParserEventType::AddInput,
					   ValidatorState::AddInput );

        addTransition( ValidatorState::AddInput,
					   Json::ParserEventType::SetVertexCount,
					   ValidatorState::SetGraphVertexCount );

        addTransition( ValidatorState::SetInputCount,
					   Json::ParserEventType::SetVertexCount,
					   ValidatorState::SetGraphVertexCount );

        addTransition( ValidatorState::SetGraphVertexCount,
					   Json::ParserEventType::SetEdgeCount,
					   ValidatorState::SetGraphEdgeCount );

        addTransition( ValidatorState::SetGraphEdgeCount,
					   Json::ParserEventType::AddEdge,
					   ValidatorState::AddGraphEdge );

        addTransition( ValidatorState::AddGraphEdge,
					   Json::ParserEventType::AddEdge,
					   ValidatorState::AddGraphEdge );

        addTransition( ValidatorState::AddGraphEdge,
					   Json::ParserEventType::SetLabelCount,
					   ValidatorState::SetGraphLabelCount );

        addTransition( ValidatorState::SetGraphEdgeCount,
					   Json::ParserEventType::SetLabelCount,
					   ValidatorState::SetGraphLabelCount );

        addTransition( ValidatorState::SetGraphLabelCount,
					   Json::ParserEventType::AddLabel,
					   ValidatorState::AddGraphLabel );

        addTransition( ValidatorState::SetGraphLabelCount,
					   Json::ParserEventType::Finish,
					   ValidatorState::Finish );

        addTransition( ValidatorState::AddGraphLabel,
					   Json::ParserEventType::AddLabel,
					   ValidatorState::AddGraphLabel );

        addTransition( ValidatorState::AddGraphLabel,
					   Json::ParserEventType::Finish,
					   ValidatorState::Finish );

        return table;
    }

    template <typename T>
    class Validator {
        private:
            static constexpr TransitionTable s_computeTransitions = buildComputeTransitionTable();
            static constexpr TransitionTable s_metricsTransitions = buildMetricsTransitionTable();
            T &m_container;

            struct InternalState {
                std::size_t numStrings;

                std::size_t numVertices;
                std::size_t numLabels;
                std::size_t numInputs;
                std::size_t numEdges;

                std::size_t currentInput;
                std::size_t currentLabel;
                std::size_t currentEdge;

                int lastVertex;
            };

        public:
            Validator( T &container ) 
                : m_container( container ) 
            {}


            [[ nodiscard ]] bool validateTransition( InternalState &state,
                    const Json::ParserEvent &event ) const noexcept
            {
                switch ( event.m_type ) {
                    case Json::ParserEventType::SetJobType:
                        if ( event.m_ident0 != std::to_underlying( Json::Token::Compute ) && event.m_ident0 != std::to_underlying( Json::Token::Metrics ) )
                            return false;
                        if ( event.m_ident1 != 0 )
                            return false;
                        return true;
                    case Json::ParserEventType::SetAlgorithm:

                        if ( event.m_ident0 != std::to_underlying( Json::Token::Bfs ) && event.m_ident0 != std::to_underlying( Json::Token::Dfs ) )
                            return false;
                        if ( event.m_ident1 != 0 )
                            return false;

                        return true;
                    case Json::ParserEventType::SetInputCount:
                        if ( event.m_ident1 != 0 )
                            return false;
                        state.numInputs = event.m_ident0;
                        return true;
                    case Json::ParserEventType::AddInput:
                        if ( event.m_ident1 != 0 )
                            return false;
                        if ( state.currentInput >= state.numInputs )
                            return false;
                        state.currentInput++;
                        return true;
                    case Json::ParserEventType::SetVertexCount:
                        if ( event.m_ident1 != 0 )
                            return false;
                        state.numVertices = event.m_ident0;
                        return true;
                    case Json::ParserEventType::SetEdgeCount:
                        if ( event.m_ident1 != 0 )
                            return false;
                        state.numEdges = event.m_ident0;
                        return true;
                    case Json::ParserEventType::AddEdge:
                        if ( state.currentEdge >= state.numEdges )
                            return false;
                        state.currentEdge++;
                        if ( event.m_ident0 >= state.numVertices )
                            return false;
                        if ( event.m_ident1 >= state.numVertices )
                            return false;
                        if ( event.m_ident0 < state.lastVertex )
                            return false;
                        state.lastVertex = event.m_ident0;
                        return true;
                    case Json::ParserEventType::SetLabelCount:
                        if ( event.m_ident1 != 0 )
                            return false;
                        state.numLabels = event.m_ident0;
                        return true;
                    case Json::ParserEventType::AddLabel:
                        if ( event.m_ident1 != 0 )
                            return false;
                        if ( state.currentLabel >= state.numLabels )
                            return false;
                        state.currentLabel++;

                        if ( event.m_ident0 >= state.numStrings )
                            return false;
                        return true;
                    case Json::ParserEventType::Finish:
                        if ( event.m_ident0 != 0 )
                            return false;
                        if ( event.m_ident1 != 0 )
                            return false;
                        if ( state.currentInput != state.numInputs )
                            return false;
                        if ( state.currentEdge != state.numEdges )
                            return false;
                        if ( state.currentLabel != state.numLabels )
                            return false;
                        return true;
                    case Json::ParserEventType::EmptyEvent:
                    case Json::ParserEventType::NumEvents:
                    default:
                        return false;
                }

            }

            [[ nodiscard ]] bool validate() const noexcept
            {
                Validator::InternalState internalState{};
                ValidatorState currentState = ValidatorState::Init;
                auto *transitions = &s_computeTransitions;

                internalState.numStrings = m_container.getStringCount();
                internalState.lastVertex = -1;

                for ( const auto &event : m_container ) {
                    std::size_t currentIndex = std::to_underlying( currentState );
                    std::size_t transitionIndex = std::to_underlying( event.m_type );

                    if ( !validateTransition( internalState, event ) ) 
                        return false;
                    if ( event.m_type == Json::ParserEventType::SetJobType &&
                            event.m_ident0 == std::to_underlying( Json::Token::Metrics ) )
                        transitions = &s_metricsTransitions;

                    currentState = (*transitions)[ currentIndex ][ transitionIndex ];
                    if ( currentState == ValidatorState::UnexpectedState )
                        return false;
                }
                return currentState == ValidatorState::Finish;
            }
    };
}
