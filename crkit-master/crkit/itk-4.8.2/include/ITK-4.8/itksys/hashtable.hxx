/*============================================================================
  KWSys - Kitware System Library
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
/*
 * Copyright (c) 1996
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#ifdef __BORLANDC__
# pragma warn -8027 /* 'for' not inlined.  */
# pragma warn -8026 /* 'exception' not inlined.  */
#endif 

#ifndef itksys_hashtable_hxx
#define itksys_hashtable_hxx

#include <itksys/Configure.hxx>

#include <itksys/cstddef>        // size_t
#include <itksys/stl/algorithm>  // lower_bound
#include <itksys/stl/functional> // unary_function
#include <itksys/stl/iterator>   // iterator_traits
#include <itksys/stl/memory>     // allocator
#include <itksys/stl/utility>    // pair
#include <itksys/stl/vector>     // vector

#if defined(_MSC_VER)
# pragma warning (push)
# pragma warning (disable:4284)
# pragma warning (disable:4786)
# pragma warning (disable:4512) /* no assignment operator for class */
#endif
#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 3970 /* pointer to int conversion */ 3321 3968
#endif

// In C++11, clang will warn about using dynamic exception specifications
// as they are deprecated.  But as this class is trying to faithfully
// mimic unordered_set and unordered_map, we want to keep the 'throw()'
// decorations below.  So we suppress the warning.
#if defined(__clang__) && defined(__has_warning)
# if __has_warning("-Wdeprecated")
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated"
# endif
#endif

#if itksys_STL_HAS_ALLOCATOR_TEMPLATE
# define itksys_HASH_DEFAULT_ALLOCATOR(T) itksys_stl::allocator< T >
#elif itksys_STL_HAS_ALLOCATOR_NONTEMPLATE
# define itksys_HASH_DEFAULT_ALLOCATOR(T) itksys_stl::allocator
#else
# define itksys_HASH_DEFAULT_ALLOCATOR(T) itksys_stl::alloc
#endif

#if itksys_STL_HAS_ALLOCATOR_OBJECTS
# define itksys_HASH_BUCKETS_INIT(__a) _M_buckets(__a)
# define itksys_HASH_BUCKETS_GET_ALLOCATOR(__b) , __b.get_allocator()
#else
# define itksys_HASH_BUCKETS_INIT(__a) _M_buckets()
# define itksys_HASH_BUCKETS_GET_ALLOCATOR(__b)
#endif

