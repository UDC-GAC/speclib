/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     SpecVector.h
/// \brief    Generic speculative vector
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef _SPECVECTOR_H_
#define _SPECVECTOR_H_

#ifndef NOALLOCA
#ifdef ALLOCAINMALLOCH
#if __cplusplus >= 201703L
#if __has_include(<malloc.h>)
#include <malloc.h>
#elif __has_include(<alloca.h>)
#include <alloca.h>
#else
#define NOALLOCA
#endif
#else
#include <malloc.h>
#endif
#else
#if __cplusplus >= 201703L
#if __has_include(<alloca.h>)
#include <alloca.h>
#elif __has_include(<malloc.h>)
#include <malloc.h>
#else
#define NOALLOCA
#endif
#else
#include <alloca.h>
#endif
#endif
#endif

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <utility>

namespace SpecLib {

template <typename VALUE_T, typename KEY_T =  std::size_t>
class SpecVector {

  static constexpr int NULL_POS = -1;
  static constexpr int INVALID_POS = -2;

  struct IntlData_t {
    std::atomic<int> next_;
    KEY_T key_;
    VALUE_T value_;

    bool valid() const noexcept { return next_.load(std::memory_order_relaxed) != INVALID_POS; }
  };

  VALUE_T * origin_;
  std::atomic<int> cur_avl_data_;
  std::atomic<int> * hash_table_;
  IntlData_t *data_;
  int alloc_chunk_data_;
  int hash_table_size_;
  const SpecVector * volatile prev_;

  void resize_for(const SpecVector& other)
  {
    if (alloc_chunk_data_ != other.alloc_chunk_data_) {
      if(alloc_chunk_data_) {
        free(data_);
      }
      alloc_chunk_data_ = other.alloc_chunk_data_;
      data_ = alloc_chunk_data_ ? reinterpret_cast<IntlData_t *>(malloc(sizeof(IntlData_t) * alloc_chunk_data_)) : nullptr;
    }

    if (hash_table_size_ != other.hash_table_size_) {
      if (hash_table_size_) {
        free(hash_table_);
      }
      hash_table_size_ = other.hash_table_size_;
      hash_table_ = hash_table_size_ ? reinterpret_cast<std::atomic<int> *>(malloc(sizeof(std::atomic<int> *) * hash_table_size_)) : nullptr;
    }
  }

  static int store_list_helper(const std::atomic<int> *ptr, const IntlData_t * const data, std::pair<KEY_T, VALUE_T> * const dest, const int max_storage)
  { int elems{0};

    for (int pos = ptr->load(std::memory_order_relaxed);
         (pos != NULL_POS);
         pos = ptr->load(std::memory_order_relaxed)) {
      if (elems == max_storage) {
        throw std::runtime_error("SpecVector operator== storage exceeded");
      }
      const IntlData_t& d = data[pos];
      dest[elems++] = { d.key_, d.value_ };
      ptr = &d.next_;
    }

    return elems;
  }

public:

  SpecVector(std::vector<VALUE_T>& source, int alloc_chunk_data, int factor = 1) :
  SpecVector(source.data(), alloc_chunk_data, factor)
  {}

  template<std::size_t N>
  SpecVector(std::array<VALUE_T, N>& source, int alloc_chunk_data, int factor = 1) :
  SpecVector(source.begin(), alloc_chunk_data, factor)
  {}

  SpecVector(VALUE_T *source = nullptr, int alloc_chunk_data = 0, int factor = 1) :
  origin_{source},
  cur_avl_data_{0},
  alloc_chunk_data_{alloc_chunk_data},
  prev_{nullptr}
  {
    if(alloc_chunk_data) {
      data_ = reinterpret_cast<IntlData_t *>(malloc(sizeof(IntlData_t) * alloc_chunk_data));
      const int max_hash_table_size = alloc_chunk_data / factor;
      for(hash_table_size_ = 1; hash_table_size_ < max_hash_table_size; hash_table_size_ *=2);
      hash_table_ = reinterpret_cast<std::atomic<int> *>(malloc(sizeof(std::atomic<int> *) * hash_table_size_));
      std::fill(hash_table_, hash_table_ + hash_table_size_, NULL_POS);
    } else {
      data_ = nullptr;
      hash_table_ = nullptr;
      hash_table_size_ = 0;
    }
  }

