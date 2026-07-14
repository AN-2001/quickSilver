#include <cstddef>
#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>

#include "jobParser/errors.h"
#include "jobParser/lexer.h"
#include "jobParser/parser.h"
#include "jobParser/parserEvents.h"
#include "utils/allocator.h"
#include "utils/arena.h"
#include "utils/job.h"
#include "utils/managedFd.h"

using namespace std::literals::string_view_literals;

struct ParserTestCase {
    const char *name;
    std::string_view json;
    std::vector<Json::ParserEvent> expected;
    std::vector<std::string_view> labels;
    Json::Error expectedError;
};

class ParserTest : public ::testing::TestWithParam<ParserTestCase> {};

TEST_P(ParserTest, ProducesExpectedEvents) {
    const auto &testParams = GetParam();
    int mockFds[ 2 ];

    if ( ::pipe( mockFds ) < 0 )
        FAIL() << "pipe call failed";

    int devNull = ::open( "/dev/null", O_RDWR );
    if ( devNull < 0 ) 
        FAIL() << "Could not open /dev/null";

    Utils::Arena arena( 1024 );
    Utils::Allocator allocator( arena );
    Utils::Job mockJob( Utils::BorrowedFd{ mockFds[ 0 ] }, Utils::BorrowedFd{ devNull } );
    Json::Lexer lexer( mockJob );
    Json::Parser parser( lexer, allocator );

    ssize_t written;

    written = ::write( mockFds[ 1 ], testParams.json.data(), testParams.json.size() );
    if ( written < 0 ||
            static_cast<std::size_t>( written ) != testParams.json.size() )
        FAIL() << "Could not write entire json to pipe";
    ::close( mockFds[ 1 ] );

    Json::Error err = parser.parse();

    auto strings = parser.releaseStrings();

    ASSERT_EQ( err, testParams.expectedError ) << "Errors don't match";

    if ( err != Json::Error::NoError ) {
        ::close(mockFds[0]);
        ::close(devNull);
        return;
    }

    std::size_t index = 0;
    for ( const auto &event : parser ) {
        ASSERT_LT( index, testParams.expected.size() ) << "Parser emitted too many events";

        const auto &expectedEvent = testParams.expected[ index ];

        ASSERT_EQ( event.m_type, expectedEvent.m_type ) << " : Mismatching event type at index " << index;;
        if ( event.m_type != Json::ParserEventType::AddLabel ) {
            ASSERT_EQ( event.m_ident0, expectedEvent.m_ident0 ) << " : Mismatching ident0 at index " << index;
            ASSERT_EQ( event.m_ident1, expectedEvent.m_ident1 ) << " : Mismatching ident1 at index " << index;
        } else {
            ASSERT_EQ( strings[ event.m_ident1 ].toView(), testParams.labels[ expectedEvent.m_ident1 ] ) << " : Mismatching labels at index " << index;
        }
        index++;
    }

    ASSERT_EQ( index, testParams.expected.size() );
    
    ::close(mockFds[0]);
    ::close(devNull);
}

static const ParserTestCase ParserTests[] = {
    {
        .name = "test_algorithm",
        .json = R"JSON(
            { 
                "algorithm": "BFS"
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {},
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_input",
        .json = R"JSON(
            { 
                "input": [ 0, 1, 2 ]
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 2 ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {},
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_algorithm_input_order",
        .json = R"JSON(
            { 
                "input": [ 0, 1, 2 ],
                "algorithm": "DFS"
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Dfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 2 ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {},
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_empty_json",
        .json = R"JSON(
            { 
            }
        )JSON",
        .expected = {
        },
        .labels = {},
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_bad_comma",
        .json = R"JSON(
            { 
                "algorithm": "BFS",
            }
        )JSON",
        .expected = {
        },
        .labels = {},
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_real_input",
        .json = R"JSON(
            { 
                "jobType": "compute",
                "algorithm": "BFS",
                "input": [ 0 ],
                "graph": {
                    "numVertices": 3,
                    "edges": [
                        [ 0, 1 ],
                        [ 1, 2 ],
                        [ 2, 1 ]
                    ],
                    "labels": [
                        "Los Angeles",
                        "San Jose",
                        "San Francisco"
                    ]
                }
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 0 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 1, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 2 ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {
            "Los Angeles"sv,
            "San Jose"sv,
            "San Francisco"sv,
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_missing_colon",
        .json = R"JSON(
            { 
                "input" [ 0 ]
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_bad_input_format",
        .json = R"JSON(
            { 
                "input": {
                    "src" : 5
                }
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_bad_graph_format",
        .json = R"JSON(
            { 
                "graph": []
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_bad_edges_format",
        .json = R"JSON(
            { 
                "graph": {
                    "edges": {}
                }
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_bad_token",
        .json = R"JSON(
            { 
                graph: {}
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_num_vertices_not_a_number",
        .json = R"JSON(
            { 
                "graph": {
                    "numVertices": "five"
                }
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_label_not_a_string",
        .json = R"JSON(
            { 
                "graph": {
                    "labels": [
                        5
                    ]
                }
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_unknown_algorithm",
        .json = R"JSON(
            { 
                "algorithm": "does not exist"
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_graph_as_a_string",
        .json = R"JSON(
            { 
                "graph": "am a graph!"
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_edges_bad_size",
        .json = R"JSON(
            { 
                "graph": {
                    "edges": [
                        [1, 2, 3]
                    ]
                }
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_input_mixed",
        .json = R"JSON(
            { 
                "input": [ 1, "hello", 2, [], {} ]
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_bad_job_type",
        .json = R"JSON(
            { 
                "jobType": "does not exist"
            }
        )JSON",
        .expected = {
        },
        .labels = {
        },
        .expectedError = Json::Error::UnexpectedToken
    },
    {
        .name = "test_metrics_job_type",
        .json = R"JSON(
            { 
                "jobType": "metrics"
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Metrics ) ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_empty_input",
        .json = R"JSON(
            { 
                "input": []
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_empty_edges",
        .json = R"JSON(
            { 
                "graph": {
                    "edges": []
                }
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_sorted_edges",
        .json = R"JSON(
            { 
                "graph": {
                    "edges": [
                        [ 3, 4 ],
                        [ 1, 2 ],
                        [ 5, 2 ],
                        [ 0, 1 ],
                        [ 2, 3 ],
                        [ 10, 11 ],
                        [ 0, 5 ],
                        [ 4, 4 ]
                    ]
                }
            }
        )JSON",
        .expected = {
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 8 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 5 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 1, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 3 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 3, 4 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 4, 4 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 5, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 10, 11 ),
            Json::ParserEvent( Json::ParserEventType::Finish )
        },
        .labels = {
        },
        .expectedError = Json::Error::NoError
    },
};

INSTANTIATE_TEST_SUITE_P (
    StaticParserTests,
    ParserTest,
    ::testing::ValuesIn(ParserTests),
    [](const ::testing::TestParamInfo<ParserTestCase> &parserTestInfo) {
        return parserTestInfo.param.name;
    }
);
