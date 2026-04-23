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
#include "token.h"
#include <variant>
#include "../utils/uniqueFd.h"

namespace GraphToys {

    class JsonLexer {
        private:
            static constexpr size_t MAX_BUFFER_SIZE = 4096;

            BorrowedFd fd;

            size_t bufferPos;
            size_t bufferSize;
            uint8_t buffer[ MAX_BUFFER_SIZE ];

            size_t scratchPos;
            size_t scratchSize;
            uint8_t scratch[ MAX_BUFFER_SIZE ];
            
            std::optional< JsonToken > lastToken;

            bool eof;

        public:
            /* Parser can only be constructed via fd.                         */
            JsonLexer( BorrowedFd fd ) : fd( fd ) {}
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