namespace itksys
{

//----------------------------------------------------------------------------
// Define an allocator adaptor for platforms that do not provide an
// allocator with the rebind member.
#if !itksys_STL_HAS_ALLOCATOR_REBIND

// Utility functions to convert item counts.
inline size_t hash_sizeof(void*) { return sizeof(char); }
inline size_t hash_sizeof(const void*) { return sizeof(char); }
template <class TPtr> inline size_t hash_sizeof(TPtr p)
{
  static_cast<void>(p);
  return sizeof(*p);
}
template <class POut, class PIn, class TSize>
inline TSize hash_allocator_n(POut out, PIn in, TSize n)
{
  return n*(hash_sizeof(out)/hash_sizeof(in) +
            (hash_sizeof(out)%hash_sizeof(in)>0));
}

// Define an allocation method to use the native allocator with
// the proper signature.  The following signatures of the allocate
// method are used on various STL implementations:
//   pointer allocate(size_type, const void* hint)
//   pointer allocate(size_type)
//   static pointer allocate(size_type, const void* hint)
//   static pointer allocate(size_type)
// Where pointer might be a real type or void*.
// This set of overloads decodes the signature for a particular STL.
// The extra three int/long arguments will favor certain signatures
// over others in the case that multiple are present to avoid
// ambiguity errors.
template <class TAlloc, class PIn, class TSize, class THint, class POut>
inline void hash_allocate(TAlloc* a, PIn (TAlloc::*allocate)(TSize, THint),
                          TSize n_out, const void* hint, POut& out,
                          int, int, int)
{
  TSize n_in = hash_allocator_n(POut(), PIn(), n_out);
  void* vout = (a->*allocate)(n_in, const_cast<THint>(hint));
  out = static_cast<POut>(vout);
}

template <class TAlloc, class PIn, class TSize, class POut>
inline void hash_allocate(TAlloc* a, PIn (TAlloc::*allocate)(TSize),
                          TSize n_out, const void*, POut& out,
                          int, int, long)
{
  TSize n_in = hash_allocator_n(POut(), PIn(), n_out);
  void* vout = (a->*allocate)(n_in);
  out = static_cast<POut>(vout);
}

template <class PIn, class TSize, class THint, class POut>
inline void hash_allocate(void*, PIn (*allocate)(TSize, THint),
                          TSize n_out, const void* hint, POut& out,
                          int, long, long)
{
  TSize n_in = hash_allocator_n(POut(), PIn(), n_out);
  void* vout = allocate(n_in, const_cast<THint>(hint));
  out = static_cast<POut>(vout);
}

template <class PIn, class TSize, class POut>
inline void hash_allocate(void*, PIn (*allocate)(TSize),
                          TSize n_out, const void*, POut& out,
                          long, long, long)
{
  TSize n_in = hash_allocator_n(POut(), PIn(), n_out);
  void* vout = allocate(n_in);
  out = static_cast<POut>(vout);
}

// Define a deallocation method to use the native allocator with
// the proper signature.  The following signatures of the deallocate
// method are used on various STL implementations:
//   void deallocate(pointer, size_type)
//   void deallocate(pointer)
//   static void deallocate(pointer, size_type)
//   static void deallocate(pointer)
// Where pointer might be a real type or void*.
// This set of overloads decodes the signature for a particular STL.
// The extra three int/long arguments will favor certain signatures
// over others in the case that multiple are present to avoid
// ambiguity errors.
template <class TAlloc, class PIn, class TSize, class PInReal, class POut>
inline void hash_deallocate(TAlloc* a, void (TAlloc::*deallocate)(PIn, TSize),
                            PInReal, POut p, TSize n_out, int, int, int)
{
  TSize n_in = hash_allocator_n(POut(), PInReal(), n_out);
  void* vout = p;
  (a->*deallocate)(static_cast<PIn>(vout), n_in);
}

template <class TAlloc, class PIn, class TSize, class PInReal, class POut>
inline void hash_deallocate(TAlloc* a, void (TAlloc::*deallocate)(PIn),
                            PInReal, POut p, TSize, int, int, long)
{
  void* vout = p;
  (a->*deallocate)(static_cast<PIn>(vout));
}

template <class PIn, class TSize, class PInReal, class POut>
inline void hash_deallocate(void*, void (*deallocate)(PIn, TSize),
                            PInReal, POut p, TSize n_out, int, long, long)
{
  TSize n_in = hash_allocator_n(POut(), PInReal(), n_out);
  void* vout = p;
  deallocate(static_cast<PIn>(vout), n_in);
}

template <class PIn, class TSize, class PInReal, class POut>
inline void hash_deallocate(void*, void (*deallocate)(PIn),
                            PInReal, POut p, TSize, long, long, long)
{
  void* vout = p;
  deallocate(static_cast<PIn>(vout));
}

// Use the same four overloads as hash_allocate to decode the type
// really used for allocation.  This is passed as PInReal to the
// deallocate functions so that hash_allocator_n has the proper size.
template <class TAlloc, class PIn, class TSize, class THint>
inline PIn hash_allocate_type(PIn (TAlloc::*)(TSize, THint),
                              int, int, int) { return 0; }
template <class TAlloc, class PIn, class TSize>
inline PIn hash_allocate_type(PIn (TAlloc::*)(TSize),
                              int, int, long) { return 0; }
template <class PIn, class TSize, class THint>
inline PIn hash_allocate_type(PIn (*)(TSize, THint),
                              int, long, long) { return 0; }
template <class PIn, class TSize>
inline PIn hash_allocate_type(PIn (*)(TSize),
                              long, long, long) { return 0; }

// Define the comparison operators in terms of a base type to avoid
// needing templated versions.
class hash_allocator_base {};
inline bool operator==(const hash_allocator_base&,
                const hash_allocator_base&) throw() { return true; }
inline bool operator!=(const hash_allocator_base&,
                const hash_allocator_base&) throw() { return false; }

// Define the allocator template.
template <class T, class Alloc>
class hash_allocator: public hash_allocator_base
{
private:
  // Store the real allocator privately.
  typedef Alloc alloc_type;
  alloc_type alloc_;

public:
  // Standard allocator interface.
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;

