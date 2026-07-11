#include <cstddef>
#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "jobParser/errors.h"
#include "jobParser/lexer.h"
#include "jobParser/token.h"
#include "utils/job.h"
#include "utils/managedFd.h"

using namespace std::literals::string_view_literals;

static constexpr float EPS = 1e-5f;

struct LexerTestCase {
    const char *name;
    std::string_view json;
    std::vector<Json::TokenWrapper> expected;
    Json::Error expectedError;
};

class LexerTest : public ::testing::TestWithParam<LexerTestCase> {};

TEST_P(LexerTest, ProducesExpectedTokens) {
    const auto &testParams = GetParam();
    int mockFds[ 2 ];

    if ( ::pipe( mockFds ) < 0 )
        FAIL() << "pipe call failed";

    int devNull = ::open( "/dev/null", O_RDWR );
    if ( devNull < 0 ) 
        FAIL() << "Could not open /dev/null";

    Utils::Job mockJob( Utils::BorrowedFd{ mockFds[ 0 ] }, devNull );
    Json::Lexer lexer( mockJob );

    ssize_t written;

    written = ::write( mockFds[ 1 ], testParams.json.data(), testParams.json.size() );
    if ( written < 0 ||
            static_cast<std::size_t>( written ) != testParams.json.size() )
        FAIL() << "Could not write entire json to pipe";
    ::close( mockFds[ 1 ] );

    int index = 0;
    for ( const auto &expToken : testParams.expected ) {
        auto tok = lexer.get();
        if ( !tok ) {
            if ( tok.error() != testParams.expectedError )
                FAIL() << Json::errorToString( tok.error() ) << " :: unexpected Lexing error at " << index;
            else
                break;
        }

        ASSERT_EQ( tok -> m_type, expToken.m_type ) << "Token mismatch at " << index;
        switch ( tok -> m_type ) {
            case Json::Token::Number:
                ASSERT_NEAR( tok -> getNumericValue(), expToken.getNumericValue(), EPS ) << "Float value mismatch at " << index;
                break;
            case Json::Token::Label:
                ASSERT_EQ( tok -> getLabelValue(), expToken.getLabelValue() ) << "Label value mismatch at " << index;
                break;

            case Json::Token::JobType:
            case Json::Token::Metrics:
            case Json::Token::Compute:
            case Json::Token::NumVertices:
            case Json::Token::Edges:
            case Json::Token::Labels:
            case Json::Token::Algorithm:
            case Json::Token::Graph:
            case Json::Token::Input:
            case Json::Token::Dfs:
            case Json::Token::Bfs:
            case Json::Token::LeftBracket:
            case Json::Token::RightBracket:
            case Json::Token::LeftBrace:
            case Json::Token::RightBrace:
            case Json::Token::Comma:
            case Json::Token::Colon:
            case Json::Token::Null:
            case Json::Token::True:
            case Json::Token::False:
            case Json::Token::Eof:
                break;
            case Json::Token::NumTokens:
            default:
                FAIL() << "Unrecognized/unacceptable token detected";
                break;
        }
        index++;
    }

    auto tok = lexer.get();
    if ( tok ) {
        ASSERT_EQ( tok -> m_type, Json::Token::Eof );
    }

    
    ::close(mockFds[0]);
    ::close(devNull);
}

