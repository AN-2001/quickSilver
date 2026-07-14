#include <cstdint>
#include <gtest/gtest.h>

#include <string>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "algorithms/bfs.h"
#include "utils/job.h"
#include "utils/managedFd.h"


using namespace std::literals::string_view_literals;


struct BfsTestCase {
    const char *name;
    Utils::GraphCsr graph;
    std::size_t numInputs;
    std::array< uint16_t, Utils::MAX_INPUT_SIZE > input;
    std::string_view expected;
};

class BfsTest : public ::testing::TestWithParam<BfsTestCase> {};

using namespace Utils;
static Utils::Arena g_arena( 200_MB );
static Utils::Allocator g_allocator( g_arena );

TEST_P(BfsTest, HandlesGraph) {
    const auto &testParams = GetParam();
    std::array< char, 1024 > buff;
    int pipe[2]{};


    if ( ::pipe( pipe ) < 0 ) {
        FAIL() << "Could not open pipe";
    }

    Utils::Job job( Utils::BorrowedFd( 0 ), Utils::BorrowedFd{ pipe[ 1 ] } );

    {
        Algorithms::Bfs bfs( g_allocator, testParams.graph, testParams.input, testParams.numInputs );
        bfs.run();
        bfs.serialize( job );
    }

    ssize_t size = ::read( pipe[ 0 ], buff.data(), buff.size() );

    if ( size < 0 )
        FAIL() << "Could not read output";
    ASSERT_EQ( std::string_view( buff.data(), static_cast< std::size_t >( size ) ) , testParams.expected );

    ::close( pipe[ 0 ] );
    ::close( pipe[ 1 ] );
}

static const BfsTestCase BfsTests[] = {
    {
        .name = "test_standard",
        .graph = {
            .numVertices = 1,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0],"parent":[0]})json"sv
    },
    {
        .name = "test_bad_input_count",
        .graph = {
            .numVertices = 1,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1 }
                    ),
            .labels = {}
        },
        .numInputs = 2,
        .input = { 0, 0 },
        .expected = R"json({"status":"BFS error","error":"Invalid input count"})json"sv
    },
    {
        .name = "test_bad_input",
        .graph = {
            .numVertices = 1,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 2 },
        .expected = R"json({"status":"BFS error","error":"Source is too big to be a vertex"})json"sv
    },
    {
        .name = "test_fan_out",
        .graph = {
            .numVertices = 10,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 3, 4, 5, 6, 7, 8, 9 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,1,1,1,1,1,1,1,1],"parent":[0,0,0,0,0,0,0,0,0,0]})json"sv
    },
    {
        .name = "test_chain",
        .graph = {
            .numVertices = 6,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 3, 4, 5 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1, 2, 3, 4, 5, 5 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,2,3,4,5],"parent":[0,0,1,2,3,4]})json"sv
    },
    {
        .name = "test_diamond",
        .graph = {
            .numVertices = 6,

            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    1, 2,       // 0 -> 1,2
                    3,          // 1 -> 3
                    3,          // 2 -> 3
                    4, 5        // 3 -> 4,5
                    }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 2, 3, 4, 6, 6, 6 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,1,2,3,3],"parent":[0,0,0,1,3,3]})json"sv
    },

    {
        .name = "test_disconnected",
        .graph = {
            .numVertices = 7,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    1, 2
                    }

                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    0, 2, 2, 2, 2, 2, 2, 2
                    }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,1,"INF","INF","INF","INF"],"parent":[0,0,0,-1,-1,-1,-1]})json"sv
    },
    {
        .name = "test_cycle",
        .graph = {
            .numVertices = 5,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    1,        // 0 -> 1
                    2,        // 1 -> 2
                    3,        // 2 -> 3
                    4,        // 3 -> 4
                    0         // 4 -> 0
                    }

                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1, 2, 3, 4, 5 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,2,3,4],"parent":[0,0,1,2,3]})json"sv
    },
    {
        .name = "test_tree",
        .graph = {
            .numVertices = 10,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    1, 2,        // 0
                    3, 4,        // 1
                    5, 6,        // 2
                    7,           // 3
                    8,           // 4
                    9            // 5
                    }

                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    0, 2, 4, 6, 7, 8, 9, 9, 9, 9, 9
                    }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,1,2,2,2,2,3,3,3],"parent":[0,0,0,1,1,2,2,3,4,5]})json"sv
    },
    {
        .name = "test_complex_graph",
        .graph = {
            .numVertices = 12,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    {
                    // 0
                    1, 2, 3,

                    // 1
                    4, 5,

                    // 2
                    5, 6,

                    // 3
                    7,

                    // 4
                    8, 1,

                    // 5
                    8, 2,

                    // 6
                    10,

                    // 7
                    9, 3,

                    // 8
                    10,

                    // 9
                    10,

                    // 10
                    11, 6,

                    // 11
                    }

            ),
                .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                        {
                        0,   // vertex 0
                        3,   // vertex 1
                        5,   // vertex 2
                        7,   // vertex 3
                        8,   // vertex 4
                        10,  // vertex 5
                        12,  // vertex 6
                        13,  // vertex 7
                        15,  // vertex 8
                        16,  // vertex 9
                        17,  // vertex 10
                        19,  // vertex 11
                        19
                        }
                        ),
                .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","distance":[0,1,1,1,2,2,2,2,3,3,3,4],"parent":[0,0,0,0,1,1,2,3,4,7,6,10]})json"sv
    },
};

INSTANTIATE_TEST_SUITE_P (
        StaticBfsTests,
        BfsTest,
        ::testing::ValuesIn(BfsTests),
        [](const ::testing::TestParamInfo<BfsTestCase> &parserTestInfo) {
        return parserTestInfo.param.name;
        }
        );
