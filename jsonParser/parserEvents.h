#include <cstdint>

#pragma once

namespace Json {

    enum class ParserEventType : uint8_t {
        EmptyEvent,
        SetJobType,
        SetAlgorithm,
        SetInputCount,
        AddInput,
        SetVertexCount,
        SetEdgeCount,
        AddEdge,
        SetLabelCount,
        AddLabel,
        Finish,
        NumEvents,
    };

    struct ParserEvent {
        uint16_t m_ident0;
        uint16_t m_ident1;
        ParserEventType m_type;

        ParserEvent() = default;

        ParserEvent( ParserEventType type ) 
            : m_ident0( 0 ), m_ident1( 0 ), m_type( type )
        {}

        ParserEvent( ParserEventType type, uint16_t ident0 ) 
            : m_ident0( ident0 ), m_ident1( 0 ),  m_type( type )
        {}

        ParserEvent( ParserEventType type, uint16_t ident0, uint16_t ident1 ) 
            : m_ident0( ident0 ), m_ident1( ident1 ),  m_type( type )
        {}

    };


};