  hash_allocator() throw(): alloc_() {}
  hash_allocator(const hash_allocator_base&) throw() : alloc_() {}
  hash_allocator(const hash_allocator& a) throw() : alloc_(a.alloc_) {}
  hash_allocator(const alloc_type& a) throw() : alloc_(a) {}
  ~hash_allocator() throw() {}
# if itksys_CXX_HAS_MEMBER_TEMPLATES
  template <class U>
  struct rebind { typedef hash_allocator<U, alloc_type> other; };
# endif
  pointer address(reference x) const { return &x; }
  const_pointer address(const_reference x) const { return &x; }
  typedef void* void_pointer;
  typedef const void* const_void_pointer;
  pointer allocate(size_type n=1, const_void_pointer hint = 0)
    {
    if(n)
      {
      pointer p;
      hash_allocate(&alloc_, &alloc_type::allocate, n, hint, p, 1, 1, 1);
      return p;
      }
    else
      {
      return 0;
      }
    }
  void deallocate(pointer p, size_type n=1)
    {
    if(n)
      {
      hash_deallocate(&alloc_, &alloc_type::deallocate,
                      hash_allocate_type(&alloc_type::allocate, 1, 1, 1),
                      p, n, 1, 1, 1);
      }
    }
#if itksys_STL_HAS_ALLOCATOR_MAX_SIZE_ARGUMENT
  size_type max_size(size_type s) const throw()
    {
    return alloc_.max_size(s);
    }
#else
  size_type max_size() const throw()
    {
    size_type n = alloc_.max_size() / sizeof(value_type);
    return n>0? n:1;
    }
#endif
  void construct(pointer p, const value_type& val) { new (p) value_type(val); }
  void destroy(pointer p) { (void)p; p->~value_type(); }
};
#endif

template <class _Val>
struct _Hashtable_node
{
  _Hashtable_node* _M_next;
  _Val _M_val;
  void public_method_to_quiet_warning_about_all_methods_private();
private:
  void operator=(_Hashtable_node<_Val> const&); // poison node assignment
};

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey,
          class _Alloc = itksys_HASH_DEFAULT_ALLOCATOR(char) >
class hashtable;

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_iterator;

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_const_iterator;

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_iterator {
  typedef hashtable<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>
          _Hashtable;
  typedef _Hashtable_iterator<_Val, _Key, _HashFcn,
                              _ExtractKey, _EqualKey, _Alloc>
          iterator;
  typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn,
                                    _ExtractKey, _EqualKey, _Alloc>
          const_iterator;
  typedef _Hashtable_node<_Val> _Node;

  typedef itksys_stl::forward_iterator_tag iterator_category;
  typedef _Val value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef _Val& reference;
  typedef _Val* pointer;

  _Node* _M_cur;
  _Hashtable* _M_ht;

  _Hashtable_iterator(_Node* __n, _Hashtable* __tab)
    : _M_cur(__n), _M_ht(__tab) {}
  _Hashtable_iterator() {}
  reference operator*() const { return _M_cur->_M_val; }
  pointer operator->() const { return &(operator*()); }
  iterator& operator++();
  iterator operator++(int);
  bool operator==(const iterator& __it) const
    { return _M_cur == __it._M_cur; }
  bool operator!=(const iterator& __it) const
    { return _M_cur != __it._M_cur; }
};


template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
struct _Hashtable_const_iterator {
  typedef hashtable<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>
          _Hashtable;
  typedef _Hashtable_iterator<_Val,_Key,_HashFcn,
                              _ExtractKey,_EqualKey,_Alloc>
          iterator;
  typedef _Hashtable_const_iterator<_Val, _Key, _HashFcn,
                                    _ExtractKey, _EqualKey, _Alloc>
          const_iterator;
  typedef _Hashtable_node<_Val> _Node;

  typedef itksys_stl::forward_iterator_tag iterator_category;
  typedef _Val value_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef const _Val& reference;
  typedef const _Val* pointer;

  const _Node* _M_cur;
  const _Hashtable* _M_ht;

  _Hashtable_const_iterator(const _Node* __n, const _Hashtable* __tab)
    : _M_cur(__n), _M_ht(__tab) {}
  _Hashtable_const_iterator() {}
  _Hashtable_const_iterator(const iterator& __it)
    : _M_cur(__it._M_cur), _M_ht(__it._M_ht) {}
  reference operator*() const { return _M_cur->_M_val; }
  pointer operator->() const { return &(operator*()); }
  const_iterator& operator++();
  const_iterator operator++(int);
  bool operator==(const const_iterator& __it) const
    { return _M_cur == __it._M_cur; }
  bool operator!=(const const_iterator& __it) const
    { return _M_cur != __it._M_cur; }
};

// Note: assumes long is at least 32 bits.
enum { _stl_num_primes = 31 };

// create a function with a static local to that function that returns
// the static
static inline const unsigned long* get_stl_prime_list() {

static const unsigned long _stl_prime_list[_stl_num_primes] =
{
  5ul,          11ul,         23ul,
  53ul,         97ul,         193ul,       389ul,       769ul,
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
  1610612741ul, 3221225473ul, 4294967291ul
};

return &_stl_prime_list[0]; }

static inline size_t _stl_next_prime(size_t __n)
{
  const unsigned long* __first = get_stl_prime_list();
  const unsigned long* __last = get_stl_prime_list() + (int)_stl_num_primes;
  const unsigned long* pos = itksys_stl::lower_bound(__first, __last, __n);
  return pos == __last ? *(__last - 1) : *pos;
}

// Forward declaration of operator==.

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
class hashtable;

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
bool operator==(const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht1,
                const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht2);

// Hashtables handle allocators a bit differently than other containers
//  do.  If we're using standard-conforming allocators, then a hashtable
//  unconditionally has a member variable to hold its allocator, even if
//  it so happens that all instances of the allocator type are identical.
// This is because, for hashtables, this extra storage is negligible.
//  Additionally, a base class wouldn't serve any other purposes; it
//  wouldn't, for example, simplify the exception-handling code.

template <class _Val, class _Key, class _HashFcn,
          class _ExtractKey, class _EqualKey, class _Alloc>
class hashtable {
public:
  typedef _Key key_type;
  typedef _Val value_type;
  typedef _HashFcn hasher;
  typedef _EqualKey key_equal;

  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;

