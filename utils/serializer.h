#pragma once

#include "utils/job.h"
#include <cstring>
#include <span>
#include <string_view>
#include <unistd.h>

namespace Utils {

    class Serializer {
        static constexpr std::size_t BUFF_SIZE = 1024;

        std::array< char, BUFF_SIZE > m_buff;
        std::size_t m_pos{};
        Utils::Job &m_job;

        void flush() noexcept
        {
            std::size_t written = 0;
            do {
                ssize_t res = m_job.write( m_buff.data() + written, m_pos - written );
                if ( res == 0 )
                    break;
                if ( res < 0 ) {
                    /* Report errors here */
                    return;
                }
                written += static_cast< std::size_t >( res );
            } while ( written < m_pos );
            m_pos = 0;
        }

        public:
            Serializer( Utils::Job &job ) noexcept
                : m_job( job )
            {}

            /* Empty out the rest of the serializer into the job.. */
            ~Serializer()
            {
                flush();
            }

            Serializer( const Serializer &other ) = delete;
            Serializer( Serializer &&other ) = delete;
            Serializer &operator=( const Serializer &other ) = delete;
            Serializer &operator=( Serializer &&other ) = delete;

            Serializer& operator <<( const std::string &str ) noexcept
            {
                return *this << std::span( str );
            }

            Serializer& operator <<( std::string_view str ) noexcept
            {
                return *this << std::span( str );
            }

            Serializer& operator <<( const char *str ) noexcept
            {
                return *this << std::string_view( str );
            }

            Serializer& operator <<( std::span< const char > str) noexcept
            {
                while ( !str.empty() ) {
                    std::size_t available = BUFF_SIZE - m_pos;
                    if ( !available ) {
                        flush();
                        continue;
                    }
                    std::size_t space = std::min( available, str.size() );
                    std::memcpy(
                            m_buff.data() + m_pos,
                            str.data(),
                            space );
                    m_pos += space;
                    str = str.subspan( space );
                }
                return *this;
            }


    };

}
