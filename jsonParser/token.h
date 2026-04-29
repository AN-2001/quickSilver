/******************************************************************************\
*  token.h                                                                     *
*                                                                              *
*  Header for representing tokens that the lexer will emit.                    *
*                                                                              *
*              Written by A.N.                                  23-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once

#include <string>
#include <variant>




namespace GraphToys {

    struct JsonToken {
        enum class Type {
            Eof,
            Null,
            Number,
            String,
            Boolean,
            LeftBrace,
            RightBrace,
            LeftBracket,
            RightBracket,
            Colon,
            Comma
        };

        using Value =
            std::variant< std::monostate,
                          double,
                          bool,
                          std::string >;

        Type type;
        Value value;

        JsonToken( Type type )
            : type( type ), value( std::monostate{} )
            {}

        JsonToken( double number )
            : type( Type::Number ), value( number  )
            {}

        JsonToken( bool boolean )
            : type( Type::Boolean ), value( boolean )
            {}

        JsonToken( const std::string &string )
            : type( Type::String ), value( string )
            {}

        operator std::string() const {

            switch( type ) {
                case GraphToys::JsonToken::Type::Eof:
                    return "EOF";
                case GraphToys::JsonToken::Type::Null:
                    return "NULL";
                case GraphToys::JsonToken::Type::String:
                    return "STRING < " + std::get< std::string > ( value ) + " >";
                case GraphToys::JsonToken::Type::Number:
                    return "NUMBER < " + std::to_string( std::get< double > ( value ) ) + " >" ;
                case GraphToys::JsonToken::Type::Boolean:
                    return "BOOLEAN < " + std::string(std::get< bool > ( value ) ? "true" : "false") + " >" ;
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

            return "";
        }

    };
}

