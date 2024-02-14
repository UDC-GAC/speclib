/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     LinkedListPool.h
/// \brief    Provides a simple pool based on a linked list with atomic operations
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef __LINKEDLISTPOOL_H
#define __LINKEDLISTPOOL_H

#include <cstdlib>
#include <vector>
#include <atomic>

/// \brief Provides a common API for heap allocation/deallocation
struct PoolAllocator_malloc_free
{
  typedef std::size_t size_type; //!< Unsigned integral type that can represent the size of the largest object to be allocated.
  typedef std::ptrdiff_t difference_type; //!< Signed integral type that can represent the difference of any two pointers.
  
  /// Allocate nbytes bytes of space in the head
  static char * malloc(const size_type nbytes)
  { return static_cast<char *>(std::malloc(nbytes)); }
  
  /// Deallocate the head space pointed by block
  static void free(char * const block)
  { std::free(block); }
  
};

/// \brief Pool implemented by means of a linked list with atomic operations
/// \tparam T type of the objects of the pool. They must have a field <tt>T * next</tt>
template <typename T>
class LinkedListPool {
  
  typedef std::vector<T *> vector_t;
  
  vector_t v_;            ///< Stores all the chunks allocated by this pool
  const int chunkSize_;   ///< How many holders to allocate each time
  const int minTSize_;    ///< Space allocated for each object
  std::atomic<T *> head_; ///< Current head of the pool
  std::atomic_flag pool_mutex_; ///< mutex for global critical sections in the pool (only in allocate)
  
  /// Allocate a new chunk of chunkSize_ elements for the pool
  void allocate() {
    char * baseptr = PoolAllocator_malloc_free::malloc(static_cast<size_t>(chunkSize_ * minTSize_));
    char * const endptr = baseptr +  minTSize_ * (chunkSize_ - 1);
    T * const h = reinterpret_cast<T *>(baseptr);
    T * const q = reinterpret_cast<T *>(endptr);
    T * p = h;
    do {
      baseptr += minTSize_;
      new (p) T();
      p->next = reinterpret_cast<T *>(baseptr); //invalid for p=q. Will be corrected during linking
      p = static_cast<T *>(p->next);
    } while (baseptr <= endptr);
    
    while (pool_mutex_.test_and_set(std::memory_order_acquire));
    
    v_.push_back(h);

    freeLinkedList(h, q);
    
    pool_mutex_.clear(std::memory_order_release);
  }
  
public:
  
  /// \brief Constructor
  /// \param chunkSize  number of elements to allocate at once in chunk when the pool is empty
  /// \param min_t_size minimum space to allocate for each item.
  LinkedListPool(int chunkSize = 1, int min_t_size = static_cast<int>(sizeof(T))) :
  chunkSize_{(chunkSize < 1) ? 1 : chunkSize},
  minTSize_{(static_cast<int>(sizeof(T)) > min_t_size) ? static_cast<int>(sizeof(T)) : min_t_size},
  head_{nullptr}
#if __cplusplus < 202002L
  // 'ATOMIC_FLAG_INIT' macro is no longer needed and deprecated since C++20, since default constructor of std::atomic_flag initializes it to clear state
  ,pool_mutex_{ATOMIC_FLAG_INIT}
#endif
  {
    allocate();
  }

  /// \brief Destructor
  /// \internal Since every \p free invokes the object's destructor, it is not called here
  ~LinkedListPool()
  {
    typename vector_t::const_iterator const itend = v_.end();
    for(typename vector_t::const_iterator it = v_.begin(); it != itend; ++it)
      PoolAllocator_malloc_free::free(reinterpret_cast<char*>(*it));
  }

  /// Return an item to the pool. Does not invoke destructor
  void shallow_free(T* const datain) noexcept
  {
    datain->next = head_.load(std::memory_order_relaxed);
    while(!head_.compare_exchange_weak(datain->next, datain));
  }

  /// Return an item to the pool. Invokes destructor
  void free(T* const datain) noexcept
  {
    datain->~T();
    shallow_free(datain);
  }
  
  /// Return a linked list of items to the pool when the end is known. Invokes destructor
  void freeLinkedList(T* const datain, T* const last_datain) noexcept
  { T *p;
    
    for (p = datain; p != last_datain; p = p->next) {
      p->~T();
    }
    p->~T();

    last_datain->next = head_.load(std::memory_order_relaxed);
    while(!head_.compare_exchange_weak(last_datain->next, datain));
  }

  /// \brief Get an item from the pool without invoking a constructor
  /// \internal May suffer ABA problem giving place to memory leaks,
  ///           but not in SpecLib because only one thread allocates nodes
  T* malloc()
  { T *ret, *next_val;
    
    ret = head_.load(std::memory_order_relaxed);;
    do {
      while(ret == nullptr) {
        allocate();
        ret = head_.load(std::memory_order_relaxed);;
      }
      next_val = static_cast<T *>(ret->next);
    } while(!head_.compare_exchange_weak(ret, next_val));
    
    //Notice that we do not make a new (ret) T()
    return ret;
  }

  /// Get an item from the pool, initializing it with a constructor
  template<typename... Args>
  T* malloc(Args&&... args)
  {
    T * const ret = this->malloc();
    new (ret) T(std::forward<Args>(args)...);
    return ret;
  }
  
  /// Get an item from the pool, initializing it with a constructor using the default constructor
  T* defaultmalloc()
  {
    T * const ret = this->malloc();
    new (ret) T();
    return ret;
  }

};

#endif // __LINKEDLISTPOOL_H
