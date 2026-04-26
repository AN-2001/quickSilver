#pragma once

#include <memory>
#include <unordered_map>
#include <variant>
#include <string>
#include <vector>

namespace GraphToys {
    struct JsonObject;
    using JsonPtr = std::unique_ptr< JsonObject >;
    using JsonMap = std::unordered_map< std::string, JsonPtr >;
    using JsonArray = std::vector< JsonPtr >;

    struct JsonObject {
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
    };

};
