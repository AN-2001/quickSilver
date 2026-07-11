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
            for ( i = 0; i < curr && inner[ i ].first != key; i++ );
            if ( i < curr ) 
                throw std::logic_error( "Key already exists" );

            if ( curr == N )
                throw std::range_error( "fast map out of range" );
            inner[ curr++ ] = std::pair{ key, val };
        }

        [[nodiscard]] constexpr Value get( const Key &key ) const {
            std::size_t i;
            for ( i = 0; i < curr && inner[ i ].first != key; i++ );
            if ( i == curr ) throw std::range_error( "Key does not exist" );
            return inner[ i ].second;
        }

        [[nodiscard]] constexpr bool exists( const Key &key ) const {
            std::size_t i;
            for ( i = 0; i < curr && inner[ i ].first != key; i++ );
            return i < curr;
        }

    };
};
