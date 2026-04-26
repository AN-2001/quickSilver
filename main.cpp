#include <iostream>
#include <memory>
#include "jsonParser/lexer.h"
#include "jsonParser/token.h"
#include "utils/job.h"

std::string tokenTypeToString( GraphToys::JsonToken::Type type )
{
    switch( type ) {
        case GraphToys::JsonToken::Type::Eof:
            return "EOF";
        case GraphToys::JsonToken::Type::Null:
            return "NULL";
        case GraphToys::JsonToken::Type::String:
            return "STRING";
        case GraphToys::JsonToken::Type::Number:
            return "NUMBER";
        case GraphToys::JsonToken::Type::Boolean:
            return "BOOLEAN";
        case GraphToys::JsonToken::Type::LeftBrace:
            return "LEFT_BRACE";
        case GraphToys::JsonToken::Type::RightBrace:
            return "RIGHT_BRACE";
        case GraphToys::JsonToken::Type::LeftBracket:
            return "LEFT_BRACKET";
        case GraphToys::JsonToken::Type::RightBracket:
            return "RIGHT_BRACKET";
        case GraphToys::JsonToken::Type::Comma:
            return "COMMA";
        case GraphToys::JsonToken::Type::Colon:
            return "COLON";
    }
    return "unknown";
}

int main()
{
    GraphToys::Job job( 0, 1, false );
    GraphToys::JsonLexer lexer( job );

    do {
        std::cout << tokenTypeToString( lexer.getToken().type ) << std::endl;
    } while ( lexer.peekToken().type != GraphToys::JsonToken::Type::Eof );
    std::cout << tokenTypeToString( lexer.peekToken().type ) << std::endl;

    return 0;
}
