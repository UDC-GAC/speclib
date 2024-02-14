/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     common_speclib.h
/// \brief    Elements that could be common to implementations based on different backends
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef _COMMON_SPECLIB_H_
#define _COMMON_SPECLIB_H_

#include <cassert>

#include <algorithm>
#include <type_traits>
#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <tuple>
#include <vector>
#include "speclib/SpecConsecVector.h"
#include "speclib/ReductionVar.h"
#include "speclib/SpecVector.h"
#include "speclib/SpecReal.h"
#include "speclib/SpecRealInd.h"
#include "speclib/SpecAtomic.h"
#ifdef SLSTATS
#include <chrono>
#endif

#ifdef THREADINSTRUMENT
#include "thread_instrument/thread_instrument.h"
#endif

#ifdef SLNSPECDEBUG
#define assert_spec(cond) static_cast<void>(0);
#define assert_spec_ret(cond, ret) static_cast<void>(0);
#else
#define assert_spec(cond) if (!(cond)) { return; };
#define assert_spec_ret(cond, ret) if (!(cond)) { return (ret); };
#endif

namespace SpecLib {

#ifdef SLSTATS
using profile_clock_t = std::conditional<(std::chrono::high_resolution_clock::is_steady || !std::chrono::steady_clock::is_steady), std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;
#endif

template<typename Ti>
constexpr size_t getChunkSize(const Ti totalIterations, const size_t numChunks) {
  return ((size_t)totalIterations - 1u) / numChunks + 1u;
}

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
struct StatsProfileTimers {
  double gwtimeRSs = 0.0;
  double gwtimeRP = 0.0;
  double gwtimeFF = 0.0;
  double gwtimeVV = 0.0;
  double gwtimeW1 = 0.0;
  double gwtimeW3 = 0.0;
  double gwtimeW6 = 0.0;
  double gwtimeOF = 0.0;
  double gwtimeOWs = 0.0;
  double gwtimeOW = 0.0;
  double gwtimeOPi = 0.0;
  double gwtimeOPs = 0.0;
  double gwtimeOP = 0.0;

  StatsProfileTimers()
  {}

  StatsProfileTimers(double gwtimeRSs_, double gwtimeRP_, double gwtimeFF_, double gwtimeVV_, double gwtimeW1_, double gwtimeW3_, double gwtimeW6_, double gwtimeOF_, double gwtimeOWs_, double gwtimeOW_, double gwtimeOPi_, double gwtimeOPs_, double gwtimeOP_) :
    gwtimeRSs(gwtimeRSs_),
    gwtimeRP(gwtimeRP_),
    gwtimeFF(gwtimeFF_),
    gwtimeVV(gwtimeVV_),
    gwtimeW1(gwtimeW1_),
    gwtimeW3(gwtimeW3_),
    gwtimeW6(gwtimeW6_),
    gwtimeOF(gwtimeOF_),
    gwtimeOWs(gwtimeOWs_),
    gwtimeOW(gwtimeOW_),
    gwtimeOPi(gwtimeOPi_),
    gwtimeOPs(gwtimeOPs_),
    gwtimeOP(gwtimeOP_)
  { }

  StatsProfileTimers operator+ (const StatsProfileTimers& first) const {
    return StatsProfileTimers(gwtimeRSs + first.gwtimeRSs, gwtimeRP + first.gwtimeRP, gwtimeFF + first.gwtimeFF, gwtimeVV + first.gwtimeVV, gwtimeW1 + first.gwtimeW1, gwtimeW3 + first.gwtimeW3, gwtimeW6 + first.gwtimeW6, gwtimeOF + first.gwtimeOF, gwtimeOWs + first.gwtimeOWs, gwtimeOW + first.gwtimeOW, gwtimeOPi + first.gwtimeOPi, gwtimeOPs + first.gwtimeOPs, gwtimeOP + first.gwtimeOP);
  }

  StatsProfileTimers& operator+= (const StatsProfileTimers& first) {
    gwtimeRSs += first.gwtimeRSs;
    gwtimeRP += first.gwtimeRP;
    gwtimeFF += first.gwtimeFF;
    gwtimeVV += first.gwtimeVV;
    gwtimeW1 += first.gwtimeW1;
    gwtimeW3 += first.gwtimeW3;
    gwtimeW6 += first.gwtimeW6;
    gwtimeOF += first.gwtimeOF;
    gwtimeOWs += first.gwtimeOWs;
    gwtimeOW += first.gwtimeOW;
    gwtimeOPi += first.gwtimeOPi;
    gwtimeOPs += first.gwtimeOPs;
    gwtimeOP += first.gwtimeOP;
    return *this;
  }