  SpecVector(const SpecVector& other) :
  SpecVector(other.origin_)
  {
    const auto used_storage = other.cur_avl_data_.load(std::memory_order_relaxed);

    cur_avl_data_.store(used_storage, std::memory_order_relaxed);
    prev_ = other.prev_;

    resize_for(other);

    std::memcpy(data_, other.data_, sizeof(IntlData_t) * used_storage);
    std::memcpy(hash_table_, other.hash_table_, sizeof(std::atomic<int> *) * hash_table_size_);
//    std::copy(other.data_, other.data_+used_storage, data_);
//    std::copy(other.hash_table_, other.hash_table_+hash_table_size_, hash_table_);
  }

  SpecVector(SpecVector&& other) :
  SpecVector(other.origin_)
  {
    const auto used_storage = other.cur_avl_data_.load(std::memory_order_relaxed);

    cur_avl_data_.store(used_storage, std::memory_order_relaxed);
    prev_ = other.prev_;

    std::swap(alloc_chunk_data_, other.alloc_chunk_data_);
    std::swap(data_, other.data_);
    std::swap(hash_table_size_, other.hash_table_size_);
    std::swap(hash_table_, other.hash_table_);
  }

  SpecVector& operator=(const SpecVector& other)
  {
    const auto used_storage = other.cur_avl_data_.load(std::memory_order_relaxed);

    origin_ = other.origin_;
    cur_avl_data_.store(used_storage, std::memory_order_relaxed);
    prev_ = other.prev_;

    resize_for(other);

    std::memcpy(data_, other.data_, sizeof(IntlData_t) * used_storage);
    std::memcpy(hash_table_, other.hash_table_, sizeof(std::atomic<int> *) * hash_table_size_);
//    std::copy(other.data_, other.data_+used_storage, data_);
//    std::copy(other.hash_table_, other.hash_table_+hash_table_size_, hash_table_);

    return *this;
  }

  SpecVector& operator=(SpecVector&& other)
  {
    const auto other_used_storage = other.cur_avl_data_.load(std::memory_order_relaxed);

    origin_ = other.origin_;
    cur_avl_data_.store(other_used_storage, std::memory_order_relaxed);
    prev_ = other.prev_;

    std::swap(alloc_chunk_data_, other.alloc_chunk_data_);
    std::swap(data_, other.data_);
    std::swap(hash_table_size_, other.hash_table_size_);
    std::swap(hash_table_, other.hash_table_);

    return *this;
  }

  VALUE_T& operator[](KEY_T index)
  { int pos;

    assert(alloc_chunk_data_);

    const int hash_pos = static_cast<int>(std::hash<KEY_T>{}(index) & (hash_table_size_ - 1));
    std::atomic<int> *ptr = hash_table_ + hash_pos;
    //fprintf(stderr, "%d @ %lu\n", ptr->load(), index);
    for (pos = ptr->load(std::memory_order_relaxed);
         pos != NULL_POS;
         pos = ptr->load(std::memory_order_relaxed)) {
      IntlData_t& d = data_[pos];
      if (d.key_ == index) {
        return d.value_;
      }
      ptr = &d.next_;
    }

    const int new_pos = cur_avl_data_.fetch_add(1);
    if (new_pos == alloc_chunk_data_) {
      throw std::runtime_error("SpecVector storage exceeded");
    }
    IntlData_t& dnew = data_[new_pos];
    dnew.key_ = index;
    const SpecVector * const prev = prev_;
    dnew.value_ = (prev == nullptr) ? origin_[index] : (*prev)[index];
    dnew.next_.store(NULL_POS, std::memory_order_release);

    while(!ptr->compare_exchange_weak(pos, new_pos, std::memory_order_release, std::memory_order_relaxed)) {
      while (pos != NULL_POS) {
        IntlData_t& d = data_[pos];
        if (d.key_ == index) {
          int new_new_pos = new_pos + 1;
          if(!cur_avl_data_.compare_exchange_strong(new_new_pos, new_pos, std::memory_order_release, std::memory_order_relaxed)) {
            dnew.next_.store(INVALID_POS, std::memory_order_release);
          }
          return d.value_;
        }
        ptr = &d.next_;
        pos = ptr->load(std::memory_order_relaxed);
      }
    }

    return dnew.value_;
  }

