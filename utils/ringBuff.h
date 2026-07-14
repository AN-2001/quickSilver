#pragma once


#include "utils/allocator.h"
#include <cstddef>
namespace Utils {

    template <typename T>
    struct RingBuffer {
        Utils::Allocator &m_allocator;

        std::size_t m_size;
        std::size_t m_head, m_tail;
        T *m_storage;



    };


};
