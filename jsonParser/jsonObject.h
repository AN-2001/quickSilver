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
    ss << str;
    ss << "\"";
    return ss.str();
}

namespace GraphToys {
    struct JsonObject {
        using JsonMap = std::unordered_map< std::string, JsonObject >;
        using JsonArray = std::vector< JsonObject >;

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

        JsonObject() : value( std::monostate() ) {}

        JsonObject( JsonMap &&map ) : value( std::move( map ) )  {}
        JsonObject( JsonArray &&arr ) : value( std::move( arr ) ) {}

        JsonObject( const std::string &str ) : value( str ) {}
        JsonObject( std::string &&str ) : value( std::move( str ) ) {}
        JsonObject( const char *str ) : value( std::string( str ) ) {}

        JsonObject( bool b ) : value( b ) {}

        JsonObject( double n ) : value( n ) {}
        JsonObject( int n ) : value( static_cast< double >( n ) ) {}

        JsonObject &operator[]( const std::string &key )
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonMap();
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not an object" );
            auto &map = std::get<JsonMap>( value );
            if ( !map.count( key ) ) 
                map[ key ] = JsonObject();
            return map[ key ];
        }

        JsonObject &operator[]( const char *key )
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonMap();
            if ( !std::holds_alternative< JsonMap >( value ) )
                throw JsonKeyError( "Json is not an object" );
            auto &map = std::get<JsonMap>( value );
            if ( !map.count( key ) ) 
                map[ key ] = JsonObject();
            return map[ key ];
        }

        JsonObject &operator[]( int index )
        {
            if ( std::holds_alternative< std::monostate >( value ) ) 
                value = JsonArray();
            if ( !std::holds_alternative< JsonArray >( value ) )
                throw JsonKeyError( "Json is not an array" );
            auto &arr = std::get<JsonArray>( value );
            if ( arr.size() <= static_cast< std::size_t >( index ) ) { 
                arr.resize( static_cast< std::size_t >( index + 1 ) );
                arr[ index ] = JsonObject();
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
