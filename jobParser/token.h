#include <cstdint>
#include <string_view>
#include <utility>
#include <utils/fastMap.h>
#include <optional>

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

    [[nodiscard]] constexpr auto buildTokenMapping() noexcept
    {
        using namespace std::string_view_literals;

        Utils::FastMap<Token, std::string_view, std::to_underlying( Token::NumTokens ) > map;

        map.add( Token::JobType, "job type"sv );
        map.add( Token::Metrics, "metrics"sv );
        map.add( Token::Compute, "compute"sv );
        map.add( Token::Algorithm, "algorithm"sv );
        map.add( Token::NumVertices, "numVertices"sv );
        map.add( Token::Edges, "edges"sv );
        map.add( Token::Graph, "graph"sv );
        map.add( Token::Input, "input"sv );
        map.add( Token::Labels, "labels"sv );
        map.add( Token::Dfs, "dfs"sv );
        map.add( Token::Bfs, "bfs"sv );
        map.add( Token::Number, "number"sv );
        map.add( Token::Label, "label"sv );
        map.add( Token::LeftBracket, "left bracket"sv );
        map.add( Token::RightBracket, "right bracket"sv );
        map.add( Token::LeftBrace, "left brace"sv );
        map.add( Token::RightBrace, "right brace"sv );
        map.add( Token::Comma, "comma"sv );
        map.add( Token::Colon, "colon"sv );
        map.add( Token::Null, "null"sv );
        map.add( Token::True, "true"sv );
        map.add( Token::False, "false"sv );
        map.add( Token::Eof, "EOF"sv );
        return map;
    }

    [[nodiscard]] constexpr std::optional<std::string_view> tokenToString( Token token ) noexcept {
        static constexpr auto mapping = buildTokenMapping();
        if ( !mapping.exists( token ) ) return std::nullopt;
        return mapping.get( token );
    }

    [[nodiscard]] constexpr std::optional<Token> graphKeywordToToken( std::string_view str ) noexcept
    {
        static constexpr auto mapping = buildGraphKeywordsMapping();
        if ( !mapping.exists( str ) ) return std::nullopt;
        return mapping.get( str );
    }

    [[nodiscard]] constexpr std::optional<Token> jsonKeywordToToken( std::string_view str ) noexcept
    {
        static constexpr auto mapping = buildJsonKeywordsMapping();
        if ( !mapping.exists( str ) ) return std::nullopt;
        return mapping.get( str );
    }

    struct TokenWrapper {
        Token m_type{};
        float m_numericValue{};
        std::string_view m_labelValue{};
        std::string repr{};

        TokenWrapper( Token type ) : m_type( type ) {}
        TokenWrapper( float numeric ) : m_type( Token::Number ), m_numericValue( numeric ) {}
        TokenWrapper( std::string_view label ) : m_type( Token::Label ), m_labelValue( label ) {}

        [[nodiscard]] constexpr std::string_view getLabelValue() const noexcept 
        {
            /* No assert on this! watch out!!! */
            return m_labelValue;
        }

        [[nodiscard]] constexpr float getNumericValue() const noexcept 
        {
            /* No assert on this! watch out!!! */
            return m_numericValue;
        }

        [[nodiscard]] constexpr bool operator==( Token t ) const noexcept 
        {
            return m_type == t;
        }

        [[nodiscard]] constexpr std::optional< std::string_view > toString() noexcept 
        {
            if ( !repr.empty() )
                return repr;
            auto name = tokenToString( m_type );
            if ( !name )
                return std::nullopt;
            repr = *name;
            if ( m_type == Token::Number )
                repr += "(" + std::to_string( m_numericValue ) + ")";
            if ( m_type == Token::Label )
                repr += "(" + std::string( m_labelValue ) + ")";
            return repr;
        }
    };

};
