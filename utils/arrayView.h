#pragma once

#include "utils/allocator.h"
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <span>

namespace Utils {

    template <typename T>
    using ArrayView = std::span< T >;

    template <typename T>
    inline Utils::ArrayView<T> makeArrayView( Utils::Allocator &allocator, std::size_t count ) 
    {
        if ( !count )
            return ArrayView<T>{};
        return ArrayView<T>{ reinterpret_cast< T* >( allocator.allocate( count * sizeof( T ) ,
                                                     alignof( T ) ) ),
                             count };
    }

    template <typename T>
    inline Utils::ArrayView<T> makeArrayView( Utils::Allocator &allocator, std::size_t count, const T& initVal ) 
    {
        if ( !count )
            return ArrayView<T>{};
        Utils::ArrayView<T> arr = makeArrayView<T>( allocator, count );
        std::fill_n( arr.data(), arr.size(), initVal );
        return arr;
    }

    template <typename T>
    inline Utils::ArrayView<T> makeArrayView( Utils::Allocator &allocator, std::initializer_list<T> elems ) 
    {
        T *ptr = reinterpret_cast< T* >( allocator.allocate( elems.size() * sizeof( T ) , alignof( T ) ) );
        T *iter = reinterpret_cast< T* >( ptr );
        for ( const auto &elem : elems ) {
            *iter = elem;
            iter++;
        }
        return ArrayView<T>{ reinterpret_cast< T* >( ptr ), elems.size() };
    }


};
