#pragma once

#include "jobParser/parserEvents.h"
#include <cstddef>
#include <string>
#include <vector>

struct EventWrapper {

    std::size_t m_stringCounts;
    std::vector< std::string > m_strings;
    std::vector< Json::ParserEvent > m_inputSequence;

    public:

    template < std::same_as< Json::ParserEvent >... Events >
    EventWrapper(std::size_t counts,
                 const std::vector< std::string > &strings,
                 Events &&...events)
        : m_stringCounts(counts),
          m_strings( strings ),
          m_inputSequence{std::forward<Events>( events )...}
    {}

    template < std::same_as< Json::ParserEvent >... Events >
    EventWrapper(std::size_t counts,
                 Events &&...events)
        : m_stringCounts(counts),
          m_strings(),
          m_inputSequence{std::forward<Events>( events )...}
    {}

    [[nodiscard]] auto begin() const noexcept {
        return m_inputSequence.begin();
    }

    [[nodiscard]] auto end() const noexcept {
        return m_inputSequence.end();
    }

    [[nodiscard]] std::size_t getStringCount() const noexcept {
        return m_stringCounts;
    }

    [[nodiscard]] std::vector< std::string > releaseStrings() const noexcept {
        return std::move( m_strings );
    }

};
