/******************************************************************************\
*  token.h                                                                     *
*                                                                              *
*  Header for representing tokens that the lexer will emit.                    *
*                                                                              *
*              Written by A.N.                                  23-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once

#include "jsonParser/exceptions.h"
#include <string>
#include <variant>

namespace Json {
    struct Token {
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

        Token( Type type )
            : type( type ), value( std::monostate{} )
            {}

        Token( double number )
            : type( Type::Number ), value( number  )
            {}

        Token( bool boolean )
            : type( Type::Boolean ), value( boolean )
            {}

        Token( const std::string &string )
            : type( Type::String ), value( string )
            {}

        bool operator==( Token::Type other ) const { 
            return type == other;
        }

        bool operator!=( Token::Type other ) const {
            return type != other;
        }

        bool takeBoolean() 
        {
            if ( !std::holds_alternative< bool >( value ) )
                throw JsonException( "Token does not contain a boolean" );
            return std::get< bool > ( value );
        }

        double takeNumber()
        {
            if ( !std::holds_alternative< double >( value ) )
                throw JsonException( "Token does not contain a number" );
            return std::get< double > ( value );
        }

        std::string takeString() 
        {
            if ( !std::holds_alternative< std::string >( value ) )
                throw JsonException( "Token does not contain a string" );
            return std::move( std::get< std::string > ( value ) );
        }

        operator std::string() const {

            switch( type ) {
                case Type::Eof:
                    return "EOF";
                case Type::Null:
                    return "NULL";
                case Type::String:
                    return "STRING < " + std::get< std::string > ( value ) + " >";
                case Type::Number:
                    return "NUMBER < " + std::to_string( std::get< double > ( value ) ) + " >" ;
                case Type::Boolean:
                    return "BOOLEAN < " + std::string(std::get< bool > ( value ) ? "true" : "false") + " >" ;
                case Type::LeftBrace:
                    return "LEFT_BRACE";
                case Type::RightBrace:
                    return "RIGHT_BRACE";
                case Type::LeftBracket:
                    return "LEFT_BRACKET";
                case Type::RightBracket:
                    return "RIGHT_BRACKET";
                case Type::Comma:
                    return "COMMA";
                case Type::Colon:
                    return "COLON";
            }

            return "";
        }

    };

    std::string tokenTypeToString( Token::Type type );

}

