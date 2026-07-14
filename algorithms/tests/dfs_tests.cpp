#include <cstdint>
#include <gtest/gtest.h>

#include <string>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "algorithms/dfs.h"
#include "utils/arrayView.h"
#include "utils/job.h"
#include "utils/managedFd.h"


using namespace std::literals::string_view_literals;


struct DfsTestCase {
    const char *name;
    Utils::GraphCsr graph;
    std::size_t numInputs;
    std::array< uint16_t, Utils::MAX_INPUT_SIZE > input;
    std::string_view expected;
};

class DfsTest : public ::testing::TestWithParam<DfsTestCase> {};

using namespace Utils;
static Utils::Arena g_arena( 200_MB );
static Utils::Allocator g_allocator( g_arena );

TEST_P(DfsTest, HandlesGraph) {
    const auto &testParams = GetParam();
    std::array< char, 1024 > buff;
    int pipe[2]{};


    if ( ::pipe( pipe ) < 0 ) {
        FAIL() << "Could not open pipe";
    }

    Utils::Job job( Utils::BorrowedFd( 0 ), Utils::BorrowedFd{ pipe[ 1 ] } );

    {
        Algorithms::Dfs dfs( g_allocator, testParams.graph, testParams.input, testParams.numInputs );
        dfs.run();
        dfs.serialize( job );
    }

    ssize_t size = ::read( pipe[ 0 ], buff.data(), buff.size() );

    if ( size < 0 )
        FAIL() << "Could not read output";
    ASSERT_EQ( std::string_view( buff.data(), static_cast< std::size_t >( size ) ) , testParams.expected );

    ::close( pipe[ 0 ] );
    ::close( pipe[ 1 ] );
}

static const DfsTestCase DfsTests[] = {
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
        .expected = R"json({"status":"ok","parent":[0]})json"sv
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
        .expected = R"json({"status":"DFS error","error":"Invalid input count"})json"sv
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
        .expected = R"json({"status":"DFS error","error":"Source is too big to be a vertex"})json"sv
    },
    {
        .name = "test_chain",
        .graph = {
            .numVertices = 4,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 3 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1, 2, 3, 3 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","parent":[0,0,1,2]})json"sv
    },
    {
        .name = "test_branching",
        .graph = {
            .numVertices = 5,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 3, 4 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 2, 2, 2, 3, 4 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","parent":[0,0,0,null,null]})json"sv
    },
    {
        .name = "test_disconnected_graph",
        .graph = {
            .numVertices = 5,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1, 2, 2, 2, 2 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","parent":[0,0,1,null,null]})json"sv
    },
    {
        .name = "test_cycle",
        .graph = {
            .numVertices = 3,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 0 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1, 2, 3 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","parent":[0,0,1]})json"sv
    },
    {
        .name = "test_multiple_children",
        .graph = {
            .numVertices = 6,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 3, 4, 5 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 5, 5, 5, 5, 5, 5 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 0 },
        .expected = R"json({"status":"ok","parent":[0,0,0,0,0,0]})json"sv
    },
    {
        .name = "test_self_loop",
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
        .expected = R"json({"status":"ok","parent":[0]})json"sv
    },
    {
        .name = "test_two_components_start_second",
        .graph = {
            .numVertices = 6,
            .adj = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 1, 2, 4, 5 }
                    ),
            .offsets = Utils::makeArrayView<std::uint16_t>(g_allocator,
                    { 0, 1, 2, 2, 3, 4, 4 }
                    ),
            .labels = {}
        },
        .numInputs = 1,
        .input = { 3 },
        .expected = R"json({"status":"ok","parent":[null,null,null,3,3,4]})json"sv
    },
};

INSTANTIATE_TEST_SUITE_P (
        StaticDfsTests,
        DfsTest,
        ::testing::ValuesIn(DfsTests),
        [](const ::testing::TestParamInfo<DfsTestCase> &parserTestInfo) {
        return parserTestInfo.param.name;
        }
        );
