#pragma once

#include "jobParser/token.h"
#include "jobParser/errors.h"
#include "utils/job.h"

#include <array>
#include <expected>
#include <optional>
#include <string_view>

namespace Json {
    class Lexer {
        static constexpr std::size_t g_buffSize = 4096;

        Utils::Job &m_job;

        std::array< char, g_buffSize > m_readBuff{};
        std::array< char, g_buffSize > m_scratch{};

        bool m_eof{};

        ssize_t m_readBuffSize{};

        std::size_t m_readBuffCursor{};

        std::optional< Json::TokenWrapper > m_nextToken{};

        [[nodiscard]] std::expected< Json::TokenWrapper, Json::Error > computeNextToken() noexcept;

        class SafeReader {
            Utils::Job &m_job;
            std::array< char, g_buffSize > &m_buff;
            ssize_t &m_size;
            std::size_t &m_cursor;
            bool &m_eof;

            [[nodiscard]] inline Json::Error populate() noexcept 
            {
                m_cursor = 0;
                if ( m_eof )
                    return Json::Error::UnexpectedEof;

                m_size = m_job.read( m_buff.data(), g_buffSize );
                if ( m_size < 0 )
                    return Json::Error::ReadError;
                if ( !m_size ) {
                    m_eof = true;
                    return Json::Error::UnexpectedEof;
                }
                return Json::Error::NoError;
            }
                

            public:
            SafeReader( Lexer &lexer ) noexcept :
                m_job( lexer.m_job ),
                m_buff( lexer.m_readBuff ),
                m_size( lexer.m_readBuffSize ),
                m_cursor( lexer.m_readBuffCursor ),
                m_eof( lexer.m_eof )
            {}

            [[nodiscard]] inline std::expected<char, Json::Error> peek() noexcept 
            {
                if ( m_cursor == std::size_t( m_size ) ) {
                    auto res = populate();
                    if ( res != Json::Error::NoError )
                        return std::unexpected( res );
                }
                return m_buff[ m_cursor ];
            }

            [[nodiscard]] inline std::expected<char, Json::Error> consume() noexcept 
            {
                if ( m_cursor == std::size_t( m_size ) ) {
                    auto res = populate();
                    if ( res != Json::Error::NoError )
                        return std::unexpected( res );
                }
                return m_buff[ m_cursor++ ];
            }

            SafeReader( const SafeReader &other ) noexcept = delete;
            SafeReader( SafeReader &&other ) noexcept = delete;
            void operator=( const SafeReader &other ) noexcept = delete;
            void operator=( SafeReader &&other ) noexcept = delete;
        };

        class ScratchPopulator {
            std::array< char, g_buffSize > &m_buff;
            std::size_t m_size;

            public:

            ScratchPopulator( Lexer &lexer ) noexcept : m_buff( lexer.m_scratch ), m_size( 0 ) {}

            void add( const char c ) noexcept 
            {
                m_buff[ m_size++ ] = c;
            }

            [[nodiscard]] std::size_t getSize() const noexcept 
            {
                return m_size;
            }
            [[nodiscard]] std::string_view toView() const noexcept 
            {
                return { m_buff.data(), m_size };
            }

            ScratchPopulator( const ScratchPopulator &other ) noexcept = delete;
            ScratchPopulator( ScratchPopulator &&other ) noexcept = delete;

            void operator=( const ScratchPopulator &other ) noexcept = delete;
            void operator=( ScratchPopulator &&other ) noexcept = delete;
        };

        public:

            explicit Lexer( Utils::Job &job ) : m_job( job ) {}

            ~Lexer() = default;

            [[nodiscard]] inline std::expected< Json::TokenWrapper, Json::Error > peek() noexcept
            {
                if ( !m_nextToken ) {
                    auto computed = computeNextToken();
                    if ( !computed )
                        return computed;
                    m_nextToken = *computed;
                }

                return *m_nextToken;
            }

            [[nodiscard]] inline std::expected< Json::TokenWrapper, Json::Error > get() noexcept
            {
                if ( !m_nextToken ) {
                    auto computed = computeNextToken();
                    if ( !computed )
                        return computed;
                    m_nextToken = *computed;
                }

                auto ret = *m_nextToken;
                m_nextToken.reset();
                return ret;
            }

            [[nodiscard]] Utils::Job &job() const noexcept
            {
                return m_job;
            }

            Lexer( const Lexer &other ) noexcept = delete;
            Lexer( Lexer &&other ) noexcept = delete;
            void operator=( const Lexer &other ) noexcept = delete;
            void operator=( Lexer &&other ) noexcept = delete;
    };
};
