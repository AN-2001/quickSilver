#pragma once

#include "jobParser/errors.h"
#include "lexer.h"
#include <array>
#include <cstdint>
#include <expected>
#include <string_view>
#include <utility>
#include "parserEvents.h"
#include "utils/allocator.h"
#include "utils/bumpArray.h"
#include "utils/arrayView.h"
#include "utils/fixedString.h"

namespace Json {
    class Parser {
        private:
        static constexpr std::size_t NUM_EVENTS_PER_TYPE = 4096;
        static constexpr std::size_t NUM_EVENTS = std::to_underlying( ParserEventType::NumEvents );

        std::array< std::array< ParserEvent, NUM_EVENTS_PER_TYPE>,
                    NUM_EVENTS > m_eventQueue;
        std::array< std::size_t, NUM_EVENTS > m_eventCounts{};

        Json::Error addEvent( const ParserEvent &event ) noexcept
        {
            uint16_t index = std::to_underlying( event.m_type );
            std::size_t &count = m_eventCounts[ index ];
            if ( count == NUM_EVENTS_PER_TYPE )
                return Json::Error::TooManyEvents;
            m_eventQueue[ index ][ count++ ] = event;
            return Json::Error::NoError;
        }

        [[nodiscard]] std::expected< uint16_t, Json::Error > addString( std::string_view view ) noexcept 
        {
            if ( m_stringTable.size() == NUM_EVENTS_PER_TYPE )
                return std::unexpected<Json::Error>( Json::Error::TooManyStrings );
            m_stringTable.bump( Utils::makeFixedString( view ) );
            return static_cast< uint16_t >( m_stringTable.size() - 1 );
        }

        Lexer &m_lexer;
        Utils::BumpArray< Utils::FixedString > m_stringTable;
        Utils::Allocator &m_allocator;

        [[nodiscard]] inline Json::Error expect( Json::Token token ) noexcept {
            auto tok = m_lexer.get();
            if ( !tok )
                return tok.error();
            if ( *tok != token )
                return Json::Error::UnexpectedToken;
            return Json::Error::NoError;
        }

        [[nodiscard]] Json::Error parseJob() noexcept;
        [[nodiscard]] Json::Error parseJobType() noexcept;
        [[nodiscard]] Json::Error parseAlgorithm() noexcept;
        [[nodiscard]] Json::Error parseGraph() noexcept;
        [[nodiscard]] Json::Error parseGraphNumVertices() noexcept;
        [[nodiscard]] Json::Error parseGraphEdges() noexcept;
        [[nodiscard]] Json::Error parseGraphLabels() noexcept;
        [[nodiscard]] Json::Error parseInput() noexcept;

        class Iterator {
            Parser *m_parser;
            uint16_t m_index;
            uint8_t m_priority;

            private:
            void advance() noexcept {
                while ( m_priority < Parser::NUM_EVENTS &&
                        m_index >= m_parser -> m_eventCounts[ m_priority ] ) {
                    m_priority++;
                    m_index = 0;
                }
            }

            public:
            Iterator( Parser * parser, uint16_t index, uint8_t priority )
                : m_parser( parser ), m_index( index ), m_priority( priority )
            {
                advance();
            }

            ParserEvent &operator*() const noexcept  {
                return m_parser -> m_eventQueue[ m_priority ][ m_index ];
            }

            Iterator& operator++() noexcept  {
                m_index++;
                advance();
                return *this;
            }

            bool operator!=( const Iterator &other ) const noexcept {
                return m_parser != other.m_parser || m_index != other.m_index || m_priority != other.m_priority;
            }

        };
        
        public:

        [[nodiscard]] Json::Error parse() noexcept;

        Parser( Lexer &lexer, Utils::Allocator &allocator )
            : m_lexer( lexer ),
              m_stringTable( allocator ),
              m_allocator( allocator ){}

        Parser( const Parser &other ) noexcept = delete;
        Parser( Parser &other ) noexcept = delete;

        Iterator begin() noexcept {
            return Iterator( this, 0, 0 );
        }

        Iterator end() noexcept {
            return Iterator( this, 0, NUM_EVENTS );
        }

        auto getStringCount() const noexcept -> std::size_t 
        {
            return m_stringTable.size();
        }

        auto releaseStrings() noexcept -> Utils::ArrayView< Utils::FixedString >
        {
            return m_stringTable.toView();
        }

        void operator=( const Parser &other ) noexcept = delete;
        void operator=( Parser &&other ) noexcept = delete;
    };
};