static const LexerTestCase LexerTests[] = {
    {
        .name = "test_empty_json",
        .json = R"JSON()JSON"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_empty_json_object",
        .json = R"JSON( 
        {}
        )JSON"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::RightBrace),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_whitespaces",
        .json = "\t\r\n {}"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::RightBrace),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_true",
        .json = "true"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::True),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_null",
        .json = "null"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::Null),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_negative_zero",
        .json = "-0"sv,
        .expected = {
            Json::TokenWrapper(0.f),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_bad_true",
        .json = "truee"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_truefalse",
        .json = "truefalse"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_false",
        .json = "falsee"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_null",
        .json = "nulll"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_nulltruefalse",
        .json = "nulltruefalse"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_number_true",
        .json = "1true"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_number_false",
        .json = "1false"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_bad_number_null",
        .json = "1null"sv,
        .expected = {
        },
        .expectedError = Json::Error::UnknownToken
    },
    {
        .name = "test_keyword_sep",
        .json = "true{false{null{"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::True),
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::False),
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::Null),
            Json::TokenWrapper(Json::Token::LeftBrace),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_all_tokens",
        .json = R"JSON( 
        ,{}[]:"graph""algorithm""input""numVertices""edges"true false null
        0.1 1.5 1e2 10 -0.1 "Chicago" "labels" "BFS""DFS" "jobType" "metrics" "compute"
        )JSON"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::RightBrace),
            Json::TokenWrapper(Json::Token::LeftBracket),
            Json::TokenWrapper(Json::Token::RightBracket),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::Graph),
            Json::TokenWrapper(Json::Token::Algorithm),
            Json::TokenWrapper(Json::Token::Input),
            Json::TokenWrapper(Json::Token::NumVertices),
            Json::TokenWrapper(Json::Token::Edges),
            Json::TokenWrapper(Json::Token::True),
            Json::TokenWrapper(Json::Token::False),
            Json::TokenWrapper(Json::Token::Null),
            Json::TokenWrapper(0.1f),
            Json::TokenWrapper(1.5f),
            Json::TokenWrapper(1e2f),
            Json::TokenWrapper(10.0f),
            Json::TokenWrapper(-0.1f),
            Json::TokenWrapper("Chicago"sv),
            Json::TokenWrapper(Json::Token::Labels),
            Json::TokenWrapper(Json::Token::Bfs),
            Json::TokenWrapper(Json::Token::Dfs),
            Json::TokenWrapper(Json::Token::JobType),
            Json::TokenWrapper(Json::Token::Metrics),
            Json::TokenWrapper(Json::Token::Compute),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },
    {
        .name = "test_real_payload",
        .json = R"JSON( 
        {
            "jobType": "compute",
            "algorithm": "BFS",
            "input": [ 0 ],
            "graph": {
                "numVertices": 3,
                "edges": [
                    [0, 1],
                    [0, 2]
                ],
                "labels": [
                    "Chicago",
                    "San Francisco",
                    "Los Angeles"
                ]
            }
        }
        )JSON"sv,
        .expected = {
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::JobType),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::Compute),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::Algorithm),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::Bfs),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::Input),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::LeftBracket),
            Json::TokenWrapper(0.f),
            Json::TokenWrapper(Json::Token::RightBracket),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::Graph),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::LeftBrace),
            Json::TokenWrapper(Json::Token::NumVertices),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(3.f),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::Edges),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::LeftBracket),
            Json::TokenWrapper(Json::Token::LeftBracket),
            Json::TokenWrapper(0.f),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(1.f),
            Json::TokenWrapper(Json::Token::RightBracket),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::LeftBracket),
            Json::TokenWrapper(0.f),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(2.f),
            Json::TokenWrapper(Json::Token::RightBracket),
            Json::TokenWrapper(Json::Token::RightBracket),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper(Json::Token::Labels),
            Json::TokenWrapper(Json::Token::Colon),
            Json::TokenWrapper(Json::Token::LeftBracket),
            Json::TokenWrapper("Chicago"sv),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper("San Francisco"sv),
            Json::TokenWrapper(Json::Token::Comma),
            Json::TokenWrapper("Los Angeles"sv),
            Json::TokenWrapper(Json::Token::RightBracket),
            Json::TokenWrapper(Json::Token::RightBrace),
            Json::TokenWrapper(Json::Token::RightBrace),
            Json::TokenWrapper(Json::Token::Eof),
        },
        .expectedError = Json::Error::NoError
    },

};

INSTANTIATE_TEST_SUITE_P (
    StaticLexerTests,
    LexerTest,
    ::testing::ValuesIn(LexerTests),
    [](const ::testing::TestParamInfo<LexerTestCase> &lexerTestInfo) {
        return lexerTestInfo.param.name;
    }
);
