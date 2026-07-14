#include <gtest/gtest.h>

#include "jobBuilder/jobBuilder.h"
#include <string>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>

#include "jobParser/parserEvents.h"
#include "jobParser/token.h"
#include "utils/allocator.h"
#include "utils/arena.h"
#include "utils/arrayView.h"
#include "utils/fixedString.h"
#include "utils/jobState.h"
#include "eventWrapper.h"

using namespace std::literals::string_view_literals;


struct BuilderTestCase {
    const char *name;
    EventWrapper input;
    Utils::JobState expected;
};

using namespace Utils;
static Utils::Arena g_arena( 200_MB );
static Utils::Allocator g_allocator( g_arena );

class BuilderTest : public ::testing::TestWithParam<BuilderTestCase> {};

TEST_P(BuilderTest, HandlesEventSequence) {
    const auto &testParams = GetParam();

    JobTools::Builder builder( testParams.input, g_allocator );

    auto ret = builder.build();
    const auto &expected = testParams.expected;

    ASSERT_EQ( ret.type, expected.type );
    ASSERT_EQ( ret.algorithm, expected.algorithm );
    ASSERT_EQ( ret.numInputs, expected.numInputs );
    ASSERT_EQ( ret.inputs.size(), expected.inputs.size() );
    for ( std::size_t i = 0; i < ret.numInputs; ++i )
        ASSERT_EQ( ret.inputs[ i ], expected.inputs[ i ] );

    ASSERT_EQ( ret.graph.numVertices, expected.graph.numVertices );

    ASSERT_EQ( ret.graph.adj.size(), expected.graph.adj.size() );
    for ( std::size_t i = 0; i < ret.graph.adj.size(); ++i )
        ASSERT_EQ( ret.graph.adj[ i ], expected.graph.adj[ i ] );

    ASSERT_EQ( ret.graph.offsets.size(), expected.graph.offsets.size() );
    for ( std::size_t i = 0; i < ret.graph.offsets.size(); ++i )
        ASSERT_EQ( ret.graph.offsets[ i ], expected.graph.offsets[ i ] );

    ASSERT_EQ( ret.strings.size(), expected.strings.size() );
    ASSERT_EQ( ret.graph.labels.size(), expected.graph.labels.size() );
    for ( std::size_t i = 0; i < ret.graph.labels.size(); i++ )
        ASSERT_EQ( ret.strings[ ret.graph.labels[ i ] ].toView(), expected.strings[ expected.graph.labels[ i ] ].toView() );


}

