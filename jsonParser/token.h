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

    };
}
