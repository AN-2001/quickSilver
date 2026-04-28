#include "lexer.h"
#include "jsonObject.h"
#include "token.h"
#include <ctype.h>
#include <exception>
#include <string>
#include "exceptions.h"

#define PROCESS_BUFFER_STATE_FREE       (0)
#define PROCESS_BUFFER_STATE_NUMBER_NEG (1)
#define PROCESS_BUFFER_STATE_NUMBER     (2)
#define PROCESS_BUFFER_STATE_STRING     (3)
#define PROCESS_BUFFER_STATE_T          (4)
#define PROCESS_BUFFER_STATE_TR         (5)
#define PROCESS_BUFFER_STATE_TRU        (6)
#define PROCESS_BUFFER_STATE_F          (7)
#define PROCESS_BUFFER_STATE_FA         (8)
#define PROCESS_BUFFER_STATE_FAL        (9)
#define PROCESS_BUFFER_STATE_FALS       (10)
#define PROCESS_BUFFER_STATE_N          (11)
#define PROCESS_BUFFER_STATE_NU         (12)
#define PROCESS_BUFFER_STATE_NUL        (13)

namespace GraphToys {


    void JsonLexer::processBuffer( int state )
    {
       if ( bufferPos == bufferSize ) {
            bufferPos = 0;
            bufferSize = job.read( buffer, MAX_BUFFER_SIZE );
            if ( !bufferSize ) {
                if ( state == PROCESS_BUFFER_STATE_FREE )
                    lastToken.emplace( JsonToken::Type::Eof );
                else
                    throw JsonParseError( "Unexpected EOF." );
                return;
            }
       }

       if ( state == PROCESS_BUFFER_STATE_STRING ) {
           if ( buffer[ bufferPos ] == '"' ) {
               bufferPos++;
               lastToken.emplace( std::string( (char*)scratch, scratchPos )  );
               scratchPos = 0;
               return;
           }
           scratch[ scratchPos++ ] = buffer[ bufferPos++ ];
           processBuffer( state );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_NUMBER || state == PROCESS_BUFFER_STATE_NUMBER_NEG ) {
           if ( !std::isdigit( buffer[ bufferPos ] ) ) {
               /* TODO: Catch exceptions here and rethrow, this needs a bit of a small redesign. */
               double res = std::stod( std::string( (char*)scratch, scratchPos )  );
               
               if ( state == PROCESS_BUFFER_STATE_NUMBER_NEG )
                   res = res * -1;
               lastToken.emplace( res );
               scratchPos = 0;
               return;
           }
           scratch[ scratchPos++ ] = buffer[ bufferPos++ ];
           processBuffer( state );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_T ) {
           if ( buffer[ bufferPos++ ] != 'r' )
               throw JsonParseError( "Unexpected literal, expecting 'true'" );
           processBuffer( PROCESS_BUFFER_STATE_TR );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_TR ) {
           if ( buffer[ bufferPos++ ] != 'u' )
               throw JsonParseError( "Unexpected literal, expecting 'true'" );
           processBuffer( PROCESS_BUFFER_STATE_TRU );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_TRU ) {
           if ( buffer[ bufferPos++ ] != 'e' ) 
               throw JsonParseError( "Unexpected literal, expecting 'true'" );
           lastToken.emplace( true );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_F ) {
           if ( buffer[ bufferPos++ ] != 'a' )
               throw JsonParseError( "Unexpected literal, expecting 'false'" );
           processBuffer( PROCESS_BUFFER_STATE_FA );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_FA ) {
           if ( buffer[ bufferPos++ ] != 'l' )
               throw JsonParseError( "Unexpected literal, expecting 'false'" );
           processBuffer( PROCESS_BUFFER_STATE_FAL );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_FAL ) {
           if ( buffer[ bufferPos++ ] != 's' )
               throw JsonParseError( "Unexpected literal, expecting 'false'" );
           processBuffer( PROCESS_BUFFER_STATE_FALS );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_FALS ) {
           if ( buffer[ bufferPos++ ] != 'e' )
               throw JsonParseError( "Unexpected literal, expecting 'false'" );
           lastToken.emplace( false );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_N ) {
           if ( buffer[ bufferPos++ ] != 'u' )
               throw JsonParseError( "Unexpected literal, expecting 'null'" );
           processBuffer( PROCESS_BUFFER_STATE_NU );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_NU ) {
           if ( buffer[ bufferPos++ ] != 'l' )
               throw JsonParseError( "Unexpected literal, expecting 'null'" );
           processBuffer( PROCESS_BUFFER_STATE_NUL );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_NUL ) {
           if ( buffer[ bufferPos++ ] != 'l' )
               throw JsonParseError( "Unexpected literal, expecting 'null'" );
           lastToken.emplace( JsonToken::Type::Null );
           return;
       }

       while( bufferPos < bufferSize && std::isspace( static_cast< unsigned char > ( buffer[ bufferPos ] ) ) )
           bufferPos++;

       if ( bufferPos == bufferSize ) {
           processBuffer( state );
           return;
       }

       switch ( buffer[ bufferPos ] ) {
            case '{':
                bufferPos++;
                lastToken.emplace( JsonToken::Type::LeftBrace );
                break;
            case '}':
                bufferPos++;
                lastToken.emplace( JsonToken::Type::RightBrace );
                break;
            case '[':
                bufferPos++;
                lastToken.emplace( JsonToken::Type::LeftBracket );
                break;
            case ']':
                bufferPos++;
                lastToken.emplace( JsonToken::Type::RightBracket );
                break;
            case ':':
                bufferPos++;
                lastToken.emplace( JsonToken::Type::Colon );
                break;
            case ',':
                bufferPos++;
                lastToken.emplace( JsonToken::Type::Comma );
                break;
            case '"':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_STRING );
                break;
            case '-':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_NUMBER_NEG );
                break;
            case '0': case '1': case '2': case '3': case '4': 
            case '5': case '6': case '7': case '8': case '9':
                processBuffer( PROCESS_BUFFER_STATE_NUMBER );
                break;
            case 't':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_T );
                break;
            case 'f':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_F );
                break;
            case 'n':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_N );
                break;
       }
    }

    JsonToken JsonLexer::peekToken() 
    {
        if ( lastToken.has_value() )
            return lastToken.value();
        processBuffer();
        return lastToken.value();
    }

    JsonToken JsonLexer::getToken() 
    {
        if ( lastToken.has_value() ) {
            JsonToken ret = lastToken.value();
            if ( ret.type != JsonToken::Type::Eof )
                lastToken.reset();
            return ret;
        }
        processBuffer();
        JsonToken ret = lastToken.value();
        if ( ret.type != JsonToken::Type::Eof )
            lastToken.reset();
        return ret;
    }

};
