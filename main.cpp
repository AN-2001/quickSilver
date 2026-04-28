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
    GraphToys::JsonObject obj;

    obj[ "based" ] = "A.N";
    obj[ "retards" ][ 0 ] = "Nadav" ;
    obj[ "retards" ][ 1 ] = "Amir" ;
    std::cout << obj.serialize() << std::endl;
    return 0;
}