  hasher hash_funct() const { return _M_hash; }
  key_equal key_eq() const { return _M_equals; }

private:
  typedef _Hashtable_node<_Val> _Node;

#if itksys_STL_HAS_ALLOCATOR_REBIND
public:
  typedef typename _Alloc::template rebind<_Val>::other allocator_type;
  allocator_type get_allocator() const { return _M_node_allocator; }
private:
  typedef typename _Alloc::template rebind<_Node>::other _M_node_allocator_type;
  typedef typename _Alloc::template rebind<_Node*>::other _M_node_ptr_allocator_type;
  typedef itksys_stl::vector<_Node*,_M_node_ptr_allocator_type> _M_buckets_type;
#else
public:
  typedef hash_allocator<_Val, _Alloc> allocator_type;
  allocator_type get_allocator() const { return allocator_type(); }
private:
  typedef hash_allocator<_Node, _Alloc> _M_node_allocator_type;
# if itksys_STL_HAS_ALLOCATOR_OBJECTS
  typedef hash_allocator<_Node*, _Alloc> _M_node_ptr_allocator_type;
# else
  typedef _Alloc _M_node_ptr_allocator_type;
# endif
  typedef itksys_stl::vector<_Node*,_M_node_ptr_allocator_type> _M_buckets_type;
#endif

private:
  _M_node_allocator_type _M_node_allocator;
  hasher                 _M_hash;
  key_equal              _M_equals;
  _ExtractKey            _M_get_key;
  _M_buckets_type        _M_buckets;
  size_type              _M_num_elements;

  _Node* _M_get_node() { return _M_node_allocator.allocate(1); }
  void _M_put_node(_Node* __p) { _M_node_allocator.deallocate(__p, 1); }

public:
  typedef _Hashtable_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>
          iterator;
  typedef _Hashtable_const_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,
                                    _Alloc>
          const_iterator;

