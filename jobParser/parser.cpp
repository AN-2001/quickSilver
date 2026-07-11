#include "parser.h"
#include "jobParser/errors.h"
#include "jobParser/parserEvents.h"
#include "jobParser/token.h"
#include <algorithm>
#include <cstdint>
#include <utility>

Json::Error Json::Parser::parseJobType() noexcept
{
    if ( auto err = expect( Json::Token::JobType ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;

    auto nextTok = m_lexer.get();
    if ( !nextTok )
        return nextTok.error();

    switch ( nextTok -> m_type ) {
        case Token::Metrics:
        case Token::Compute:
            if ( auto err = addEvent( ParserEvent( ParserEventType::SetJobType, std::to_underlying( nextTok -> m_type ) ) ); err != Json::Error::NoError )
                return err;
            break;

        case Token::JobType:
        case Token::Dfs:
        case Token::Bfs:
        case Token::Algorithm:
        case Token::Graph:
        case Token::Input:
        case Token::NumVertices:
        case Token::Edges:
        case Token::Labels:
        case Token::Number:
        case Token::Label:
        case Token::LeftBracket:
        case Token::RightBracket:
        case Token::LeftBrace:
        case Token::RightBrace:
        case Token::Comma:
        case Token::Colon:
        case Token::Null:
        case Token::True:
        case Token::False:
        case Token::Eof:
        case Token::NumTokens:
            return Json::Error::UnexpectedToken;
        default:
            return Json::Error::UnexpectedToken; /* TODO: this is catastrophic, report it better. */
    }

    return Json::Error::NoError;

}

Json::Error Json::Parser::parseAlgorithm() noexcept
{
    if ( auto err = expect( Json::Token::Algorithm ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;
    auto nextTok = m_lexer.get();
    if ( !nextTok )
        return nextTok.error();
    switch ( nextTok -> m_type ) {
        case Token::Dfs:
        case Token::Bfs:
            if ( auto err = addEvent( ParserEvent( ParserEventType::SetAlgorithm, std::to_underlying( nextTok -> m_type ) ) ); err != Json::Error::NoError )
                return err;
            break;

        case Token::JobType:
        case Token::Metrics:
        case Token::Compute:
        case Token::Algorithm:
        case Token::Graph:
        case Token::Input:
        case Token::NumVertices:
        case Token::Edges:
        case Token::Labels:
        case Token::Number:
        case Token::Label:
        case Token::LeftBracket:
        case Token::RightBracket:
        case Token::LeftBrace:
        case Token::RightBrace:
        case Token::Comma:
        case Token::Colon:
        case Token::Null:
        case Token::True:
        case Token::False:
        case Token::Eof:
        case Token::NumTokens:
            return Json::Error::UnexpectedToken;
        default:
            return Json::Error::UnexpectedToken; /* TODO: this is catastrophic, report it better. */
    }

    return Json::Error::NoError;
}

Json::Error Json::Parser::parseGraphNumVertices() noexcept 
{
    if ( auto err = expect( Json::Token::NumVertices ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;

    auto nextTok = m_lexer.get();
    if ( !nextTok )
        return nextTok.error();

    if ( nextTok -> m_type != Json::Token::Number )
        return Json::Error::UnexpectedToken;
    if ( auto err = addEvent( Json::ParserEvent( Json::ParserEventType::SetVertexCount, static_cast<uint16_t>( nextTok -> getNumericValue() ) ) ); err != Json::Error::NoError )
        return err;
    return Json::Error::NoError;
}

Json::Error Json::Parser::parseGraphEdges() noexcept 
{
    uint16_t edgeCount = 0;

    if ( auto err = expect( Json::Token::Edges ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;

    if ( auto err = expect( Json::Token::LeftBracket ); err != Json::Error::NoError )
        return err;

    auto nextTok = m_lexer.peek();
    if ( !nextTok )
        return nextTok.error();
    if ( nextTok -> m_type == Json::Token::RightBracket ) {
        if ( auto err = addEvent( Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 0 ) ); err != Json::Error::NoError )
            return err;
        if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
            return err;
        return Json::Error::NoError;
    }

    while ( true ) {

        if ( auto err = expect( Json::Token::LeftBracket ); err != Json::Error::NoError )
            return err;


        nextTok = m_lexer.get();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok -> m_type != Json::Token::Number ) 
            return Json::Error::UnexpectedToken;
        uint16_t src = static_cast<uint16_t>( nextTok -> getNumericValue() );
        if ( auto err = expect( Json::Token::Comma ); err != Json::Error::NoError )
            return err;
        nextTok = m_lexer.get();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok -> m_type != Json::Token::Number ) 
            return Json::Error::UnexpectedToken;
        uint16_t dst = static_cast<uint16_t>( nextTok -> getNumericValue() );
        if ( auto err = addEvent( Json::ParserEvent( Json::ParserEventType::AddEdge, src, dst ) ); err != Json::Error::NoError )
            return err;

        edgeCount++;

        if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
            return err;

        nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok != Token::Comma )
            break;
        if ( auto err = expect( Json::Token::Comma ); err != Json::Error::NoError )
            return err;
    }

    if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
        return err;

    if ( auto err = addEvent( Json::ParserEvent( Json::ParserEventType::SetEdgeCount, edgeCount ) ); err != Json::Error::NoError )
        return err;
    return Json::Error::NoError;
}

Json::Error Json::Parser::parseGraphLabels() noexcept 
{
    uint16_t labelCount = 0;
    if ( auto err = expect( Json::Token::Labels ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;

    if ( auto err = expect( Json::Token::LeftBracket ); err != Json::Error::NoError )
        return err;

    auto nextTok = m_lexer.peek();
    if ( !nextTok )
        return nextTok.error();
    if ( nextTok -> m_type == Json::Token::RightBracket ) {
        if ( auto err = addEvent( Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ) ); err != Json::Error::NoError )
            return err;
        if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
            return err;
        return Json::Error::NoError;
    }

    while ( true ) {
        nextTok = m_lexer.get();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok -> m_type != Json::Token::Label )
            return Json::Error::UnexpectedToken;

        auto index = addString( nextTok -> getLabelValue() );
        if ( !index )
            return index.error();
        if ( auto err = addEvent( ParserEvent( ParserEventType::AddLabel, *index ) ); err != Json::Error::NoError )
            return err;
        labelCount++;
        nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok != Token::Comma )
            break;
        if ( auto err = expect( Json::Token::Comma ); err != Json::Error::NoError )
            return err;
    }

    if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
        return err;

    if ( auto err = addEvent( ParserEvent( ParserEventType::SetLabelCount, labelCount ) ); err != Json::Error::NoError )
        return err;
    return Json::Error::NoError;
}

Json::Error Json::Parser::parseGraph() noexcept
{
    if ( auto err = expect( Json::Token::Graph ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;

    if ( auto err = expect( Json::Token::LeftBrace ); err != Json::Error::NoError )
        return err;
    while ( true ) {
        auto nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        switch ( nextTok -> m_type ) {
            case Token::NumVertices:
                if( auto err = parseGraphNumVertices(); err != Json::Error::NoError )
                    return err;
                break;

            case Token::Edges:
                if( auto err = parseGraphEdges(); err != Json::Error::NoError )
                    return err;
                break;

            case Token::Labels:
                if( auto err = parseGraphLabels(); err != Json::Error::NoError )
                    return err;
                break;

            case Token::JobType:
            case Token::Metrics:
            case Token::Compute:
            case Token::Algorithm:
            case Token::Graph:
            case Token::Input:
            case Token::Dfs:
            case Token::Bfs:
            case Token::Number:
            case Token::Label:
            case Token::LeftBracket:
            case Token::RightBracket:
            case Token::LeftBrace:
            case Token::RightBrace:
            case Token::Comma:
            case Token::Colon:
            case Token::Null:
            case Token::True:
            case Token::False:
            case Token::Eof:
            case Token::NumTokens:
                return Json::Error::UnexpectedToken;
            default:
                return Json::Error::UnexpectedToken; /* TODO: this is catastrophic, report it better. */
        }

        nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok != Token::Comma )
            break;
        if ( auto err = expect( Json::Token::Comma ); err != Json::Error::NoError )
            return err;
    }

    if ( auto err = expect( Json::Token::RightBrace ); err != Json::Error::NoError )
        return err;

    return Json::Error::NoError;
}

Json::Error Json::Parser::parseInput() noexcept
{
    uint16_t inputCount = 0;
    if ( auto err = expect( Json::Token::Input ); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Colon ); err != Json::Error::NoError )
        return err;

    if ( auto err = expect( Json::Token::LeftBracket ); err != Json::Error::NoError )
        return err;

    auto nextTok = m_lexer.peek();
    if ( !nextTok )
        return nextTok.error();
    if ( nextTok -> m_type == Json::Token::RightBracket ) {
        if ( auto err = addEvent( Json::ParserEvent( Json::ParserEventType::SetInputCount, 0 ) ); err != Json::Error::NoError )
            return err;
        if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
            return err;
        return Json::Error::NoError;
    }

    while ( true ) {
        nextTok = m_lexer.get();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok -> m_type != Json::Token::Number ) 
            return Json::Error::UnexpectedToken;
        uint16_t val = static_cast< uint16_t >( nextTok -> getNumericValue() );
        if ( auto err = addEvent( Json::ParserEvent( ParserEventType::AddInput, val ) ); err != Json::Error::NoError )
            return err;
        inputCount++;


        nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok != Token::Comma )
            break;
        if ( auto err = expect( Json::Token::Comma ); err != Json::Error::NoError )
            return err;
    }

    if ( auto err = expect( Json::Token::RightBracket ); err != Json::Error::NoError )
        return err;
    if ( auto err = addEvent( Json::ParserEvent( ParserEventType::SetInputCount, inputCount ) ); err != Json::Error::NoError )
        return err;
    return Json::Error::NoError;
}

Json::Error Json::Parser::parseJob() noexcept
{
    if ( auto err = expect( Json::Token::LeftBrace ); err != Json::Error::NoError )
        return err;
    while ( true ) {
        auto nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        switch ( nextTok -> m_type ) {
            case Token::JobType:
                if( auto err =  parseJobType(); err != Json::Error::NoError )
                    return err;
                break;
            case Token::Algorithm:
                if( auto err =  parseAlgorithm(); err != Json::Error::NoError )
                    return err;
                break;
            case Token::Graph:
                if( auto err =  parseGraph(); err != Json::Error::NoError )
                    return err;
                break;
            case Token::Input:
                if( auto err =  parseInput(); err != Json::Error::NoError )
                    return err;
                break;
            case Token::NumVertices:
            case Token::Edges:
            case Token::Labels:
            case Token::Metrics:
            case Token::Compute:
            case Token::Dfs:
            case Token::Bfs:
            case Token::Number:
            case Token::Label:
            case Token::LeftBracket:
            case Token::RightBracket:
            case Token::LeftBrace:
            case Token::RightBrace:
            case Token::Comma:
            case Token::Colon:
            case Token::Null:
            case Token::True:
            case Token::False:
            case Token::Eof:
            case Token::NumTokens:
                return Json::Error::UnexpectedToken;
            default:
                return Json::Error::UnexpectedToken; /* TODO: this is catastrophic, report it better. */
        }

        nextTok = m_lexer.peek();
        if ( !nextTok )
            return nextTok.error();
        if ( nextTok != Token::Comma )
            break;

        if ( auto err = expect( Json::Token::Comma ); err != Json::Error::NoError )
            return err;
    }

    if ( auto err = expect( Json::Token::RightBrace ); err != Json::Error::NoError )
        return err;
    return Json::Error::NoError;
}

Json::Error Json::Parser::parse() noexcept
{
    if ( auto err = parseJob(); err != Json::Error::NoError )
        return err;
    if ( auto err = expect( Json::Token::Eof ); err != Json::Error::NoError )
        return err;

    uint16_t addEdgeIndex = std::to_underlying( ParserEventType::AddEdge );
    std::sort( m_eventQueue[ addEdgeIndex ].begin(),
               m_eventQueue[ addEdgeIndex ].begin() + m_eventCounts[ addEdgeIndex ],
               []( const auto &e0, const auto &e1 ) -> bool {
                    return e0.m_ident0 < e1.m_ident0;
               });

    if ( auto err = addEvent( Json::ParserEvent( ParserEventType::Finish ) ); err != Json::Error::NoError )
        return err;

    return Json::Error::NoError;
}
