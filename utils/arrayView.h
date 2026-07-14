#pragma once

#include "utils/allocator.h"
#include <algorithm>
#include <cstddef>
#include <initializer_list>

namespace Utils {

    template <typename T>
    struct ArrayView 
    {
        T *m_base{};
        std::size_t m_count{};

        [[nodiscard]] T &operator[]( std::size_t idx ) const noexcept
        {
            return m_base[ idx ];
        }

        [[nodiscard]] std::size_t size() const noexcept 
        {
            return m_count;
        }

        [[nodiscard]] T* data() const noexcept
        {
            return m_base;
        }

        [[nodiscard]] T* begin() const noexcept
        {
            return m_base;
        }

        [[nodiscard]] T* end() const noexcept
        {
            return m_base + m_count;
        }
    };

    template <typename T>
    inline Utils::ArrayView<T> makeArrayView( Utils::Allocator &allocator, std::size_t count ) 
    {
        if ( !count )
            return ArrayView<T>{ nullptr, 0 };
        std::uintptr_t ptr = allocator.allocate( count * sizeof( T ) , alignof( T ) );
        return ArrayView<T>{ reinterpret_cast< T*>( ptr ), count };
    }

    template <typename T>
    inline Utils::ArrayView<T> makeArrayView( Utils::Allocator &allocator, std::size_t count, const T& initVal ) 
    {
        if ( !count )
            return ArrayView<T>{ nullptr, 0 };

        Utils::ArrayView<T> arr = makeArrayView<T>( allocator, count );
        std::fill_n( arr.data(), arr.size(), initVal );
        return arr;
    }

    template <typename T>
    inline Utils::ArrayView<T> makeArrayView( Utils::Allocator &allocator, std::initializer_list<T> elems ) 
    {
        std::uintptr_t ptr = allocator.allocate( elems.size() * sizeof( T ) , alignof( T ) );
        T *iter = reinterpret_cast< T* >( ptr );
        for ( const auto &elem : elems ) {
            *iter = elem;
            iter++;
        }
        return ArrayView<T>{ reinterpret_cast< T* >( ptr ), elems.size() };
    }


};
