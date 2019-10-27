// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#include "stl_allocator.h"

template<>
class Allocator<void>
{
    typedef void        value_type;
    typedef void*       pointer;
    typedef const void* const_pointer;
 
    template <class U>
    struct rebind
    {
        typedef Allocator<U> other;
    };
};
