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

namespace GraphToys {

    class JsonLexer {
        private:
            static constexpr size_t MAX_BUFFER_SIZE = 4096;

            Job &job;

            size_t bufferPos = 0;
            size_t bufferSize = 0;
            uint8_t buffer[ MAX_BUFFER_SIZE ]{};

            size_t scratchPos = 0;
            size_t scratchSize = 0;
            uint8_t scratch[ MAX_BUFFER_SIZE ]{};
            
            std::optional< JsonToken > lastToken;

        private:
            void processBuffer( int state = 0 );

        public:
            /* Parser can only be constructed via fd.                         */
            JsonLexer( Job &job ) : job( job ) {}
            JsonLexer( const JsonLexer &other ) = delete;
            JsonLexer( JsonLexer &&other ) = delete;
            JsonLexer &operator=( const JsonLexer &other ) = delete;
            JsonLexer &operator=( JsonLexer &&other ) = delete;

            ~JsonLexer(){ /* Lexer should not own anything. */ }

            JsonToken getToken();
            JsonToken peekToken();
            bool isEof() const;
    };
};
