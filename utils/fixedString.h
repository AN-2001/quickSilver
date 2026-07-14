#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <string_view>

namespace Utils {
    constexpr std::size_t g_maxStringSize = 16;

    class FixedString {
        public:
        std::size_t m_size{};
        std::array< char, g_maxStringSize > m_internal{};

        std::string_view toView() const noexcept 
        {
            return std::string_view( m_internal.data(), m_size );
        }
    };

    inline FixedString makeFixedString( std::string_view view ) noexcept
    {
        FixedString str;
        str.m_size = view.size();
        std::memcpy( str.m_internal.data(), view.data(), view.size() );
        return str;
    }

};