  friend struct
  _Hashtable_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>;
  friend struct
  _Hashtable_const_iterator<_Val,_Key,_HashFcn,_ExtractKey,_EqualKey,_Alloc>;

public:
  hashtable(size_type __n,
            const _HashFcn&    __hf,
            const _EqualKey&   __eql,
            const _ExtractKey& __ext,
            const allocator_type& __a = allocator_type())
    : _M_node_allocator(__a),
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(__ext),
      itksys_HASH_BUCKETS_INIT(__a),
      _M_num_elements(0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(size_type __n,
            const _HashFcn&    __hf,
            const _EqualKey&   __eql,
            const allocator_type& __a = allocator_type())
    : _M_node_allocator(__a),
      _M_hash(__hf),
      _M_equals(__eql),
      _M_get_key(_ExtractKey()),
      itksys_HASH_BUCKETS_INIT(__a),
      _M_num_elements(0)
  {
    _M_initialize_buckets(__n);
  }

  hashtable(const hashtable& __ht)
    : _M_node_allocator(__ht.get_allocator()),
      _M_hash(__ht._M_hash),
      _M_equals(__ht._M_equals),
      _M_get_key(__ht._M_get_key),
      itksys_HASH_BUCKETS_INIT(__ht.get_allocator()),
      _M_num_elements(0)
  {
    _M_copy_from(__ht);
  }

  hashtable& operator= (const hashtable& __ht)
  {
    if (&__ht != this) {
      clear();
      _M_hash = __ht._M_hash;
      _M_equals = __ht._M_equals;
      _M_get_key = __ht._M_get_key;
      _M_copy_from(__ht);
    }
    return *this;
  }

  ~hashtable() { clear(); }

  size_type size() const { return _M_num_elements; }
  size_type max_size() const { return size_type(-1); }
  bool empty() const { return size() == 0; }

  void swap(hashtable& __ht)
  {
    itksys_stl::swap(_M_hash, __ht._M_hash);
    itksys_stl::swap(_M_equals, __ht._M_equals);
    itksys_stl::swap(_M_get_key, __ht._M_get_key);
    _M_buckets.swap(__ht._M_buckets);
    itksys_stl::swap(_M_num_elements, __ht._M_num_elements);
  }

  iterator begin()
  {
    for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
      if (_M_buckets[__n])
        return iterator(_M_buckets[__n], this);
    return end();
  }

  iterator end() { return iterator(0, this); }

  const_iterator begin() const
  {
    for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
      if (_M_buckets[__n])
        return const_iterator(_M_buckets[__n], this);
    return end();
  }

  const_iterator end() const { return const_iterator(0, this); }

  friend bool operator==itksys_CXX_NULL_TEMPLATE_ARGS(const hashtable&,
                                                                 const hashtable&);

public:

  size_type bucket_count() const { return _M_buckets.size(); }

  size_type max_bucket_count() const
    { return get_stl_prime_list()[(int)_stl_num_primes - 1]; }

  size_type elems_in_bucket(size_type __bucket) const
  {
    size_type __result = 0;
    for (_Node* __cur = _M_buckets[__bucket]; __cur; __cur = __cur->_M_next)
      __result += 1;
    return __result;
  }

  itksys_stl::pair<iterator, bool> insert_unique(const value_type& __obj)
  {
    resize(_M_num_elements + 1);
    return insert_unique_noresize(__obj);
  }

  iterator insert_equal(const value_type& __obj)
  {
    resize(_M_num_elements + 1);
    return insert_equal_noresize(__obj);
  }

  itksys_stl::pair<iterator, bool> insert_unique_noresize(const value_type& __obj);
  iterator insert_equal_noresize(const value_type& __obj);

#if itksys_STL_HAS_ITERATOR_TRAITS
# define itksys_HASH_ITERATOR_CATEGORY(T,I) \
  typename itksys_stl::iterator_traits< T >::iterator_category()
#elif itksys_STL_HAS_ITERATOR_CATEGORY
# define itksys_HASH_ITERATOR_CATEGORY(T,I) \
  itksys_stl::iterator_category( I )
#elif itksys_STL_HAS___ITERATOR_CATEGORY
# define itksys_HASH_ITERATOR_CATEGORY(T,I) \
  itksys_stl::__iterator_category( I )
#endif

#if itksys_CXX_HAS_MEMBER_TEMPLATES && defined(itksys_HASH_ITERATOR_CATEGORY)
  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l)
  {
    insert_unique(__f, __l,
      itksys_HASH_ITERATOR_CATEGORY(_InputIterator, __f));
  }

  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l)
  {
    insert_equal(__f, __l,
      itksys_HASH_ITERATOR_CATEGORY(_InputIterator, __f));
  }

  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l,
                     itksys_stl::input_iterator_tag)
  {
    for ( ; __f != __l; ++__f)
      insert_unique(*__f);
  }

  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l,
                    itksys_stl::input_iterator_tag)
  {
    for ( ; __f != __l; ++__f)
      insert_equal(*__f);
  }

  template <class _ForwardIterator>
  void insert_unique(_ForwardIterator __f, _ForwardIterator __l,
                     itksys_stl::forward_iterator_tag)
  {
    size_type __n = 0;
    itksys_stl::distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }

  template <class _ForwardIterator>
  void insert_equal(_ForwardIterator __f, _ForwardIterator __l,
                    itksys_stl::forward_iterator_tag)
  {
    size_type __n = 0;
    itksys_stl::distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }

#else
  void insert_unique(const value_type* __f, const value_type* __l)
  {
    size_type __n = __l - __f;
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }

  void insert_equal(const value_type* __f, const value_type* __l)
  {
    size_type __n = __l - __f;
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }

  void insert_unique(const_iterator __f, const_iterator __l)
  {
    size_type __n = 0;
    itksys_stl::distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_unique_noresize(*__f);
  }

  void insert_equal(const_iterator __f, const_iterator __l)
  {
    size_type __n = 0;
    itksys_stl::distance(__f, __l, __n);
    resize(_M_num_elements + __n);
    for ( ; __n > 0; --__n, ++__f)
      insert_equal_noresize(*__f);
  }
