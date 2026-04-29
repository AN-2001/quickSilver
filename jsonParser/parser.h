/******************************************************************************\
*  parser.h                                                                    *
*                                                                              *
*  Defines a top-down parser for json.                                         *
*                                                                              *
*              Written by A.N.                                  29-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once
#include "utils/job.h"
#include "lexer.h"
#include <memory>

namespace Json {

    class Parser {
        private:
            Utils::Job &job;
            Lexer lexer;

            void expect( Token::Type tokenType );
            std::string expectString();
            Object parseJson();
            Object parseMap();
            Object parseArray();

        public:
            /* Parser can only be constructed via fd.                         */
            Parser( Utils::Job &job ) : job( job ), lexer( job ) {}

            Parser( const Parser &other ) = delete;
            Parser( Parser &&other ) = delete;
            Parser &operator=( const Parser &other ) = delete;
            Parser &operator=( Parser &&other ) = delete;

            ~Parser(){ /* Parser should not own anything. */ }
            void parse();

    };
};
