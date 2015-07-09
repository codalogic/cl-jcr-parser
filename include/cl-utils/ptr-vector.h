//----------------------------------------------------------------------------
// Copyright (c) 2015, Codalogic Ltd (http://www.codalogic.com)
// All rights reserved.
//
// The license for this file is based on the BSD-3-Clause license
// (http://www.opensource.org/licenses/BSD-3-Clause).
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// - Neither the name Codalogic Ltd nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ptr_vector is a container that stores a vector of pointers.  The benefits
// using it over vector<T*> is that it will deep-copy, the iterators return
// T& rather than T*, and it will delete the pointed to objects when the
// container is destroyed.  The benefits over deque<T> are that you can
// push_back() an independently new-ed object, rather than having to do a copy
// into place operation.
//----------------------------------------------------------------------------

#ifndef PTR_VECTOR
#define PTR_VECTOR

#include <cstddef>  // For size_t
#include <vector>
#include <memory>

namespace clutils {

template<typename T> struct dereference;
template<typename T> struct dereference<T*> { typedef T type; };
template<typename T> struct mkconst;
template<typename T> struct mkconst<T*> { typedef const T * type; };

template< typename Tcontainer >
class ConstIndirectIterator;

template< typename Tcontainer >
class IndirectIterator
{
private:
    typename Tcontainer::iterator i;

public:
    IndirectIterator( const IndirectIterator & rhs ) : i( rhs.i ) {}
    IndirectIterator( const typename Tcontainer::iterator & r_i_in ) : i( r_i_in ) {}
    typename dereference<typename Tcontainer::value_type>::type & operator * () const { return **i; }
    typename Tcontainer::value_type operator -> () const { return &**this; }
    IndirectIterator & operator ++ () { ++i; return *this; }
    IndirectIterator & operator ++ (int) { i++; return *this; }
    bool operator != ( const IndirectIterator & r_rhs ) { return i != r_rhs.i; }
    bool operator == ( const IndirectIterator & r_rhs ) { return ! operator != (r_rhs); }

    friend class ConstIndirectIterator<Tcontainer>;
};

template< typename Tcontainer >
class ConstIndirectIterator
{
private:
    typename Tcontainer::const_iterator i;

public:
    ConstIndirectIterator( const typename Tcontainer::const_iterator & r_i_in ) : i( r_i_in ) {}
    ConstIndirectIterator( const IndirectIterator< Tcontainer > & r_i_in ) : i( r_i_in.i ) {}
    const typename dereference<typename Tcontainer::value_type>::type & operator * () const { return **i; }
    typename mkconst<typename Tcontainer::value_type>::type operator -> () const { return &**this; }
    ConstIndirectIterator & operator ++ () { ++i; return *this; }
    ConstIndirectIterator & operator ++ (int) { i++; return *this; }
    bool operator != ( const ConstIndirectIterator & r_rhs ) { return i != r_rhs.i; }
    bool operator == ( const ConstIndirectIterator & r_rhs ) { return ! operator != (r_rhs); }
};

template< typename T >
class ptr_vector
{
public:
    typedef T value_type;
    typedef std::vector< T * > container_t;
    typedef IndirectIterator< container_t > iterator;
    typedef ConstIndirectIterator< container_t > const_iterator;

private:
    container_t container;

    template<typename U>
    struct auto_or_uniq_ptr
    {
    #if __cplusplus < 201103L
        typedef std::auto_ptr<U> type;
    #else
        typedef std::unique_ptr<U> type;
    #endif
    };
    typedef typename auto_or_uniq_ptr<T>::type uniq_ptr;

public:
    ptr_vector() {}
    ptr_vector( const ptr_vector & rhs )
    {
        // Deep copy
        for( size_t i=0; i<rhs.size(); ++i )
            push_back( rhs[i] );
    }
    ptr_vector & operator = ( const ptr_vector & rhs )
    {
        // Deep copy
        ptr_vector( rhs ).swap( *this );
        return *this;
    }
    ~ptr_vector()
    {
        for( size_t i=0; i<size(); ++i )
            delete container[i];
    }

    void swap( ptr_vector & rhs ) { container.swap( rhs.container ); }

    size_t size() const { return container.size(); }
    bool empty() const { return container.empty(); }

    void push_back( T * p_in )
    {
        uniq_ptr pu_to_append( p_in );
        container.push_back( pu_to_append.get() );
        pu_to_append.release();
    }
    void push_back( const T & r_in )
    {
        push_back( new T( r_in ) );
    }
    const_iterator begin() const { return const_iterator( container.begin() ); }
    iterator begin() { return iterator( container.begin() ); }
    const_iterator end() const { return const_iterator( container.end() ); }
    iterator end() { return iterator( container.end() ); }

    const T & at( size_t i ) const { return *container.at(i); }
    T & at( size_t i ) { return *container.at(i); }
    const T & operator [] ( size_t i ) const { return *(container[i]); }
    T & operator [] ( size_t i ) { return *(container[i]); }

    const T & back() const { return *container.back(); }
    T & back() { return *container.back(); }
    const T & front() const { return *container.front(); }
    T & front() { return *container.front(); }
};

}   // namespace clutils

#endif  // PTR_VECTOR
