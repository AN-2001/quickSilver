#include "lexer.h"
#include "object.h"
#include "token.h"
#include <cctype>
#include <ctype.h>
#include <exception>
#include <stdexcept>
#include <string>
#include "exceptions.h"
#include <charconv>
#include <system_error>

#define PROCESS_BUFFER_STATE_FREE       (0)
#define PROCESS_BUFFER_STATE_NUMBER     (1)
#define PROCESS_BUFFER_STATE_STRING     (2)
#define PROCESS_BUFFER_STATE_T          (3)
#define PROCESS_BUFFER_STATE_TR         (4)
#define PROCESS_BUFFER_STATE_TRU        (5)
#define PROCESS_BUFFER_STATE_TRUE       (6)
#define PROCESS_BUFFER_STATE_F          (7)
#define PROCESS_BUFFER_STATE_FA         (8)
#define PROCESS_BUFFER_STATE_FAL        (9)
#define PROCESS_BUFFER_STATE_FALS       (10)
#define PROCESS_BUFFER_STATE_FALSE      (11)
#define PROCESS_BUFFER_STATE_N          (12)
#define PROCESS_BUFFER_STATE_NU         (13)
#define PROCESS_BUFFER_STATE_NUL        (14)
#define PROCESS_BUFFER_STATE_NULL       (15)
#define PROCESS_BUFFER_STATE_COMMENT    (16)

namespace Json {
    static bool isNumberState( char c )
    {
        return std::isdigit( c ) || c == '-' || c == 'e' || c == 'E' || c == '.';
    }

    void Lexer::processBuffer( int state )
    {
       if ( bufferPos == bufferSize ) {
            bufferPos = 0;
            bufferSize = job.read( buffer, MAX_BUFFER_SIZE );
            if ( !bufferSize ) {
                if ( state == PROCESS_BUFFER_STATE_FREE ||
                     state == PROCESS_BUFFER_STATE_TRUE ||
                     state == PROCESS_BUFFER_STATE_FALSE ||
                     state == PROCESS_BUFFER_STATE_NULL )
                    lastToken.emplace( Token::Type::Eof );
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
           if ( scratchPos >= MAX_BUFFER_SIZE )
               throw JsonParseError( "Out of scratch space" );
           scratch[ scratchPos++ ] = buffer[ bufferPos++ ];
           processBuffer( state );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_NUMBER ) {
           if ( !isNumberState( buffer[ bufferPos ] ) ) {

               double res;
               auto [ ptr, err ] = std::from_chars(
                    (char*)scratch,
                    (char*)scratch + scratchPos,
                    res
               );
               if ( err == std::errc::invalid_argument ) 
                   throw JsonParseError( "Number is malformed" );
               if ( err == std::errc::result_out_of_range )
                   throw JsonParseError( "Number is out of range" );
               if ( ptr != (char*)scratch + scratchPos ) 
                   throw JsonParseError( "Number was not fully consumed" );

               lastToken.emplace( res );
               scratchPos = 0;
               return;
           }

           if ( scratchPos >= MAX_BUFFER_SIZE )
               throw JsonParseError( "Out of scratch space" );
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
           processBuffer( PROCESS_BUFFER_STATE_TRUE );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_TRUE ) {
           if ( !std::isspace( (unsigned char) buffer[ bufferPos ] ) &&
                buffer[ bufferPos ] != ',' &&
                buffer[ bufferPos ] != ':' &&
                buffer[ bufferPos ] != '[' &&
                buffer[ bufferPos ] != ']' &&
                buffer[ bufferPos ] != '{' &&
                buffer[ bufferPos ] != '}')
               throw JsonParseError( "Unexpected token after 'true'" );

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
           processBuffer( PROCESS_BUFFER_STATE_FALSE );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_FALSE ) {
           if ( !std::isspace( (unsigned char) buffer[ bufferPos ] ) &&
                buffer[ bufferPos ] != ',' &&
                buffer[ bufferPos ] != ':' &&
                buffer[ bufferPos ] != '[' &&
                buffer[ bufferPos ] != ']' &&
                buffer[ bufferPos ] != '{' &&
                buffer[ bufferPos ] != '}')
               throw JsonParseError( "Unexpected token after 'false'" );

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
           processBuffer( PROCESS_BUFFER_STATE_NULL );
           return;
       }

       if ( state == PROCESS_BUFFER_STATE_NULL ) {
           if ( !std::isspace( (unsigned char) buffer[ bufferPos ] ) &&
                buffer[ bufferPos ] != ',' &&
                buffer[ bufferPos ] != ':' &&
                buffer[ bufferPos ] != '[' &&
                buffer[ bufferPos ] != ']' &&
                buffer[ bufferPos ] != '{' &&
                buffer[ bufferPos ] != '}')
               throw JsonParseError( "Unexpected token after 'null'" );

           lastToken.emplace( Token::Type::Null );
           return;
       }

       while( bufferPos < bufferSize && (
               std::isspace( static_cast< unsigned char > ( buffer[ bufferPos ] ) )  ||
               state == PROCESS_BUFFER_STATE_COMMENT )
            ) {
           if ( buffer[ bufferPos ] == '\n' && state == PROCESS_BUFFER_STATE_COMMENT )
                   state = PROCESS_BUFFER_STATE_FREE;
           bufferPos++;
       }

       if ( bufferPos == bufferSize ) {
           processBuffer( state );
           return;
       }

       switch ( buffer[ bufferPos ] ) {
            case '{':
                bufferPos++;
                lastToken.emplace( Token::Type::LeftBrace );
                break;
            case '}':
                bufferPos++;
                lastToken.emplace( Token::Type::RightBrace );
                break;
            case '[':
                bufferPos++;
                lastToken.emplace( Token::Type::LeftBracket );
                break;
            case ']':
                bufferPos++;
                lastToken.emplace( Token::Type::RightBracket );
                break;
            case ':':
                bufferPos++;
                lastToken.emplace( Token::Type::Colon );
                break;
            case ',':
                bufferPos++;
                lastToken.emplace( Token::Type::Comma );
                break;
            case '"':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_STRING );
                break;

            case '-':
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
            case '#':
                bufferPos++;
                processBuffer( PROCESS_BUFFER_STATE_COMMENT );
                break;
            default:
                throw JsonParseError( "Unrecognized literal" );
       }
    }

    const Token &Lexer::peekToken() 
    {
        if ( lastToken.has_value() )
            return lastToken.value();
        processBuffer();
        return lastToken.value();
    }

    Token Lexer::getToken() 
    {
        if ( lastToken.has_value() ) {
            Token ret = std::move( lastToken.value() );
            if ( ret.type != Token::Type::Eof )
                lastToken.reset();
            return ret;
        }
        processBuffer();
        Token ret = std::move( lastToken.value() );
        if ( ret.type != Token::Type::Eof )
            lastToken.reset();
        return ret;
    }

};
