/******************************************************************************\
*  jsonObject.h                                                                *
*                                                                              *
*  This header describes how a json object should behave in our context.       *
*  It implements a recursive type that leverages std::variant to give an ergo- *
*  nomic interface for manipulating json.                                      *
*                                                                              *
*              Written by A.N.                                  29-04-2026     *
*                                                                              *
\******************************************************************************/

#pragma once

#include <exception>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include "exceptions.h"

static std::string escapeString( const std::string &str ) {
    std::stringstream ss;
    ss << "\"";
    for ( unsigned char c : str ) {
        if ( c == '\n' )
            ss << "\\n";
        else if ( c == '\r' )
            ss << "\\r";
        else if ( c == '\t' )
            ss << "\\t";
        else if ( c == '"' )
            ss << "\\\"";
        else if ( c == '\\' )
            ss << "\\\\";
        else if ( c == '\b' )
            ss << "\\b";
        else if ( c == '\f' )
            ss << "\\f";
        else
            ss << c;
    }
    ss << "\"";
    return ss.str();
}

namespace Json {
    struct Object {
        using JsonMap = std::unordered_map< std::string, Object >;
        using JsonArray = std::vector< Object >;

        using Value =
                std::variant<
                 std::monostate,
                 JsonMap,
                 JsonArray,
                 std::string,
                 bool,
                 double
                >;
        Value value;

        Object() : value( std::monostate() ) {}

        Object( JsonMap &&map ) : value( std::move( map ) )  {}
        Object( JsonArray &&arr ) : value( std::move( arr ) ) {}

        Object( const std::string &str ) : value( str ) {}
        Object( std::string &&str ) : value( std::move( str ) ) {}
        Object( const char *str ) : value( std::string( str ) ) {}

        Object( bool b ) : value( b ) {}

        Object( double n ) : value( n ) {}
        Object( int n ) : value( static_cast< double >( n ) ) {}

        void push_back( Object &obj ) 
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonArray();
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonKeyError( "Json is not an array" );
            auto &arr = std::get< JsonArray > ( value );
            arr.push_back( std::move( obj ) ) ;
        }

        Object &operator[]( const std::string &key )
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonMap();
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not an object" );
            auto &map = std::get<JsonMap>( value );
            if ( !map.count( key ) ) 
                map[ key ] = Object();
            return map[ key ];
        }

        Object &operator[]( const char *key )
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonMap();
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not an object" );
            auto &map = std::get<JsonMap>( value );
            if ( !map.count( key ) ) 
                map[ key ] = Object();
            return map[ key ];
        }

        Object &operator[]( int index )
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonArray();
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonKeyError( "Json is not an array" );
            auto &arr = std::get<JsonArray>( value );
            if ( arr.size() <= static_cast< std::size_t >( index ) ) { 
                arr.resize( static_cast< std::size_t >( index + 1 ) );
                arr[ index ] = Object();
            }
            return arr[ index ];
        }

        operator double() const {
            if ( !std::holds_alternative< double >( value ) )
                throw JsonException( "Cannot convert JSON object to 'double" );
            return std::get< double > ( value );
        }

        operator int() const {
            if ( !std::holds_alternative< double >( value ) )
                throw JsonException( "Cannot convert JSON object to 'int" );
            return static_cast< int > ( std::get< double > ( value ) );
        }

        operator bool() const {
            if ( !std::holds_alternative< bool >( value ) )
                throw JsonException( "Cannot convert JSON object to 'bool" );
            return std::get< bool > ( value );
        }

        operator std::string() const {
            if ( !std::holds_alternative< std::string >( value ) )
                throw JsonException( "Cannot convert JSON object to 'std::string" );
            return std::get< std::string > ( value );
        }

        operator const char*() const {
            if ( !std::holds_alternative< std::string >( value ) )
                throw JsonException( "Cannot convert JSON object to 'char*'" );
            return std::get< std::string > ( value ).c_str();;
        }


        public:

        std::string serialize() const {
            struct Visitor {
                std::string operator()( std::monostate ) const {
                    return "null";
                }

                std::string operator()( const std::string &str ) const {
                    return escapeString( str );
                }

                std::string operator()( const JsonMap &map ) const {
                    std::stringstream ss;
                    bool first = true;

                    ss << "{";
                    for ( const auto &pair : map ) {
                        if ( first ) {
                            ss << escapeString( pair.first );
                            ss << ":";
                            ss << pair.second.serialize();
                            first = false;
                        } else {
                            ss << ",";
                            ss << escapeString( pair.first );
                            ss << ":";
                            ss << pair.second.serialize();
                        }
                    }
                    ss << "}";

                    return ss.str();
                }

                std::string operator()( const JsonArray &arr ) const {
                    std::stringstream ss;
                    bool first = true;

                    ss << "[";
                    for ( const auto &json : arr ) {
                        if ( first ) {
                            ss << json.serialize();
                            first = false;
                        } else {
                            ss << ",";
                            ss << json.serialize();
                        }
                    }
                    ss << "]";

                    return ss.str();
                }

                std::string operator()( bool b ) const {
                    return b ? "true" : "false";
                }

                std::string operator()( double d ) const {
                    return std::to_string( d );
                }
            };

            return std::visit( Visitor{}, value );
        }


    };
};
