#pragma once

#include "jobParser/token.h"
#include "jobParser/errors.h"
#include "utils/job.h"

#include <array>
#include <cstring>
#include <string_view>

namespace Json {
    class Lexer {
        static constexpr std::size_t g_buffSize = 1024 * 16;
        static constexpr std::size_t g_scratchBuffSize = 1024;

        Utils::Job &m_job;

        std::array< char, g_buffSize > m_readBuff{};
        std::array< char, g_scratchBuffSize > m_scratch{};

        std::size_t m_readBuffSize{};

        std::size_t m_readBuffCursor{};

        Json::TokenWrapper m_nextToken{};

        Json::Error m_error{ Error::NoError };
        bool m_hasNextToken{};
        bool m_eof{};

        [[nodiscard]] Json::TokenWrapper computeNextToken() noexcept;

        class ScratchPopulator {
            Lexer &m_lexer;
            std::size_t m_begin;
            std::size_t m_prefixSize{};

            public:

            inline ScratchPopulator( Lexer &lexer ) noexcept :
                m_lexer( lexer ),
                m_begin( m_lexer.m_readBuffCursor ) {}

            inline void triggerCopy() noexcept 
            {
                std::size_t size = m_lexer.m_readBuffCursor - m_begin;
                std::memcpy( m_lexer.m_scratch.data() + m_prefixSize, m_lexer.m_readBuff.data() + m_begin, size );
                m_prefixSize += size;
                m_begin = 0;
            }

            /* CAN ONLY BE CALLED ONCE! */
            [[nodiscard]] inline std::string_view toView( bool ignoreLast = false ) const noexcept 
            {
                std::size_t size = m_lexer.m_readBuffCursor - m_begin ;
                if ( m_prefixSize ) {
                    std::memcpy( m_lexer.m_scratch.data() + m_prefixSize, m_lexer.m_readBuff.data() + m_begin, size );
                    return { m_lexer.m_scratch.data(), size + m_prefixSize - ignoreLast };
                } 
                return { m_lexer.m_readBuff.data() + m_begin, size - ignoreLast };
            }

            ScratchPopulator( const ScratchPopulator &other ) noexcept = delete;
            ScratchPopulator( ScratchPopulator &&other ) noexcept = delete;

            void operator=( const ScratchPopulator &other ) noexcept = delete;
            void operator=( ScratchPopulator &&other ) noexcept = delete;
        };

            inline void fillReadBuff( ScratchPopulator *populator = nullptr ) noexcept 

            {
                if ( m_eof )
                    return;

                if ( populator )
                    populator -> triggerCopy();

                m_readBuffCursor = 0;

                ssize_t size = m_job.read( m_readBuff.data(), g_buffSize );
                if ( size < 0 ) {
                    m_error = Json::Error::ReadError;
                }

                m_readBuffSize = static_cast< std::size_t >( size );

                if ( !size ) {
                    m_eof = true;
                    return;
                }
            }

            inline char consumeChar( ScratchPopulator *populator = nullptr ) noexcept 
            {
                char ret = ( m_error != Json::Error::NoError || m_eof ) ? '\0' : m_readBuff[ m_readBuffCursor++ ];
                if ( m_readBuffCursor == m_readBuffSize )
                    fillReadBuff( populator );
                return ret;
            }

            [[nodiscard]] inline char peekChar() noexcept 
            {
                return ( m_error != Json::Error::NoError || m_eof ) ? '\0' : m_readBuff[ m_readBuffCursor ];
            }


        public:

            explicit Lexer( Utils::Job &job ) : m_job( job )
            {
                fillReadBuff();
            }

            ~Lexer() = default;

            [[nodiscard]] inline Json::TokenWrapper peek() noexcept
            {
                if ( !m_hasNextToken ) {
                    auto computed = computeNextToken();
                    m_nextToken = computed;
                    m_hasNextToken = true;
                }

                return m_nextToken;
            }

            inline Json::TokenWrapper get() noexcept
            {
                if ( !m_hasNextToken ) {
                    m_nextToken = computeNextToken();
                    m_hasNextToken = true;
                }

                auto ret = m_nextToken;
                m_hasNextToken = false;
                return ret;
            }

            [[nodiscard]] Json::Error error() const noexcept 
            {
                return m_error;
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
