#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <string_view>

namespace Utils {
    constexpr std::size_t g_maxStringSize = 256;

    class FixedString {
        std::size_t m_size{};
        std::array< char, g_maxStringSize > m_internal{};

        public:
        explicit FixedString( std::string_view view ) noexcept
            : m_size( view.size() )
        {
            std::memcpy( m_internal.data(), view.data(), view.size() );
        }

        FixedString( const FixedString &other ) noexcept
            : m_size( other.m_size ),
              m_internal( other.m_internal )
        {}

        FixedString( FixedString &&other ) noexcept
            : m_size( other.m_size ),
              m_internal( other.m_internal )
        {}

        FixedString &operator=( const FixedString &other ) noexcept
        {
            m_internal = other.m_internal;
            m_size = other.m_size;
            return *this;
        }

        FixedString &operator=( FixedString &&other ) noexcept
        {
            m_internal = other.m_internal;
            m_size = other.m_size;
            return *this;
        }

        bool operator==( const FixedString &other ) const noexcept 
        {
            return ( m_size == other.m_size && 
                     std::memcmp( m_internal.data(), other.m_internal.data(), m_size ) == 0 );
        }

        ~FixedString() = default;

        std::string_view toView() const noexcept 
        {
            return std::string_view( m_internal.data(), m_size );
        }

    };

};