#endif

  reference find_or_insert(const value_type& __obj);

  iterator find(const key_type& __key)
  {
    size_type __n = _M_bkt_num_key(__key);
    _Node* __first;
    for ( __first = _M_buckets[__n];
          __first && !_M_equals(_M_get_key(__first->_M_val), __key);
          __first = __first->_M_next)
      {}
    return iterator(__first, this);
  }

  const_iterator find(const key_type& __key) const
  {
    size_type __n = _M_bkt_num_key(__key);
    const _Node* __first;
    for ( __first = _M_buckets[__n];
          __first && !_M_equals(_M_get_key(__first->_M_val), __key);
          __first = __first->_M_next)
      {}
    return const_iterator(__first, this);
  }

  size_type count(const key_type& __key) const
  {
    const size_type __n = _M_bkt_num_key(__key);
    size_type __result = 0;

    for (const _Node* __cur = _M_buckets[__n]; __cur; __cur = __cur->_M_next)
      if (_M_equals(_M_get_key(__cur->_M_val), __key))
        ++__result;
    return __result;
  }

  itksys_stl::pair<iterator, iterator>
  equal_range(const key_type& __key);

  itksys_stl::pair<const_iterator, const_iterator>
  equal_range(const key_type& __key) const;

  size_type erase(const key_type& __key);
  void erase(const iterator& __it);
  void erase(iterator __first, iterator __last);

  void erase(const const_iterator& __it);
  void erase(const_iterator __first, const_iterator __last);

  void resize(size_type __num_elements_hint);
  void clear();

private:
  size_type _M_next_size(size_type __n) const
    { return _stl_next_prime(__n); }

  void _M_initialize_buckets(size_type __n)
  {
    const size_type __n_buckets = _M_next_size(__n);
    _M_buckets.reserve(__n_buckets);
    _M_buckets.insert(_M_buckets.end(), __n_buckets, (_Node*) 0);
    _M_num_elements = 0;
  }

  size_type _M_bkt_num_key(const key_type& __key) const
  {
    return _M_bkt_num_key(__key, _M_buckets.size());
  }

  size_type _M_bkt_num(const value_type& __obj) const
  {
    return _M_bkt_num_key(_M_get_key(__obj));
  }

  size_type _M_bkt_num_key(const key_type& __key, size_t __n) const
  {
    return _M_hash(__key) % __n;
  }

  size_type _M_bkt_num(const value_type& __obj, size_t __n) const
  {
    return _M_bkt_num_key(_M_get_key(__obj), __n);
  }

  void construct(_Val* p, const _Val& v)
    {
    new (p) _Val(v);
    }
  void destroy(_Val* p)
    {
    (void)p;
    p->~_Val();
    }

  _Node* _M_new_node(const value_type& __obj)
  {
    _Node* __n = _M_get_node();
    __n->_M_next = 0;
    try {
      construct(&__n->_M_val, __obj);
      return __n;
    }
    catch(...) {_M_put_node(__n); throw;}
  }

  void _M_delete_node(_Node* __n)
  {
    destroy(&__n->_M_val);
    _M_put_node(__n);
  }

  void _M_erase_bucket(const size_type __n, _Node* __first, _Node* __last);
  void _M_erase_bucket(const size_type __n, _Node* __last);

  void _M_copy_from(const hashtable& __ht);

};