static const BuilderTestCase BuilderTests[] = {
    {
        .name = "test_standard",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = {
            .type = Utils::JobType::Compute,
            .algorithm = Utils::AlgorithmType::BFS,
            .numInputs = 1,
            .inputs = { 5 },
            .graph = {
                .numVertices = 2,
                .adj = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 1 }
                        ),

                .offsets = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 0, 1, 1 }
                        ),
                .labels = Utils::makeArrayView<std::uint16_t>( g_allocator, 0 )
            },
            .strings = {}
        }
    },
    {
        .name = "test_with_edges",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 0 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = {
            .type = Utils::JobType::Compute,
            .algorithm = Utils::AlgorithmType::BFS,
            .numInputs = 1,
            .inputs = { 5 },
            .graph = {
                .numVertices = 3,
                .adj = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 1, 2, 0 }
                        ),

                .offsets = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 0, 2, 2, 3 }
                        ),
                .labels = Utils::makeArrayView<std::uint16_t>( g_allocator, 0 )
            },
            .strings = {}
        }
    },
    {
        .name = "test_multiple_inputs",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 4 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 5 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 1, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 3, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = {
            .type = Utils::JobType::Compute,
            .algorithm = Utils::AlgorithmType::BFS,
            .numInputs = 3,
            .inputs = { 0, 1, 2 },
            .graph = {
                .numVertices = 4,
                .adj = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 1, 2, 3, 0, 2 }
                        ),

                .offsets = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 0, 2, 3, 4, 5 }
                        ),

                .labels = Utils::makeArrayView<std::uint16_t>( g_allocator, 0 )
            },
            .strings = {}
        }
    },
    {
        .name = "test_with_labels",
        .input = { 2,
            Utils::makeArrayView< Utils::FixedString>(
                g_allocator,
                {
                    Utils::makeFixedString( "Chicago"sv ),
                    Utils::makeFixedString( "Los Angeles"sv ),
                } ),
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 4 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 5 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 1, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 3, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = {
            .type = Utils::JobType::Compute,
            .algorithm = Utils::AlgorithmType::BFS,
            .numInputs = 3,
            .inputs = { 0, 1, 2 },
            .graph = {
                .numVertices = 4,
                .adj = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 1, 2, 3, 0, 2 }
                        ),

                .offsets = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 0, 2, 3, 4, 5 }
                        ),

                .labels = Utils::makeArrayView<std::uint16_t>( g_allocator,
                            { 0, 1 }

                        )
            },
            .strings = Utils::makeArrayView<Utils::FixedString>(g_allocator,
                    { 
                      Utils::makeFixedString( "Chicago"sv ),
                      Utils::makeFixedString( "Los Angeles"sv )
                    }
                    )
        }
    },
    {
    .name = "test_complex",
    .input = { 3,
        Utils::makeArrayView< Utils::FixedString>(
            g_allocator,
            {
                Utils::makeFixedString( "Chicago"sv ),
                Utils::makeFixedString( "Los Angeles"sv ),
                Utils::makeFixedString( "New York"sv )
            } ),
        Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
        Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
        Json::ParserEvent( Json::ParserEventType::SetInputCount, 3 ),
        Json::ParserEvent( Json::ParserEventType::AddInput, 0 ),
        Json::ParserEvent( Json::ParserEventType::AddInput, 1 ),
        Json::ParserEvent( Json::ParserEventType::AddInput, 2 ),

        Json::ParserEvent( Json::ParserEventType::SetVertexCount, 8 ),
        Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 17 ),

        // 0 -> 1,2,3
        Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 2 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 3 ),

        // 1 -> 4,5
        Json::ParserEvent( Json::ParserEventType::AddEdge, 1, 4 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 1, 5 ),

        // 2 -> 5,6
        Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 5 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 6 ),

        // 3 -> 6
        Json::ParserEvent( Json::ParserEventType::AddEdge, 3, 6 ),

        // 4 -> 7,0 (cycle)
        Json::ParserEvent( Json::ParserEventType::AddEdge, 4, 7 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 4, 0 ),

        // 5 -> 7,2,3 (cross edges)
        Json::ParserEvent( Json::ParserEventType::AddEdge, 5, 7 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 5, 2 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 5, 3 ),

        // 6 -> 4,1
        Json::ParserEvent( Json::ParserEventType::AddEdge, 6, 4 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 6, 1 ),

        // 7 -> 0,6
        Json::ParserEvent( Json::ParserEventType::AddEdge, 7, 0 ),
        Json::ParserEvent( Json::ParserEventType::AddEdge, 7, 6 ),

        Json::ParserEvent( Json::ParserEventType::SetLabelCount, 3 ),
        Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
        Json::ParserEvent( Json::ParserEventType::AddLabel, 1 ),
        Json::ParserEvent( Json::ParserEventType::AddLabel, 2 ),

        Json::ParserEvent( Json::ParserEventType::Finish ),
    },
    .expected = {
        .type = Utils::JobType::Compute,
        .algorithm = Utils::AlgorithmType::BFS,
        .numInputs = 3,
        .inputs = { 0, 1, 2 },
        .graph = {
            .numVertices = 8,

            // CSR adjacency:
            // v0: 1,2,3
            // v1: 4,5
            // v2: 5,6
            // v3: 6
            // v4: 7,0
            // v5: 7,2,3
            // v6: 4,1
            // v7: 0,6
            .adj = Utils::makeArrayView<std::uint16_t>( g_allocator,
             {
                1, 2, 3,
                4, 5,
                5, 6,
                6,
                7, 0,
                7, 2, 3,
                4, 1,
                0, 6
            }),

            .offsets = Utils::makeArrayView<std::uint16_t>( g_allocator,
                {
                    0, 3, 5, 7, 8, 10, 13, 15, 17
                }
            ),

            .labels = Utils::makeArrayView<std::uint16_t>( g_allocator,
                    {
                     0, 1, 2
                    }
            )
        },
        .strings = Utils::makeArrayView<Utils::FixedString>(g_allocator,
                { Utils::makeFixedString( "Chicago"sv ),
                  Utils::makeFixedString( "Los Angeles"sv ),
                  Utils::makeFixedString( "New York"sv ) } )
    }
    },
};

INSTANTIATE_TEST_SUITE_P (
    StaticBuilderTests,
    BuilderTest,
    ::testing::ValuesIn(BuilderTests),
    [](const ::testing::TestParamInfo<BuilderTestCase> &parserTestInfo) {
        return parserTestInfo.param.name;
    }
);