  void reset() noexcept {
    gwtimeRSs = 0.0;
    gwtimeRP = 0.0;
    gwtimeFF = 0.0;
    gwtimeVV = 0.0;
    gwtimeW1 = 0.0;
    gwtimeW3 = 0.0;
    gwtimeW6 = 0.0;
    gwtimeOF = 0.0;
    gwtimeOWs = 0.0;
    gwtimeOW = 0.0;
    gwtimeOPi = 0.0;
    gwtimeOPs = 0.0;
    gwtimeOP = 0.0;
  }
};

struct StatsRunInfo {
  size_t totalNthreads;
  unsigned long long int successes;
  unsigned long long int failures;
  unsigned long long int sequential;
  unsigned long long int total;
  double total_exec_time;
  StatsProfileTimers pt;

  StatsRunInfo() :
    totalNthreads(0),
    successes(0),
    failures(0),
    sequential(0),
    total(0),
    total_exec_time(0.0)
  {
    pt.reset();
  }

  StatsRunInfo(size_t totalNthreads_, unsigned long long int successes_, unsigned long long int failures_, unsigned long long int sequential_, double total_exec_time_ = 0.0) :
    totalNthreads(totalNthreads_),
    successes(successes_),
    failures(failures_),
    sequential(sequential_),
    total(successes + failures + sequential),
    total_exec_time(total_exec_time_)
  {
    pt.reset();
  }

  StatsRunInfo(size_t totalNthreads_, unsigned long long int successes_, unsigned long long int failures_, unsigned long long int sequential_, double total_exec_time_, StatsProfileTimers pt_) :
    totalNthreads(totalNthreads_),
    successes(successes_),
    failures(failures_),
    sequential(sequential_),
    total(successes + failures + sequential),
    total_exec_time(total_exec_time_),
    pt(pt_)
  { }

  StatsRunInfo operator+ (const StatsRunInfo& first) const {
    return StatsRunInfo(totalNthreads ? totalNthreads : first.totalNthreads, successes + first.successes, failures + first.failures, sequential + first.sequential, total_exec_time + first.total_exec_time, pt + first.pt);
  }

  StatsRunInfo& operator+= (const StatsRunInfo& first) {
    totalNthreads = totalNthreads ? totalNthreads : first.totalNthreads;
    successes += first.successes;
    failures += first.failures;
    sequential += first.sequential;
    total += first.successes + first.failures + first.sequential;
    total_exec_time += first.total_exec_time;
    pt += first.pt;
    return *this;
  }

  void reset() noexcept {
    successes = 0;
    failures = 0;
    sequential = 0;
    total = 0;
    total_exec_time = 0.0;
    pt.reset();
  }
};
#endif

struct Configuration {

  size_t nthreads_;  ///< Total number of threads to use for the loop execution
  size_t min_paral_nthreads_; ///< Minimum number of threads to use for parallel execution of chunks
#ifdef SLSIMULATE
  float simulate_ratio_successes_ = -1.0f; ///< Simulate the percent of successes (negative number to disable)
#endif

