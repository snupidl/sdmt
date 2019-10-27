// Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr

#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include "common.h"

template <class T>
class Allocator
{
  public:
    typedef T                   value_type;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;

    /*
     * constructor/destructor
     */
    Allocator() {}
    Allocator(const Allocator&) {}
    ~Allocator() {}

    /*
     * allocate/dealloacte
     */
    pointer allocate(size_type n, const_pointer = 0) {
        void* p = std::malloc(n);
        if(!p) throw std::bad_alloc();
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type)
    {
        std::free(p);
    }

    inline void construct(pointer p, const value_type& x)
    {
        new(p) value_type(x);
    }
 
    void destroy(pointer p)
    {
        p->~value_type();
    }

    /*
     * addressing
     */
    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return x; }

    /*
     * util functions
     */
    template <class U>
    struct rebind
    {
        typedef Allocator<U> other;
    };
 
private:
    void operator=(const Allocator&);
};
 
template <class T>
inline bool operator==(const Allocator<T>&, const Allocator<T>&)
{
    return true;
}
 
template <class T>
inline bool operator!=(const Allocator<T>&, const Allocator<T>&)
{
    return false;
}

#endif  // ALLOCATOR_H_
