#include "parser.h"
#include "jsonParser/object.h"
#include "jsonParser/token.h"
#include <memory>


namespace Json {

    void Parser::expect( Token::Type tokenType ) 
    {
        Token token = lexer.getToken();
        if ( token != tokenType )
            throw JsonParseError( "JsonParseError: Unexpected token " + std::string( token ) );
    }

    std::string Parser::expectString()
    {
        Token token = lexer.getToken();
        if ( token != Token::Type::String )
            throw JsonParseError( "JsonParseError: Unexpected token " + std::string( token ) );
        return static_cast< std::string > ( token );
    }


    Object Parser::parseMap()
    {
        Object obj;
        std::string key;
        bool first = true;
        expect( Token::Type::LeftBrace );
        while ( lexer.peekToken() != Token::Type::RightBrace ) {
            if ( first ) {
                key = expectString();
                expect( Token::Type::Colon );
                obj[ key ] = parseJson();
                first = false;
            } else {
                expect( Token::Type::Comma );
                key = expectString();
                expect( Token::Type::Colon );
                obj[ key ] = parseJson();
            }
        }
        expect( Token::Type::RightBrace );
        return obj;
    }

    Object Parser::parseArray()
    {
        Object obj;
        bool first = true;

        expect( Token::Type::LeftBracket );
        while ( lexer.peekToken() != Token::Type::RightBracket ) {
            if ( first ) {
                obj.push_back( parseJson() );
                first = false;
            } else {
                expect( Token::Type::Comma );
                obj.push_back( parseJson() );
            }
        }
        expect( Token::Type::RightBracket );
        return obj;
    }

    Object Parser::parseJson()
    {
        if ( lexer.peekToken() == Token::Type::Boolean ) 
            return Object( (bool) lexer.getToken() );
        if ( lexer.peekToken() == Token::Type::String ) 
            return Object( (std::string) lexer.getToken() );
        if ( lexer.peekToken() == Token::Type::Number ) 
            return Object( (double) lexer.getToken() );
        if ( lexer.peekToken() == Token::Type::Null ) 
            return Object();
        if ( lexer.peekToken() == Token::Type::LeftBracket ) 
            return parseArray();
        if ( lexer.peekToken() == Token::Type::LeftBrace ) 
            return parseMap();
        expect( Token::Type::Eof );
        return Object();
    }

    void Parser::parse() 
    {
        job.json = std::make_unique< Object > ( parseJson() );
        expect( Token::Type::Eof );
    }

}