  Configuration(size_t nthreads, size_t min_paral_nthreads = 2) :
  nthreads_{nthreads},
  min_paral_nthreads_{min_paral_nthreads}
  {}

#ifdef SLSIMULATE
  Configuration(size_t nthreads, size_t min_paral_nthreads, float simulate_ratio_successes) :
  nthreads_{nthreads},
  min_paral_nthreads_{min_paral_nthreads},
  simulate_ratio_successes_{simulate_ratio_successes}
  {}
#endif

};

namespace internal {


#ifdef THREADINSTRUMENT

/** @name Helpers for ThreadInstrument
 */
///@{
enum Activities : size_t { PARCOMP = 0, SEQCOMP = 1, PARCOMPF = 2, SEQCOMPF = 3, VERIFY = 4 };
enum State      : int { BEGIN = 0, END = 1 };

std::string simple_printer(int event, void *p)
{
  static const std::string activity [] = { "PARCOMP", "SEQCOMP", "PARCOMPF", "SEQCOMPF", "VERIFY" };
  return activity[event] + (((size_t)p & END) ? " END" : " BEGIN");
}

std::string verbose_printer(int event, void *p)
{
  const size_t code = (size_t)p;
  static const std::string activity [] = { "PARCOMP", "SEQCOMP", "PARCOMPF", "SEQCOMPF", "VERIFY" };
  return activity[event] + '_' + std::to_string(code >> 1) + ((code & END) ? " END" : " BEGIN");
}

///@}

#endif


/** @name Traits to map from argument types to speculation types and the proper actions on them
 */
///@{
template <typename T>
struct SpecType {
  using type = typename std::remove_reference<T>::type;
};


template <const bool PosStep, typename T, typename Ti>
static inline T spec_version(T& v, const Ti, const size_t)
{
  return v;
}

template <const bool PosStep, typename T, typename Ti>
static inline SpecConsecVector<T> spec_version(SpecConsecVector<T>& v, const Ti begin, const size_t size)
{
  return SpecConsecVector<T>(v, static_cast<size_t>((PosStep) ? begin : (begin+1-size)), size);
}

template <const bool PosStep, typename T, typename U, typename Ti>
static inline SpecVector<T, U> spec_version(SpecVector<T, U>& v, const Ti, const size_t)
{
  return SpecVector<T, U>(v);
}

template <const bool PosStep, typename T, typename Ti>
static inline void next_spec_version(const T& v, T& dest, const Ti, const size_t)
{
  dest = v;
}

template <const bool PosStep, typename T, typename Ti>
static inline void next_spec_version(const SpecConsecVector<T>& v, SpecConsecVector<T>& dest, const Ti, const size_t size)
{
  v.template next<PosStep>(dest, size);
}

template <const bool PosStep, typename T, typename U, typename Ti>
static inline void next_spec_version(const SpecVector<T, U>& v, SpecVector<T, U>& dest, const Ti, const size_t)
{
  v.next(dest);
}


template <typename T>
static inline void spec_copy_back_array_chunks(const T&)
{ }

template <typename T>
static inline void spec_copy_back_array_chunks(const SpecConsecVector<T>& v)
{
  v.copy_back();
}

template <typename T, typename U>
static inline void spec_copy_back_array_chunks(const SpecVector<T, U>& v)
{
  v.copy_back();
}

template <typename T>
static inline void initialize_if_ReductionVar(T&)
{ }

template <typename T>
static inline void initialize_if_ReductionVar(ReductionVar<T>& v)
{
  v.initialize();
}

template <typename T>
static inline void reduce_if_ReductionVar(T&)
{ }

template <typename T>
static inline void reduce_if_ReductionVar(ReductionVar<T>& v)
{
  v.reduce();
}



template <typename T>
static inline void unlink_if_SpecVector(T&)
{ }

template <typename T, typename U>
static inline void unlink_if_SpecVector(SpecVector<T, U>& v)
{
  v.unlink();
}


//template <typename T>
//static inline T& final_spec_assign(const T& v)
//{
//  return v;
//}

template <typename T>
static inline T& final_spec_assign(T& v)
{
  return v;
}

template <typename T>
static inline auto& final_spec_assign(const SpecConsecVector<T>&)
{
  return std::ignore;
}

template <typename T>
static inline auto& final_spec_assign(SpecConsecVector<T>&)
{
  return std::ignore;
}

template <typename T, typename U>
static inline auto& final_spec_assign(const SpecVector<T, U>&)
{
  return std::ignore;
}

template <typename T, typename U>
static inline auto& final_spec_assign(SpecVector<T, U>&)
{
  return std::ignore;
}

///@}

/// Storage for the sequential and the speculative values of a chunk
template <typename... ArgT>
struct ChunkVals_t {

  using TupleVal_t = std::tuple<ArgT...>;

  static constexpr size_t PAD = (sizeof(TupleVal_t) % 128) ? (128 - (sizeof(TupleVal_t) % 128)) : 128;

  TupleVal_t seqVals_;
  char pad1[PAD];
  TupleVal_t specVals_;

  ChunkVals_t(const TupleVal_t& src) :
  seqVals_{src},
  specVals_{src}
  { }

  ChunkVals_t(TupleVal_t&& src) :
  seqVals_{std::move(src)},
  specVals_{seqVals_}
  { }

  ChunkVals_t()
  {}

};

} //namespace internal

} //namespace SpecLib

#endif
