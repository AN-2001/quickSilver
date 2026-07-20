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
        static constexpr std::size_t g_buffSize = 1024 * 16;

        Utils::Job &m_job;

        std::array< char, g_buffSize > m_readBuff{};
        std::array< char, g_buffSize > m_scratch{};

        bool m_eof{};

        std::size_t m_readBuffSize{};

        std::size_t m_readBuffCursor{};

        std::optional< Json::TokenWrapper > m_nextToken{};

        [[nodiscard]] std::expected< Json::TokenWrapper, Json::Error > computeNextToken() noexcept;

        class SafeReader {
            Lexer &m_lexer;

            [[nodiscard]] inline Json::Error populate() noexcept 
            {
                m_lexer.m_readBuffCursor = 0;
                if ( m_lexer.m_eof )
                    return Json::Error::UnexpectedEof;

                ssize_t size = m_lexer.m_job.read( m_lexer.m_readBuff.data(), g_buffSize );
                if ( size < 0 )
                    return Json::Error::ReadError;

                m_lexer.m_readBuffSize = static_cast< std::size_t >( size );

                if ( !size ) {
                    m_lexer.m_eof = true;
                    return Json::Error::UnexpectedEof;
                }

                return Json::Error::NoError;
            }
                

            public:
            inline SafeReader( Lexer &lexer ) noexcept : m_lexer( lexer )
            {}

            [[nodiscard]] inline std::expected<char, Json::Error> peek() noexcept 
            {
                if ( m_lexer.m_readBuffCursor == m_lexer.m_readBuffSize ) {
                    auto res = populate();
                    if ( res != Json::Error::NoError )
                        return std::unexpected( res );
                }
                return m_lexer.m_readBuff[ m_lexer.m_readBuffCursor ];
            }

            [[nodiscard]] inline std::expected<char, Json::Error> consume() noexcept 
            {
                if ( m_lexer.m_readBuffCursor == m_lexer.m_readBuffSize ) {
                    auto res = populate();
                    if ( res != Json::Error::NoError )
                        return std::unexpected( res );
                }
                return m_lexer.m_readBuff[ m_lexer.m_readBuffCursor++ ];
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

            inline ScratchPopulator( Lexer &lexer ) noexcept : m_buff( lexer.m_scratch ), m_size( 0 ) {}

            inline void add( const char c ) noexcept 
            {
                m_buff[ m_size++ ] = c;
            }

            [[nodiscard]] inline std::size_t getSize() const noexcept 
            {
                return m_size;
            }
            [[nodiscard]] inline std::string_view toView() const noexcept 
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
