#pragma once

#include "utils/fastMap.h"
#include <cstdint>
#include <string_view>
#include <utility>
namespace Json {

    enum class Error : std::uint8_t {
        NoError = 0,
        ReadError,
        InvalidNumber,
        UnexpectedToken,
        UnknownToken,
        UnexpectedEof,
        TooManyEvents,
        TooManyStrings,
        NumErrors
    };

    [[nodiscard]] constexpr auto buildErrorMapping() noexcept
    {
        using namespace std::string_view_literals;

        Utility::FastMap< Json::Error, std::string_view, std::to_underlying( Error::NumErrors ) > map;

        map.add( Error::NoError, "no error"sv );
        map.add( Error::UnexpectedToken, "unexpected token"sv );
        map.add( Error::ReadError, "read error"sv );
        map.add( Error::InvalidNumber, "invalid number"sv );
        map.add( Error::UnknownToken, "unknown token"sv );
        map.add( Error::UnexpectedEof, "unexpected end of file"sv );
        return map;
    }

    [[nodiscard]] constexpr std::string_view errorToString( Json::Error err ) noexcept
    {
        static constexpr auto mapping = buildErrorMapping();
        if ( !mapping.exists( err ) ) return "unknown";
        return mapping.get( err );
    }

};
