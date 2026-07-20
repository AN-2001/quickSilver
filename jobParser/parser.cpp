#include "parser.h"
#include "jobParser/errors.h"
#include "jobParser/parserEvents.h"
#include "jobParser/token.h"
#include <cstdint>
#include <utility>

bool Json::Parser::parseJobType() noexcept
{
    if ( !expect( Json::Token::JobType ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }
    if ( !expect( Json::Token::Colon ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    auto nextTok = m_lexer.get();

    switch ( nextTok.m_type ) {
        case Token::Metrics:
        case Token::Compute:
            addEvent( ParserEvent( ParserEventType::SetJobType, std::to_underlying( nextTok.m_type ) ) );
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
        case Token::Invalid:
        default:
            m_error = Error::UnexpectedToken;
            return false;
    }

    return true;
}

bool Json::Parser::parseAlgorithm() noexcept
{
    if ( !expect( Json::Token::Algorithm ) ) {
        m_error = Json::Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::Colon ) ) {
        m_error = Json::Error::UnexpectedToken;
        return false;
    }

    auto nextTok = m_lexer.get();
    switch ( nextTok.m_type ) {
        case Token::Dfs:
        case Token::Bfs:
            addEvent( ParserEvent( ParserEventType::SetAlgorithm, std::to_underlying( nextTok.m_type ) ) );
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
        case Token::Invalid:
        default:
            m_error = Error::UnexpectedToken;
            return false;
    }

    return true;
}

bool Json::Parser::parseGraphNumVertices() noexcept 
{
    if ( !expect( Json::Token::NumVertices ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::Colon ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    auto nextTok = m_lexer.get();
    if ( nextTok.m_type != Json::Token::Number ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    addEvent( Json::ParserEvent( Json::ParserEventType::SetVertexCount, static_cast<uint16_t>( nextTok.getNumericValue() ) ) );
    return true;
}

bool Json::Parser::parseGraphEdges() noexcept 
{
    uint16_t edgeCount = 0;
    uint16_t currentSrc = 0;

    if ( !expect( Json::Token::Edges ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::Colon ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::LeftBracket ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    auto nextTok = m_lexer.peek();
    if ( nextTok.m_type == Json::Token::RightBracket ) {
        m_lexer.get();
        addEvent( Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 0 ) );
        return true;
    }

    while ( true ) {
        bool skip = false;

        if ( !expect( Json::Token::LeftBracket ) ) {
            m_error = Error::UnexpectedToken;
            return false;
        }

        nextTok = m_lexer.peek();
        if ( nextTok.m_type == Json::Token::RightBracket ) {
            m_lexer.get();
            skip = true;
        }

        while ( !skip ) {
            nextTok = m_lexer.get();
            if ( nextTok.m_type != Json::Token::Number ) {
                m_error = Error::UnexpectedToken;
                return false;
            }

            uint16_t dst = static_cast<uint16_t>( nextTok.getNumericValue() );
            addEvent( Json::ParserEvent( Json::ParserEventType::AddEdge, currentSrc, dst ) );
            edgeCount++;

            nextTok = m_lexer.peek();
            if ( nextTok.m_type == Json::Token::RightBracket ) {
                m_lexer.get();
                break;
            }

            if ( !expect( Json::Token::Comma ) ) {
                m_error = Error::UnexpectedToken;
                return false;
            }
        }


        nextTok = m_lexer.peek();
        if ( nextTok.m_type == Json::Token::RightBracket ) {
            m_lexer.get();
            break;
        }

        currentSrc++;

        if ( !expect( Json::Token::Comma ) ) {
            m_error = Error::UnexpectedToken;
            return false;
        }

    }

    addEvent( Json::ParserEvent( Json::ParserEventType::SetEdgeCount, edgeCount ) ); 
    return true;
}

bool Json::Parser::parseGraphLabels() noexcept 
{
    uint16_t labelCount = 0;
    if ( !expect( Json::Token::Labels ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }
    if ( !expect( Json::Token::Colon ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::LeftBracket ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    auto nextTok = m_lexer.peek();
    if ( nextTok.m_type == Json::Token::RightBracket ) {
        m_lexer.get();
        addEvent( Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ) ); 
        return true;
    }

    while ( true ) {
        nextTok = m_lexer.get();
        if ( nextTok.m_type != Json::Token::Label ) {
            m_error = Error::UnexpectedToken;
            return false;
        }

        auto index = addString( nextTok.getLabelValue() );

        addEvent( ParserEvent( ParserEventType::AddLabel, index ) );

        labelCount++;
        nextTok = m_lexer.peek();

        if ( nextTok.m_type == Token::RightBracket ) {
            m_lexer.get();
            break;
        }

        if ( !expect( Token::Comma ) ) {
            m_error = Error::UnexpectedToken;
            return false;
        }
    }

    addEvent( ParserEvent( ParserEventType::SetLabelCount, labelCount ) );
    return true;
}

bool Json::Parser::parseGraph() noexcept
{
    if ( !expect( Json::Token::Graph ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::Colon ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::LeftBrace ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    while ( true ) {
        auto nextTok = m_lexer.peek();
        switch ( nextTok.m_type ) {
            case Token::NumVertices:
                if ( !parseGraphNumVertices() )
                    return false;
                break;
            case Token::Edges:
                if ( !parseGraphEdges() )
                    return false;
                break;
            case Token::Labels:
                if ( !parseGraphLabels() )
                    return false;
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
            case Token::Invalid:
            default:
                m_error = Error::UnexpectedToken;
                return false;
        }

        nextTok = m_lexer.peek();
        if ( nextTok.m_type == Token::RightBrace ) {
            m_lexer.get();
            break;
        }

        if ( !expect( Json::Token::Comma ) ) {
            m_error = Error::UnexpectedToken;
            return false;
        }
    }


    return true;
}

bool Json::Parser::parseInput() noexcept
{
    uint16_t inputCount = 0;
    if ( !expect( Json::Token::Input ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::Colon ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    if ( !expect( Json::Token::LeftBracket ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    auto nextTok = m_lexer.peek();
    if ( nextTok.m_type == Json::Token::RightBracket ) {
        m_lexer.get();
        addEvent( Json::ParserEvent( Json::ParserEventType::SetInputCount, 0 ) ); 
        return true;
    }

    while ( true ) {
        nextTok = m_lexer.get();
        if ( nextTok.m_type != Json::Token::Number ) {
            m_error = Error::UnexpectedToken;
            return false;
        }

        uint16_t val = static_cast< uint16_t >( nextTok.getNumericValue() );
        addEvent( Json::ParserEvent( ParserEventType::AddInput, val ) );
        inputCount++;


        nextTok = m_lexer.peek();
        if ( nextTok.m_type == Token::RightBracket ) {
            m_lexer.get();
            break;
        }

        if ( !expect( Json::Token::Comma ) ) {
            m_error = Error::UnexpectedToken;
            return false;
        }
    }

    addEvent( Json::ParserEvent( ParserEventType::SetInputCount, inputCount ) ); 
    return true;
}

bool Json::Parser::parseJob() noexcept
{
    if ( !expect( Json::Token::LeftBrace ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }
    while ( true ) {
        auto nextTok = m_lexer.peek();
        switch ( nextTok.m_type ) {
            case Token::JobType:
                if ( !parseJobType() )
                    return false;
                break;
            case Token::Algorithm:
                if ( !parseAlgorithm() )
                    return false;
                break;
            case Token::Graph:
                if ( !parseGraph() )
                    return false;
                break;
            case Token::Input:
                if ( !parseInput() )
                    return false;
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
            case Token::Invalid:
            default:
                m_error = Error::UnexpectedToken;
                return false;
        }

        nextTok = m_lexer.peek();
        if ( nextTok.m_type == Token::RightBrace ) {
            m_lexer.get();
            break;
        }


        if ( !expect( Json::Token::Comma ) ) {
            m_error = Error::UnexpectedToken;
            return false;
        }
    }

    return true;
}

bool Json::Parser::parse() noexcept
{
    if ( !parseJob() )
        return false;
    if ( !expect( Json::Token::Eof ) ) {
        m_error = Error::UnexpectedToken;
        return false;
    }

    addEvent( Json::ParserEvent( ParserEventType::Finish ) ); 

    return true;
}
