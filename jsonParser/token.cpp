#include "token.h"

namespace Json {
    std::string tokenTypeToString( Token::Type type )
    {
        switch( type ) {
            case Token::Type::Eof:
                return "EOF";
            case Token::Type::Null:
                return "NULL";
            case Token::Type::String:
                return "STRING";
            case Token::Type::Number:
                return "NUMBER";
            case Token::Type::Boolean:
                return "BOOLEAN";
            case Token::Type::LeftBrace:
                return "LEFT_BRACE";
            case Token::Type::RightBrace:
                return "RIGHT_BRACE";
            case Token::Type::LeftBracket:
                return "LEFT_BRACKET";
            case Token::Type::RightBracket:
                return "RIGHT_BRACKET";
            case Token::Type::Comma:
                return "COMMA";
            case Token::Type::Colon:
                return "COLON";
        }
        return "UNKNOWN";
    }
}
