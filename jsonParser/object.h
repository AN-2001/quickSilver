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

        template <typename T >
        void push_back( T &&obj ) 
        {
            if ( !std::is_constructible_v< Object, T&& > )
                throw JsonIndexError( "Json array element element must be a Json::Object" );
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonKeyError( "Json is not an array" );
            auto &arr = std::get< JsonArray > ( value );
            arr.emplace_back( std::forward<T>( obj ) ) ;
        }

        void set_array_size( size_t n ) {
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonKeyError( "Json is not an array" );
            auto &arr = std::get< JsonArray > ( value );
            arr.resize( n );
        }

        template <typename T >
        void set_array_at_index( int index, T &&obj ) 
        {
            if ( !std::is_constructible_v< Object, T&& > )
                throw JsonIndexError( "Json array element element must be a Json::Object" );
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonKeyError( "Json is not an array" );
            auto &arr = std::get< JsonArray > ( value );
            arr[ index ] = std::forward<T>( obj );
        }

        template < typename T, typename K >
        void add_mapping( K &&str, T &&obj ) 
        {
            if ( !std::is_constructible_v< std::string, K&& > )
                throw JsonKeyError( "Json map key must be a string" );
            if ( !std::is_constructible_v< Object, T&& > )
                throw JsonKeyError( "Json map object must be a Json::Object" );
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not a map" );
            auto &map = std::get< JsonMap > ( value );
            map.emplace( std::forward<K>( str ), std::forward<T>( obj ) );
        }

        const Object &operator[]( const std::string &key ) const
        {
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not a map" );
            auto &map = std::get<JsonMap>( value );
            const auto &it = map.find( key );
            if ( it == map.end() )
                throw JsonKeyError( "Json key does not exist" );
            return (*it).second;
        }

        const Object &operator[]( const char *key ) const
        {
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not a a map" );
            auto &map = std::get<JsonMap>( value );
            const auto &it = map.find( key );
            if ( it == map.end() )
                throw JsonKeyError( "Json key does not exist" );
            return (*it).second;
        }

        const Object &operator[]( int index ) const
        {
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonIndexError( "Json is not an array" );
            auto &arr = std::get<JsonArray>( value );
            if ( arr.size() <= static_cast< std::size_t >( index ) )
                throw JsonIndexError( "Json index out of range" );
            return arr[ index ];
        }

        double toNumber() const {
            if ( !std::holds_alternative< double >( value ) )
                throw JsonException( "Cannot convert JSON object to 'double" );
            return std::get< double > ( value );
        }

        double toBool() const {
            if ( !std::holds_alternative< bool >( value ) )
                throw JsonException( "Cannot convert JSON object to 'bool" );
            return std::get< bool > ( value );
        }

        const std::string &toString() const {
            if ( !std::holds_alternative< std::string >( value ) )
                throw JsonException( "Cannot convert JSON object to 'std::string" );
            return std::get< std::string > ( value );
        }

        const JsonArray &toArray() const {
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonException( "Cannot convert JSON object to 'Json Array" );
            return std::get< JsonArray > ( value );
        }

        const JsonMap &toMap() const {
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonException( "Cannot convert JSON object to 'Json Map" );
            return std::get< JsonMap > ( value );
        }

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
                    return " " + std::to_string( (int)d );
                }
            };

            return std::visit( Visitor{}, value );
        }


    };
};