template <class _Val, class _Key, class _HF, class _ExK, class _EqK,
          class _All>
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++()
{
  const _Node* __old = _M_cur;
  _M_cur = _M_cur->_M_next;
  if (!_M_cur) {
    size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
    while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
      _M_cur = _M_ht->_M_buckets[__bucket];
  }
  return *this;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK,
          class _All>
inline _Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++(int)
{
  iterator __tmp = *this;
  ++*this;
  return __tmp;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK,
          class _All>
_Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>&
_Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++()
{
  const _Node* __old = _M_cur;
  _M_cur = _M_cur->_M_next;
  if (!_M_cur) {
    size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
    while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
      _M_cur = _M_ht->_M_buckets[__bucket];
  }
  return *this;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK,
          class _All>
inline _Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>
_Hashtable_const_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::operator++(int)
{
  const_iterator __tmp = *this;
  ++*this;
  return __tmp;
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
bool operator==(const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht1,
                const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht2)
{
  typedef typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::_Node _Node;
  if (__ht1._M_buckets.size() != __ht2._M_buckets.size())
    return false;
  for (int __n = 0; __n < __ht1._M_buckets.size(); ++__n) {
    _Node* __cur1 = __ht1._M_buckets[__n];
    _Node* __cur2 = __ht2._M_buckets[__n];
    for ( ; __cur1 && __cur2 && __cur1->_M_val == __cur2->_M_val;
          __cur1 = __cur1->_M_next, __cur2 = __cur2->_M_next)
      {}
    if (__cur1 || __cur2)
      return false;
  }
  return true;
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
inline bool operator!=(const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht1,
                       const hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>& __ht2) {
  return !(__ht1 == __ht2);
}

template <class _Val, class _Key, class _HF, class _Extract, class _EqKey,
          class _All>
inline void swap(hashtable<_Val, _Key, _HF, _Extract, _EqKey, _All>& __ht1,
                 hashtable<_Val, _Key, _HF, _Extract, _EqKey, _All>& __ht2) {
  __ht1.swap(__ht2);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
itksys_stl::pair<itksys_CXX_DECL_TYPENAME hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator, bool>
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_unique_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
      return itksys_stl::pair<iterator, bool>(iterator(__cur, this), false);

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return itksys_stl::pair<iterator, bool>(iterator(__tmp, this), true);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::insert_equal_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
      _Node* __tmp = _M_new_node(__obj);
      __tmp->_M_next = __cur->_M_next;
      __cur->_M_next = __tmp;
      ++_M_num_elements;
      return iterator(__tmp, this);
    }

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return iterator(__tmp, this);
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::reference
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::find_or_insert(const value_type& __obj)
{
  resize(_M_num_elements + 1);

  size_type __n = _M_bkt_num(__obj);
  _Node* __first = _M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
      return __cur->_M_val;

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements;
  return __tmp->_M_val;
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
itksys_stl::pair<itksys_CXX_DECL_TYPENAME hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator,
     itksys_CXX_DECL_TYPENAME hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::iterator>
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::equal_range(const key_type& __key)
{
  typedef itksys_stl::pair<iterator, iterator> _Pii;
  const size_type __n = _M_bkt_num_key(__key);

  for (_Node* __first = _M_buckets[__n]; __first; __first = __first->_M_next)
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (_Node* __cur = __first->_M_next; __cur; __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(iterator(__first, this), iterator(__cur, this));
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(iterator(__first, this),
                     iterator(_M_buckets[__m], this));
      return _Pii(iterator(__first, this), end());
    }
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
itksys_stl::pair<itksys_CXX_DECL_TYPENAME hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::const_iterator,
     itksys_CXX_DECL_TYPENAME hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::const_iterator>
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::equal_range(const key_type& __key) const
{
  typedef itksys_stl::pair<const_iterator, const_iterator> _Pii;
  const size_type __n = _M_bkt_num_key(__key);

  for (const _Node* __first = _M_buckets[__n] ;
       __first;
       __first = __first->_M_next) {
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (const _Node* __cur = __first->_M_next;
           __cur;
           __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(const_iterator(__first, this),
                      const_iterator(__cur, this));
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(const_iterator(__first, this),
                      const_iterator(_M_buckets[__m], this));
      return _Pii(const_iterator(__first, this), end());
    }
  }
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
typename hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::size_type
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const key_type& __key)
{
  const size_type __n = _M_bkt_num_key(__key);
  _Node* __first = _M_buckets[__n];
  size_type __erased = 0;

  if (__first) {
    _Node* __cur = __first;
    _Node* __next = __cur->_M_next;
    while (__next) {
      if (_M_equals(_M_get_key(__next->_M_val), __key)) {
        __cur->_M_next = __next->_M_next;
        _M_delete_node(__next);
        __next = __cur->_M_next;
        ++__erased;
        --_M_num_elements;
      }
      else {
        __cur = __next;
        __next = __cur->_M_next;
      }
    }
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      _M_buckets[__n] = __first->_M_next;
      _M_delete_node(__first);
      ++__erased;
      --_M_num_elements;
    }
  }
  return __erased;
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const iterator& __it)
{
  _Node* __p = __it._M_cur;
  if (__p) {
    const size_type __n = _M_bkt_num(__p->_M_val);
    _Node* __cur = _M_buckets[__n];

    if (__cur == __p) {
      _M_buckets[__n] = __cur->_M_next;
      _M_delete_node(__cur);
      --_M_num_elements;
    }
    else {
      _Node* __next = __cur->_M_next;
      while (__next) {
        if (__next == __p) {
          __cur->_M_next = __next->_M_next;
          _M_delete_node(__next);
          --_M_num_elements;
          break;
        }
        else {
          __cur = __next;
          __next = __cur->_M_next;
        }
      }
    }
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::erase(iterator __first, iterator __last)
{
  size_type __f_bucket = __first._M_cur ?
    _M_bkt_num(__first._M_cur->_M_val) : _M_buckets.size();
  size_type __l_bucket = __last._M_cur ?
    _M_bkt_num(__last._M_cur->_M_val) : _M_buckets.size();

  if (__first._M_cur == __last._M_cur)
    return;
  else if (__f_bucket == __l_bucket)
    _M_erase_bucket(__f_bucket, __first._M_cur, __last._M_cur);
  else {
    _M_erase_bucket(__f_bucket, __first._M_cur, 0);
    for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n)
      _M_erase_bucket(__n, 0);
    if (__l_bucket != _M_buckets.size())
      _M_erase_bucket(__l_bucket, __last._M_cur);
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
inline void
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const_iterator __first,
                                             const_iterator __last)
{
  erase(iterator(const_cast<_Node*>(__first._M_cur),
                 const_cast<hashtable*>(__first._M_ht)),
        iterator(const_cast<_Node*>(__last._M_cur),
                 const_cast<hashtable*>(__last._M_ht)));
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
inline void
hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::erase(const const_iterator& __it)
{
  erase(iterator(const_cast<_Node*>(__it._M_cur),
                 const_cast<hashtable*>(__it._M_ht)));
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::resize(size_type __num_elements_hint)
{
  const size_type __old_n = _M_buckets.size();
  if (__num_elements_hint > __old_n) {
    const size_type __n = _M_next_size(__num_elements_hint);
    if (__n > __old_n) {
      _M_buckets_type __tmp(
        __n, (_Node*)(0)
        itksys_HASH_BUCKETS_GET_ALLOCATOR(_M_buckets));
      try {
        for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
          _Node* __first = _M_buckets[__bucket];
          while (__first) {
            size_type __new_bucket = _M_bkt_num(__first->_M_val, __n);
            _M_buckets[__bucket] = __first->_M_next;
            __first->_M_next = __tmp[__new_bucket];
            __tmp[__new_bucket] = __first;
            __first = _M_buckets[__bucket];
          }
        }
        _M_buckets.swap(__tmp);
      }
      catch(...) {
        for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
          while (__tmp[__bucket]) {
            _Node* __next = __tmp[__bucket]->_M_next;
            _M_delete_node(__tmp[__bucket]);
            __tmp[__bucket] = __next;
          }
        }
        throw;
      }
    }
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __first, _Node* __last)
{
  _Node* __cur = _M_buckets[__n];
  if (__cur == __first)
    _M_erase_bucket(__n, __last);
  else {
    _Node* __next;
    for (__next = __cur->_M_next;
         __next != __first;
         __cur = __next, __next = __cur->_M_next)
      ;
    while (__next != __last) {
      __cur->_M_next = __next->_M_next;
      _M_delete_node(__next);
      __next = __cur->_M_next;
      --_M_num_elements;
    }
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __last)
{
  _Node* __cur = _M_buckets[__n];
  while (__cur != __last) {
    _Node* __next = __cur->_M_next;
    _M_delete_node(__cur);
    __cur = __next;
    _M_buckets[__n] = __cur;
    --_M_num_elements;
  }
}

template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>::clear()
{
  for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
    _Node* __cur = _M_buckets[__i];
    while (__cur != 0) {
      _Node* __next = __cur->_M_next;
      _M_delete_node(__cur);
      __cur = __next;
    }
    _M_buckets[__i] = 0;
  }
  _M_num_elements = 0;
}


template <class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
void hashtable<_Val,_Key,_HF,_Ex,_Eq,_All>
  ::_M_copy_from(const hashtable& __ht)
{
  _M_buckets.clear();
  _M_buckets.reserve(__ht._M_buckets.size());
  _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (_Node*) 0);
  try {
    for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
      const _Node* __cur = __ht._M_buckets[__i];
      if (__cur) {
        _Node* __copy = _M_new_node(__cur->_M_val);
        _M_buckets[__i] = __copy;

        for (_Node* __next = __cur->_M_next;
             __next;
             __cur = __next, __next = __cur->_M_next) {
          __copy->_M_next = _M_new_node(__next->_M_val);
          __copy = __copy->_M_next;
        }
      }
    }
    _M_num_elements = __ht._M_num_elements;
  }
  catch(...) {clear(); throw;}
}

} // namespace itksys

// Normally the comparison operators should be found in the itksys
// namespace by argument dependent lookup.  For compilers that do not
// support it we must bring them into the global namespace now.
#if !itksys_CXX_HAS_ARGUMENT_DEPENDENT_LOOKUP
using itksys::operator==;
using itksys::operator!=;
#endif

// Undo warning suppression.
#if defined(__clang__) && defined(__has_warning)
# if __has_warning("-Wdeprecated")
#  pragma clang diagnostic pop
# endif
#endif

#if defined(_MSC_VER)
# pragma warning (pop)
#endif

#endif