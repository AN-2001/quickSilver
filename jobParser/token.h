#include <cstdint>
#include <string_view>
#include <utils/fastMap.h>

#pragma once

namespace Json {

    enum class Token : std::uint8_t
    {
        JobType,
        Metrics,
        Compute,
        Algorithm,
        NumVertices,
        Edges,
        Graph,
        Input,
        Labels,
        Dfs,
        Bfs,
        Number,
        Label,
        LeftBracket,
        RightBracket,
        LeftBrace,
        RightBrace,
        Comma,
        Colon,
        Null,
        True,
        False,
        Eof,
        Invalid,
        NumTokens,
    };


    [[nodiscard]] constexpr auto buildGraphKeywordsMapping() noexcept 
    {
        using namespace std::string_view_literals;

        Utils::FastMap<std::string_view, Token, 11 > map;

        map.add( "jobType"sv, Token::JobType );
        map.add( "metrics"sv, Token::Metrics );
        map.add( "compute"sv, Token::Compute );
        map.add( "algorithm"sv, Token::Algorithm );
        map.add( "numVertices"sv, Token::NumVertices );
        map.add( "edges"sv, Token::Edges );
        map.add( "graph"sv, Token::Graph );
        map.add( "input"sv, Token::Input );
        map.add( "labels"sv, Token::Labels );
        map.add( "DFS"sv, Token::Dfs );
        map.add( "BFS"sv, Token::Bfs );
        return map;
    }

    [[nodiscard]] constexpr auto buildJsonKeywordsMapping() noexcept 
    {
        using namespace std::string_view_literals;

        Utils::FastMap<std::string_view, Token, 3 > map;

        map.add( "false"sv, Token::False );
        map.add( "true"sv, Token::True );
        map.add( "null"sv, Token::Null );
        return map;
    }

    [[nodiscard]] constexpr Token graphKeywordToToken( std::string_view str ) noexcept
    {
        static constexpr auto mapping = buildGraphKeywordsMapping();
        Token tok;
        if ( !mapping.find( str, tok ) ) return Token::Invalid;
        return tok;
    }

    [[nodiscard]] constexpr Token jsonKeywordToToken( std::string_view str ) noexcept
    {
        static constexpr auto mapping = buildJsonKeywordsMapping();
        Token tok;
        if ( !mapping.find( str, tok ) ) return Token::Invalid;
        return tok;
    }

    struct TokenWrapper {
        Token m_type;
        double m_numericValue;
        std::string_view m_labelValue;

        constexpr TokenWrapper() noexcept {}

        constexpr TokenWrapper( Token type ) noexcept : m_type( type ) {}
        constexpr TokenWrapper( double numeric ) noexcept : m_type( Token::Number ), m_numericValue( numeric ) {}
        constexpr TokenWrapper( std::string_view label ) noexcept : m_type( Token::Label ), m_labelValue( label ) {}

        [[nodiscard]] constexpr std::string_view getLabelValue() const noexcept 
        {
            return m_labelValue;
        }

        [[nodiscard]] constexpr double getNumericValue() const noexcept 
        {
            return m_numericValue;
        }

    };

};
