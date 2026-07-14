#include "lexer.h"
#include "jobParser/errors.h"
#include "jobParser/token.h"
#include <cctype>
#include <charconv>
#include <expected>
#include <system_error>

namespace {

    inline std::expected< float, std::errc > parseNumber( std::string_view sv ) 
    {
        float num;
        const char *begin = sv.data();
        const char *end = sv.data() + sv.length();

        auto [ ptr, ec ] = std::from_chars( begin, end, num );

        if ( ec == std::errc{} ) {
            if ( ptr != end )
                return std::unexpected< std::errc > ( std::errc::invalid_argument );
            return num;
        }
        return std::unexpected< std::errc >( ec );
    }

};

std::expected< Json::TokenWrapper, Json::Error > Json::Lexer::computeNextToken() noexcept
{
    auto reader = Json::Lexer::SafeReader( *this );

    auto c = reader.consume();
    if ( !c ) {
        if ( c.error() == Json::Error::UnexpectedEof )
            return Token::Eof;
        return std::unexpected( c.error() );
    }
    while ( std::isspace( *c ) ) {
        c = reader.consume();
        if ( !c ) {
            if ( c.error() == Json::Error::UnexpectedEof )
                return Token::Eof;
            return std::unexpected( c.error() );
        }
    }

    if ( c == '"' ) {
        auto populator = Json::Lexer::ScratchPopulator( *this );
        c = reader.consume();
        if ( !c )
            return std::unexpected( c.error() );

        while ( c != '"' ) {
            populator.add( *c );
            c = reader.consume();
            if ( !c )
                return std::unexpected( c.error() );
        }

        auto view = populator.toView();

        auto tok = graphKeywordToToken( view );
        if ( tok )
            return *tok;
        return view;
    }

    if ( c == 'n' || c == 't' || c == 'f' ) {
        auto populator = Json::Lexer::ScratchPopulator( *this );
        populator.add( *c );

        auto peek = reader.peek();

        if ( !peek )
            return std::unexpected( peek.error() );

        while ( !std::isspace(*peek) ) {
            if ( *peek == '[' || *peek == ']' ||
                 *peek == '{' || *peek == '}' ||
                 *peek == ',' || *peek == ':' )
                break;

            populator.add( *peek );

            c = reader.consume();
            if ( !c )
                return std::unexpected( c.error() );

            peek = reader.peek();
            if ( !peek ) {
                if ( peek.error() == Json::Error::UnexpectedEof ) 
                    break;
                return std::unexpected( peek.error() );
            }
        }

        auto tok = jsonKeywordToToken( populator.toView() );
        if ( tok )
            return *tok;
        return std::unexpected( Json::Error::UnknownToken );
    }

    if ( std::isdigit( *c ) || c == '-' ) {
        auto populator = Json::Lexer::ScratchPopulator( *this );
        populator.add( *c );
        auto peek = reader.peek();

        if ( !peek )
            return std::unexpected( peek.error() );

        while ( !std::isspace(*peek) ) {
            if ( *peek == '[' || *peek == ']' ||
                 *peek == '{' || *peek == '}' ||
                 *peek == ',' || *peek == ':' )
                break;

            populator.add( *peek );

            c = reader.consume();
            if ( !c )
                return std::unexpected( c.error() );

            peek = reader.peek();
            if ( !peek ) {
                if ( peek.error() == Json::Error::UnexpectedEof ) 
                    break;
                return std::unexpected( peek.error() );
            }
        }

        auto num = parseNumber( populator.toView() );
        if ( !num )
            return std::unexpected( Json::Error::InvalidNumber );

        return *num;
    }

    switch ( *c ) {
        case ':':
            return Json::Token::Colon;
        case ',':
            return Json::Token::Comma;
        case '[':
            return Json::Token::LeftBracket;
        case '{':
            return Json::Token::LeftBrace;
        case ']':
            return Json::Token::RightBracket;
        case '}':
            return Json::Token::RightBrace;
        default:
            return std::unexpected( Json::Error::UnknownToken );
    }
}
