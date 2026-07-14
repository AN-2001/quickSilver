#pragma once

#include "jobParser/parserEvents.h"
#include "utils/arrayView.h"
#include "utils/fixedString.h"
#include <cstddef>
#include <vector>

struct EventWrapper {

    std::size_t m_stringCounts;
    Utils::ArrayView< Utils::FixedString > m_strings;
    std::vector< Json::ParserEvent > m_inputSequence;

    public:

    template < std::same_as< Json::ParserEvent >... Events >
    EventWrapper(std::size_t counts,
                 Utils::ArrayView< Utils::FixedString > strings,
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

    [[nodiscard]] Utils::ArrayView< Utils::FixedString > releaseStrings() const noexcept {
        return m_strings;
    }

};
