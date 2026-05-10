/******************************************************************************\
*  lexer.h                                                                     *
*                                                                              *
*  Streaming json lexer, manages the buffers and provides get/peek token       *
*  functionality.                                                              *
*                                                                              *
*              Written by A.N.                                  23-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include "token.h"
#include "utils/job.h"

namespace Json {
    class Lexer {
        private:
            static constexpr size_t MAX_BUFFER_SIZE = 4096;

            Utils::Job &job;

            size_t bufferPos = 0;
            size_t bufferSize = 0;
            uint8_t buffer[ MAX_BUFFER_SIZE ]{};

            size_t scratchPos = 0;
            size_t scratchSize = 0;
            uint8_t scratch[ MAX_BUFFER_SIZE ]{};
            
            std::optional< Token > lastToken;

        private:
            void processBuffer( int state = 0 );

        public:
            /* Parser can only be constructed via fd.                         */
            Lexer( Utils::Job &job ) : job( job ) {}
            Lexer( const Lexer &other ) = delete;
            Lexer( Lexer &&other ) = delete;
            Lexer &operator=( const Lexer &other ) = delete;
            Lexer &operator=( Lexer &&other ) = delete;

            ~Lexer(){ /* Lexer should not own anything. */ }

            Token getToken();
            const Token &peekToken();
    };
};
