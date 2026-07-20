#include "lexer.h"
#include "jobParser/errors.h"
#include "jobParser/token.h"
#include <charconv>
#include <system_error>

namespace {

    inline bool parseNumberInt( std::string_view sv, int &num ) 
    {
        const char *begin = sv.data();
        const char *end = sv.data() + sv.length();

        auto [ ptr, ec ] = std::from_chars( begin, end, num );

        if ( ec == std::errc{} )
            return ptr == end;
        return false;
    }

    inline bool parseNumberFloat( std::string_view sv, double &num ) 
    {
        const char *begin = sv.data();
        const char *end = sv.data() + sv.length();

        auto [ ptr, ec ] = std::from_chars( begin, end, num );

        if ( ec == std::errc{} )
            return ptr == end;
        return false;
    }

    inline bool isSpace(char c) noexcept
    {
        switch ( c ) {
            case ' ':
            case '\n':
            case '\r':
            case '\t':
                return true;
            default:
                return false;
        }
    }

    inline bool isDigit(char c) noexcept
    {
        return static_cast< unsigned char > ( c - '0' ) < 10;
    }

    static constexpr std::array< Json::Token, 256 > buildCharTable() {
        std::array< Json::Token, 256 > table{};
        table.fill( Json::Token::Invalid );
        table[ ':' ] = Json::Token::Colon;
        table[ ',' ] = Json::Token::Comma;
        table[ '[' ] = Json::Token::LeftBracket;
        table[ '{' ] = Json::Token::LeftBrace;
        table[ ']' ] = Json::Token::RightBracket;
        table[ '}' ] = Json::Token::RightBrace;
        table[ '\0' ] = Json::Token::Eof;
        return table;
    }

};

Json::TokenWrapper Json::Lexer::computeNextToken() noexcept
{
    static constexpr auto charTable = buildCharTable();
    char c;
    while ( isSpace( c = peekChar() ) )
        consumeChar();

    if ( c == '"' ) {
        consumeChar();

        auto populator = Json::Lexer::ScratchPopulator( *this );

        while ( peekChar() != '"' )
            consumeChar( &populator );
        /* Consume last " */
        consumeChar( &populator );

        /* Ignore the last "! */
        auto view = populator.toView( true );
        auto tok = graphKeywordToToken( view );
        if ( tok != Token::Invalid )
            return tok;

        return view;
    }

    if ( c == 'n' || c == 't' || c == 'f' ) {
        auto populator = Json::Lexer::ScratchPopulator( *this );
        consumeChar( &populator );

        char peek = peekChar();
        while ( !isSpace(peek) ) {
            if ( charTable[ std::size_t( peek ) ] != Token::Invalid )
                break;

            c = consumeChar( &populator );
            peek = peekChar();
        }

        auto tok = jsonKeywordToToken( populator.toView() );
        if ( tok == Token::Invalid )
            m_error = Error::UnknownToken;
        return tok;
    }

    if ( isDigit( c ) || c == '-' ) {
        auto populator = Json::Lexer::ScratchPopulator( *this );
        consumeChar( &populator );

        auto peek = peekChar();
        bool isFloat = false;

        while ( !isSpace(peek) ) {
            if ( peek == '.' || peek == 'f' || peek == 'F' || peek == 'e' || peek == 'E' )
                isFloat = true;

            if ( charTable[ std::size_t( peek ) ] != Token::Invalid )
                break;

            c = consumeChar( &populator );
            peek = peekChar();
        }

        double ret;

        if ( isFloat ) {
            double &num = ret;
            if ( !parseNumberFloat( populator.toView(), num ) ) {
                m_error = Error::InvalidNumber;
                return Token::Invalid;
            }
        } else {
            int num;
            if ( !parseNumberInt( populator.toView(), num ) ) {
                m_error = Error::InvalidNumber;
                return Token::Invalid;
            }
            ret = num;
        }

        return ret;
    }

    auto t = charTable[ std::size_t( c ) ];
    consumeChar();
    if ( t == Token::Invalid )
        m_error = Error::UnknownToken;
    return t;
}
