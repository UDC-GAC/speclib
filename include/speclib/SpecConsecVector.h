/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     SpecConsecVector.h
/// \brief    SpecConsecVector class to speculate on array chunks
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef _SPECCONSECVECTOR_H
#define _SPECCONSECVECTOR_H

#include <cassert>
#include <algorithm>
#include <array>
#include <vector>

namespace SpecLib {

/// Speculative version of a chunk of a vector
template <typename T>
class SpecConsecVector {

  size_t copy_offset_;
  size_t size_;
  T * origin_;
  T * copy_;

  void fill_copy(T * const origin)
  {
    std::copy(origin, origin + size_, copy_);
  }

  void free_storage()
  {
    if (copy_ != nullptr) {
      delete [] copy_;
    }
  }

public:

  SpecConsecVector(std::vector<T>& source) :
  SpecConsecVector(source.data())
  {}

  template<std::size_t N>
  SpecConsecVector(std::array<T, N>& source) :
  SpecConsecVector(source.begin())
  {}

  SpecConsecVector(T * const source = nullptr) :
  copy_offset_{0},
  size_{0},
  origin_{source},
  copy_{nullptr}
  {}

  SpecConsecVector(const SpecConsecVector& source, const size_t copy_offset, const size_t size) :
  SpecConsecVector(source.origin_, copy_offset, size)
  {}

  SpecConsecVector(T * const origin, const size_t copy_offset, const size_t size) :
  copy_offset_{copy_offset},
  size_{size},
  origin_{origin},
  copy_{new T[size]}
  {
    fill_copy(origin + copy_offset);
  }

  SpecConsecVector(const SpecConsecVector& other) :
  copy_offset_{other.copy_offset_},
  size_{other.size_},
  origin_{other.origin_},
  copy_{new T[other.size_]}
  {
    fill_copy(other.copy_);
  }

  SpecConsecVector(SpecConsecVector&& other) :
  copy_offset_{other.copy_offset_},
  size_{other.size_},
  origin_{other.origin_},
  copy_{other.copy_}
  {
    other.copy_ = nullptr;
  }

  SpecConsecVector& operator=(const SpecConsecVector& other)
   {
     copy_offset_ = other.copy_offset_;
     origin_ = other.origin_;

     if(size_ != other.size_) {
       size_ = other.size_;
       free_storage();
       copy_ = new T[size_];
     }

     fill_copy(other.copy_);

     return *this;
   }

   SpecConsecVector& operator=(SpecConsecVector&& other)
   {
     copy_offset_ = other.copy_offset_;
     size_ = other.size_;
     origin_ = other.origin_;
     free_storage();
     copy_ = other.copy_;
     other.copy_ = nullptr;

     return *this;
   }

   void fill_with(T * const origin, const size_t copy_offset, const size_t size)
   {
     copy_offset_ = copy_offset;
     origin_ = origin;

     if(size_ != size) {
       size_ = size;
       free_storage();
       copy_ = new T[size_];
     }

     fill_copy(origin + copy_offset);
   }

        T& operator[](size_t index)       { assert(size_); return copy_[index - copy_offset_]; }
  const T& operator[](size_t index) const { assert(size_); return copy_[index - copy_offset_]; }

  bool operator==(const SpecConsecVector& other) const noexcept
  {
    return (origin_ == other.origin_)
    && (copy_offset_ == other.copy_offset_)
    && (size_ == other.size_)
    && std::equal(copy_, copy_ + size_, other.copy_);
  }

  bool operator!=(const SpecConsecVector& other) const noexcept
  {
    return !(*this == other);
  }

  void copy_back() const
  {
	std::copy(copy_, copy_ + size_, origin_ + copy_offset_);
  }

  template<const bool PosStep>
  void next(SpecConsecVector<T>& dest, const size_t size) const
  {
    assert(size_);
    dest.fill_with(origin_, (PosStep) ? (copy_offset_ +  size_) : (copy_offset_ -  size_), size);
  }

  ~SpecConsecVector()
  {
    free_storage();
  }

};

} //namespace SpecLib

#endif
