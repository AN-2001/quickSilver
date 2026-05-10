#include <cstdint>
#include <string>
#include <array>

#pragma once

namespace Json {

    enum class Token : std::uint8_t {
        Number,
        String,
        LeftBracket,
        RightBracket,
        LeftBrace,
        RightBrace,
        Comma,
        Colon,
        Null,
        True,
        False,
        NumTokens,
    };

    namespace {
        inline auto buildStringTable() 
        {
            using namespace std::string_literals;
            std::array< std::string, static_cast< std::size_t >( Token::NumTokens ) > 
                arr = {
                    "number"s,
                    "string"s,
                    "left bracket"s,
                    "right bracket"s,
                    "left brace"s,
                    "right brace"s,
                    "comma"s,
                    "colon"s,
                    "null"s,
                    "true"s,
                    "false"s
                };
            return arr;
        }
    };

    const auto tokenStringTable = buildStringTable();
    constexpr std::uint8_t toIndex( const Token token ) {
        return static_cast< std::uint8_t >( token );
    }
    constexpr std::string_view tokenToString( const Token token ) {
        return tokenStringTable[ toIndex( token ) ];
    }
};