  const VALUE_T& operator[](size_t index) const
  {
    assert(alloc_chunk_data_);

    const int hash_pos = static_cast<int>(std::hash<KEY_T>{}(index) & (hash_table_size_ - 1));
    for (const SpecVector * p = this; p != nullptr; p = p->prev_) {
      const std::atomic<int> *ptr = p->hash_table_ + hash_pos;
      IntlData_t * const local_data = p->data_;
      for (int pos = ptr->load(std::memory_order_relaxed);
           pos != NULL_POS;
           pos = ptr->load(std::memory_order_relaxed)) {
        const IntlData_t& d = local_data[pos];
        if (d.key_ == index) {
          return d.value_;
        }
        ptr = &d.next_;
      }
    }

    return origin_[index];
  }

  bool operator==(const SpecVector& other) const
  {
    static const auto pair_comp = [](const std::pair<KEY_T, VALUE_T>& a, const std::pair<KEY_T, VALUE_T>& b) { return a.first < b.first; };

    bool equal = (origin_ == other.origin_)
    && (prev_ == other.prev_)
    && (alloc_chunk_data_ == other.alloc_chunk_data_)
    && (hash_table_size_ == other.hash_table_size_);

    if (equal) {

      const int alloc_items = (alloc_chunk_data_ + hash_table_size_ - 1) / hash_table_size_ * 8;
      const size_t total_bytes = sizeof(std::pair<KEY_T, VALUE_T>) * alloc_items * 2;
#ifndef NOALLOCA
      std::pair<KEY_T, VALUE_T> * const my_p = reinterpret_cast<std::pair<KEY_T, VALUE_T> *>(alloca(total_bytes));
#else
      std::pair<KEY_T, VALUE_T> * const my_p = reinterpret_cast<std::pair<KEY_T, VALUE_T> *>(malloc(total_bytes));
#endif
      std::pair<KEY_T, VALUE_T> * const other_p = my_p + alloc_items;

      for (int h = 0; h < hash_table_size_; ++h) {

        int my_elems = store_list_helper(hash_table_ + h, data_, my_p, alloc_items);
        int other_elems = store_list_helper(other.hash_table_ + h, other.data_, other_p, alloc_items);

        if (my_elems != other_elems) {
#ifdef NOALLOCA
          free(my_p);
#endif
          return false;
        }

        if (my_elems) {
          if (my_elems == 1) { // optimize for 1 element
            if (my_p[0] != other_p[0]) {
#ifdef NOALLOCA
              free(my_p);
#endif
              return false;
            }
          } else if (my_elems == 2) { // optimize for 2 elements
            if (my_p[0].first > my_p[1].first) {
              std::swap(my_p[0], my_p[1]);
            }
            if (other_p[0].first > other_p[1].first) {
              std::swap(other_p[0], other_p[1]);
            }
            if ((my_p[0] != other_p[0]) || (my_p[1] != other_p[1])) {
#ifdef NOALLOCA
              free(my_p);
#endif
              return false;
            }
          } else { // general case
            std::sort(my_p, my_p + my_elems, pair_comp);
            std::sort(other_p, other_p + my_elems, pair_comp);
            if (!std::equal(my_p, my_p + my_elems, other_p)) {
#ifdef NOALLOCA
              free(my_p);
#endif
              return false;
            }
          }
        }

      }
#ifdef NOALLOCA
      free(my_p);
#endif
    }

    return equal;
  }

  bool operator!=(const SpecVector& other) const
  {
    return !(*this == other);
  }

  void unlink() noexcept
  {
    prev_ = nullptr;
  }

  void copy_back() const
  {
    const int end = cur_avl_data_.load(std::memory_order_relaxed);
    for (int i = 0; i < end; ++i) {
      if (data_[i].valid()) {
        origin_[data_[i].key_] = data_[i].value_;
      }
    }
  }

  void next(SpecVector& dest) const
  {
    assert(alloc_chunk_data_);

    dest.origin_ =  origin_;
    dest.prev_ = this;
    dest.resize_for(*this);
    dest.cur_avl_data_.store(0, std::memory_order_relaxed);
    std::fill(dest.hash_table_, dest.hash_table_ + hash_table_size_, NULL_POS);
  }

  ~SpecVector()
  {
    if (alloc_chunk_data_) {
      free(data_);
      free(hash_table_);
    }
  }

};

template <typename VALUE_T, typename KEY_T>
constexpr int SpecVector<VALUE_T, KEY_T>::NULL_POS;

template <typename VALUE_T, typename KEY_T>
constexpr int SpecVector<VALUE_T, KEY_T>::INVALID_POS;

}

#endif
