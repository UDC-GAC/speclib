/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     ReductionVar.h
/// \brief    Variable for efficient thread-safe reductions
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///


#ifndef __REDUCTIONVAR_H_
#define __REDUCTIONVAR_H_

#include <array>
#include <atomic>
#include <cassert>
#include <functional>
#include <stdexcept>
#include <mutex>

namespace SpecLib {

template<typename T, const size_t NUM_VARS = 2048>
class ReductionVar {

  static thread_local std::array<T, NUM_VARS> comm_vals_;

  struct CommonValue_t {
    std::atomic<char> state_;
    CommonValue_t() :
    state_{0}
    { }
  };
  static std::array<CommonValue_t, NUM_VARS>& getCommonStorage() {
    static std::array<CommonValue_t, NUM_VARS> CommonStorage{};
    return CommonStorage;
  }

  static size_t startFindStorageId;  // not thread-safe but its only a hint from where start the find process

  T identity_;
  std::function<T(const T&, const T&)> reduction_function_;
  T common_value_;
  size_t storageId_;
  std::mutex red_mtx;

  static size_t FindAvailableVar() {
      auto& CommonStorage = getCommonStorage();
      const size_t localStartFindStorageId = startFindStorageId;
      size_t i = localStartFindStorageId;
      do {
      if (!CommonStorage[i].state_.load(std::memory_order_relaxed)) {
        char tmp = 0;
        if (CommonStorage[i].state_.compare_exchange_strong(tmp, 1)) {
          startFindStorageId = ((i + 1) < NUM_VARS) ? (i + 1) : 0;
          return i;
        }
      }
      i = ((i + 1) < NUM_VARS) ? (i + 1) : 0;
    }while(i != localStartFindStorageId);
    throw std::runtime_error("Unable to allocate ReductionVar");
  }

public:

  ReductionVar() noexcept :
  storageId_{NUM_VARS}
  { }

  template<typename F>
  ReductionVar(const T identity, const F& reduction_function) :
  identity_{identity},
  reduction_function_{reduction_function},
  common_value_{identity},
  storageId_{FindAvailableVar()}
  {
    comm_vals_[storageId_] = identity;
  }

  template<typename F>
  ReductionVar(const T identity, const F& reduction_function, const T& init_value) :
  identity_{identity},
  reduction_function_{reduction_function},
  common_value_{init_value},
  storageId_{FindAvailableVar()}
  {
    comm_vals_[storageId_] = identity;
  }

  ~ReductionVar() {
    free_storage();
  }

  ReductionVar(const ReductionVar& other) :                  // Copy constructor
  identity_{other.identity_},
  reduction_function_{other.reduction_function_},
  common_value_{other.common_value_},
  storageId_{other.empty() ? NUM_VARS : FindAvailableVar()}
  {
    if (!empty()) {
      comm_vals_[storageId_] = other.comm_vals_[other.storageId_];
    }
  }

  ReductionVar(ReductionVar&& other) noexcept :              // Move constructor
  identity_{other.identity_},
  reduction_function_{other.reduction_function_},
  common_value_{other.common_value_},
  storageId_{other.storageId_}
  {
    other.storageId_ = NUM_VARS;
  }

  ReductionVar& operator=(const ReductionVar& other) {       // Copy assignment
    if (other.empty()) {
      free_storage();
      storageId_ = NUM_VARS;
    } else {
      if (empty()) {
        storageId_ = FindAvailableVar();
      }
      common_value_ = other.common_value_;
      identity_ = other.identity_;
      reduction_function_ = other.reduction_function_;
    }
    return *this;
  }
  ReductionVar& operator=(ReductionVar&& other) noexcept {   // Move assignment
    free_storage();
    storageId_ = other.storageId_;
    other.storageId_ = NUM_VARS;
    common_value_ = other.common_value_;
    identity_ = other.identity_;
    reduction_function_ = other.reduction_function_;
    return *this;
  }

  bool operator==(const ReductionVar& other) const noexcept {
    return result() == other.result();
  }

  template<typename P>
  bool operator==(const P& other) const noexcept {
    return result() == other;
  }

  bool operator!=(const ReductionVar& other) const noexcept {
    return !(*this == other);
  }

  template<typename P>
  bool operator!=(const P& other) const noexcept {
    return !(*this == other);
  }

  constexpr bool empty() const noexcept { return (storageId_ == NUM_VARS); }

  inline void initialize() noexcept {
    assert(!empty());
    comm_vals_[storageId_] = identity_;
  }

  inline void set(const T& new_val) noexcept {
    common_value_ = new_val;
  }
  inline void set(T&& new_val) noexcept {
    common_value_ = std::forward<T>(new_val);
  }

  inline void reduce() {
    assert(!empty());
    std::lock_guard<std::mutex> lck(red_mtx);
    common_value_ = reduction_function_(common_value_, comm_vals_[storageId_]);
  }

  constexpr T& thread_val() const noexcept {
    assert(!empty());
    return comm_vals_[storageId_];
  };

  const constexpr T& identity() const noexcept {
    return const_cast<const T&>(identity_);
  }

  constexpr T& identity_ref() const noexcept {
    return identity_;
  }

  const constexpr T& result() const noexcept {
    return const_cast<const T&>(common_value_);
  }

  constexpr T& result_ref() const noexcept {
    return common_value_;
  }

  void free_storage() noexcept {
    if (!empty()) {
//    startFindStorageId = storageId_;
      getCommonStorage()[storageId_].state_.store(0, std::memory_order_relaxed);
    }
  }
};

template<typename T, const size_t NUM_VARS>
thread_local std::array<T, NUM_VARS> ReductionVar<T, NUM_VARS>::comm_vals_{};

template<typename T, const size_t NUM_VARS>
size_t ReductionVar<T, NUM_VARS>::startFindStorageId{};

}

#endif
