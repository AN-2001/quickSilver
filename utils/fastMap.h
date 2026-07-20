#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace Utils {
    template < typename Key, typename Value, std::size_t N = 16 >
    class FastMap {
        std::array< std::pair< Key, Value >, N > inner;
        std::size_t curr{};

        public:
        constexpr void add( const Key &key, const Value &val ) {
            std::size_t i;
            for ( i = 0; i < curr && inner[ i ].first != key; ++i );
            if ( i < curr ) 
                throw std::logic_error( "Key already exists" );

            if ( curr == N )
                throw std::range_error( "fast map out of range" );
            inner[ curr++ ] = std::pair{ key, val };
        }

        [[nodiscard]] constexpr bool find( const Key &key, Value &val ) const {
            std::size_t i;
            for ( i = 0; i < curr && inner[ i ].first != key; ++i );
            if ( i == curr ) { return false; }
            val = inner[ i ].second;
            return true;
        }

        [[nodiscard]] constexpr Value get( const Key &key ) const {
            std::size_t i;
            for ( i = 0; i < curr && inner[ i ].first != key; ++i );
            return inner[ i ].second;
        }

    };
};
