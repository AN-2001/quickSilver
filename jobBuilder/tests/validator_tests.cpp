#include <gtest/gtest.h>

#include "jobBuilder/jobValidator.h"
#include <string>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include "eventWrapper.h"

#include "jobParser/parserEvents.h"
#include "jobParser/token.h"

using namespace std::literals::string_view_literals;

struct ValidatorTestCase {
    const char *name;
    EventWrapper input;
    bool expected;
};

class ValidatorTest : public ::testing::TestWithParam<ValidatorTestCase> {};

TEST_P(ValidatorTest, HandlesEventSequence) {
    const auto &testParams = GetParam();

    JobTools::Validator validator( testParams.input );

    ASSERT_EQ( validator.validate(), testParams.expected );
}

static const ValidatorTestCase ValidatorTests[] = {
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
        .expected = true
    },
    {
        .name = "test_bad_order_0",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_bad_order_1",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_bad_order_2",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_bad_order_3",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_bad_order_4",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_0",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_1",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_2",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_3",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_4",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_5",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_6",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_early_finish_7",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_no_finish",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
        },
        .expected = false
    },
    {
        .name = "test_too_much_input_count",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 6 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_too_little_input_count",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_too_much_edge_count",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 0 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_too_little_edge_count",
        .input = { 0,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },

    {
        .name = "test_with_label",
        .input = { 1,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = true
    },
    {
        .name = "test_too_much_label_count",
        .input = { 2,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_too_little_label_count",
        .input = { 2,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_edge_out_of_range",
        .input = { 1,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 2, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 0 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
    {
        .name = "test_label_out_of_range",
        .input = { 1,
            Json::ParserEvent( Json::ParserEventType::SetJobType, std::to_underlying( Json::Token::Compute ) ),
            Json::ParserEvent( Json::ParserEventType::SetAlgorithm, std::to_underlying( Json::Token::Bfs ) ),
            Json::ParserEvent( Json::ParserEventType::SetInputCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddInput, 5 ),
            Json::ParserEvent( Json::ParserEventType::SetVertexCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::SetEdgeCount, 2 ),
            Json::ParserEvent( Json::ParserEventType::AddEdge, 0, 1 ),
            Json::ParserEvent( Json::ParserEventType::SetLabelCount, 1 ),
            Json::ParserEvent( Json::ParserEventType::AddLabel, 1 ),
            Json::ParserEvent( Json::ParserEventType::Finish ),
        },
        .expected = false
    },
};

INSTANTIATE_TEST_SUITE_P (
    StaticValidatorTests,
    ValidatorTest,
    ::testing::ValuesIn(ValidatorTests),
    [](const ::testing::TestParamInfo<ValidatorTestCase> &parserTestInfo) {
        return parserTestInfo.param.name;
    }
);
