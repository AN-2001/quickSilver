#pragma once

#include "utils/arrayView.h"
#include "utils/fixedString.h"
#include <cstddef>
#include <cstdint>

namespace Utils {

    static constexpr std::size_t MAX_INPUT_SIZE = 32;

    enum class JobType {
        Compute,
        Metrics
    };

    enum class AlgorithmType {
        BFS,
        DFS
    };

    struct GraphCsr {
        std::size_t numVertices;
        Utils::ArrayView< std::uint16_t > adj;
        Utils::ArrayView< std::uint16_t > offsets;
        Utils::ArrayView< std::uint16_t > labels;
    };

    struct JobState {
        JobType type;
        AlgorithmType algorithm;
        std::size_t numInputs;
        std::array< std::uint16_t, MAX_INPUT_SIZE > inputs;
        GraphCsr graph;
        Utils::ArrayView< Utils::FixedString > strings;
    };

}
