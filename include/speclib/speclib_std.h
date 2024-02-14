/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     speclib_std.h
/// \brief    Implementation based on standard C++11 facilities
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#ifndef _SPECLIB_STD_H_
#define _SPECLIB_STD_H_

#include <type_traits>
#include "speclib/ThreadPool.h"
#include "speclib/LinkedListPool.h"
#if defined(SLSTATS) || defined(SLMINIMALSTATS)
#include <mutex>
#endif
#ifdef SLSIMULATE
#include <random>
#include <chrono>
#endif

namespace SpecLib {

/// Common data for a set of speculative executions
struct CommonSpecInfo_t {

  std::atomic<size_t> nthreads_; ///< # threads available for planning the execution of a new chunk
  void * volatile cancelled_ptr_; ///< ::WorkNode of chunk with failed validation
  volatile int check_var; ///< variable to check if the execution of the chunk has been canceled

  void reset(const size_t nthreads) noexcept
  {
    cancelled_ptr_ = nullptr;
    check_var = 0;
    nthreads_.store(nthreads);
  }

  bool cancelled() const noexcept { return check_var != 0; }

  void *cancelled_node() const noexcept { return cancelled_ptr_; }

#if __cplusplus < 202002L
  void cancel(void * p) noexcept { cancelled_ptr_ = p; check_var |= 1; }

  void seq_cancel() noexcept { check_var |= 2; }

  void end_seq_cancel() noexcept { check_var ^= 2; }
#else
  // Compound assignment with 'volatile'-qualified left operand is deprecated in C++20
  void cancel(void * p) noexcept { cancelled_ptr_ = p; check_var = check_var | 1; }

  void seq_cancel() noexcept { check_var = check_var | 2; }

  void end_seq_cancel() noexcept { check_var = check_var ^ 2; }
#endif

  bool failed() const noexcept { return (cancelled_ptr_ != nullptr);}

};

/// External data for a set of speculative runs that the user can use to obtain additional information
struct ExCommonSpecInfo_t {
  const CommonSpecInfo_t& cs; ///< The common data associated with this set of speculative executions
  const bool isParExec; ///< True for executions of the parallel part, False for executions of the sequential part
  const bool fromSpeculative; ///< Variable indicating whether this set of speculative runs starts from a speculative initial value or not

#ifndef SLNOCANCEL
  bool cancelled() const noexcept {
    return (cs.cancelled() && (isParExec || cs.failed()));
  }
#else
  constexpr bool cancelled() const noexcept {
    return false;
  }
#endif

  ExCommonSpecInfo_t(const CommonSpecInfo_t& cs_, const bool isParExec_, const bool fromSpeculative_) :
  cs(cs_),
  isParExec(isParExec_),
  fromSpeculative(fromSpeculative_)
  {}

};

namespace internal {

#ifdef SLSIMULATE
auto real_rand_gen = std::bind(std::uniform_real_distribution<float>(0,1), std::mt19937(static_cast<std::mt19937::result_type>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch()).count())));
#endif

ThreadPool& SpecLibThreadPool()
{ static ThreadPool SpecLibThreadPool_(0);
  
  return SpecLibThreadPool_;
}

template<typename T, typename = decltype(&T::operator())>
std::true_type  intl_supports_call_test(const T&);

std::false_type intl_supports_call_test(...);

template<class T>
using supports_call_t = decltype(intl_supports_call_test(std::declval<T>()));

template<typename T>
struct Deduct_ExCommonSpecInfo_t;

template<bool B, typename T>
struct GetOpParType_t {
  using type = std::false_type;
};

template<typename T>
struct GetOpParType_t<true, T> {
  using type = typename Deduct_ExCommonSpecInfo_t<decltype(&T::operator())>::type;
};

template<typename T>
struct Deduct_ExCommonSpecInfo_t {
  using type = typename GetOpParType_t<supports_call_t<T>::value, T>::type;
};

template<typename Ret, typename... ArgT>
struct Deduct_ExCommonSpecInfo_t<Ret(const ExCommonSpecInfo_t&, ArgT...)> {
  using type = std::true_type;
};

template<typename Ret, typename... ArgT>
struct Deduct_ExCommonSpecInfo_t<Ret(&)(const ExCommonSpecInfo_t&, ArgT...)> {
  using type = std::true_type;
};

template<typename ClassT, typename Ret, typename... ArgT>
struct Deduct_ExCommonSpecInfo_t<Ret (ClassT::*) (const ExCommonSpecInfo_t&, ArgT...) const> {
  using type = std::true_type;
};

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
class ThreadPoolHandler;

/// Supports the parallel execution of a chunk
template <const bool PosStep, typename F, typename Ti, typename... ArgT>
class WorkNode {

  using TupleVal_t = typename ChunkVals_t<ArgT...>::TupleVal_t;

  static constexpr int Disabled = 0x4000;
  size_t spec_info_idx_; ///< id of the CommonSpecInfo_t associated to this WorkNode in WorkNode::SpecInfos
  volatile size_t paral_threads_; ///< \# threads for the parallel execution of the chunk, without the sequential one
  Ti begin_, end_;
  size_t grain_, grainD_, grainM_;
  volatile bool seq_valid; ///< indicates if the execution of the sequential part has finished before the parallel part
  ChunkVals_t<ArgT...> chunk_vals_;
  std::atomic<size_t> in_threads_; ///< \# threads that started the seq (first one) + parallel execution
  std::atomic<size_t> out_threads_; ///< \# threads that finished the parallel execution
  std::atomic<int> validation_state_; /**< Number of events before validation is run. They are
                                       always the sequential run + (the creation of the next chunk
                                       or the final wait on the last chunk). In speculative chunks
                                       a third dependency is the validation of the previous chunk
                                     */
  int pre_val_state_; /**< indicates the origins of the ::WorkNode, 0 for the first of the entire system, 1 if it
                       comes from non-speculative values (failure recovery), 2 if it comes from speculative values
                     */
#ifdef THREADINSTRUMENT
  bool forced_parallelization_;
#endif
#ifdef SLSTATS
  volatile int aux_statstiming_flag = 1; // Auxiliary flag to ensure a good reading of some times
  profile_clock_t::time_point wts0;                 // ////////// //
  profile_clock_t::time_point wts1;                 // Timepoints //
  profile_clock_t::time_point wts2;                 // to         //
  profile_clock_t::time_point wts3;                 // profile    //
  profile_clock_t::time_point wts4;                 // execution  //
  profile_clock_t::time_point wts5;                 // time       //
  profile_clock_t::time_point wtp0;                 //            //
  profile_clock_t::time_point wtp1;                 //            //
  profile_clock_t::time_point wtp2;                 //            //
  profile_clock_t::time_point wtv0;                 //            //
  profile_clock_t::time_point wtv1;                 //            //
  profile_clock_t::time_point wtv2;                 //            //
  profile_clock_t::time_point wtv3;                 //            //
  std::vector<profile_clock_t::time_point> awt2;    //            //
  std::vector<profile_clock_t::time_point> awt3;    //            //
  std::vector<profile_clock_t::time_point> awt4;    // ////////// //

  double wts5adj = 0.0;         // Auxiliar time to determine the extra-parallel effort done by thread 0
  double wtimeW6 = 0.0;         // Wait for the execution of the last chunk and return whether everything was ok
  double wtimeOPs = 0.0;        // Overhead - Preparation time before sequential section
  double wtimeRSs = 0.0;        // Runtime  - Execution time of the sequential section
  double wtimeOWs = 0.0;        // Overhead - End time after sequential section (includes wait while canceling by sequential)
  double wtimeVV = 0.0;         // Validate - Time spent in validations
  double wtimeW1o = 0.0;        // Wait - Waiting time spent by threads waiting to receive more work (special case)
  std::vector<double> wtimeOP;  // Overhead - Preparation time before parallel section ([0] includes most preparation done by thread 0)
  std::vector<double> wtimeRP;  // Runtime  - Parallel section execution time
  std::vector<double> wtimeOW;  // Overhead - End time after parallel section
  std::vector<double> wtimeW1;  // Wait - Waiting time spent by threads waiting to receive more work
#endif

  WorkNode *next; //< Pointer to the next ::WorkNode

  static std::array<std::atomic<std::uintptr_t>, 2> SpecInfos_sync; /**< Synchronization array for the 2 SpecInfos that allows to ensure
                                                                     that they are no longer in use when they are going to be reset
                                                                   */
  static LinkedListPool<WorkNode<PosStep, F, Ti, ArgT...>> Pool;
  static ThreadPoolHandler<PosStep, F, Ti, ArgT...> *TPH;
  static CommonSpecInfo_t SpecInfos[2]; /**< There are at most 2 SpecInfos alive at a given point:
                                         One associated to a failed speculation, and another one
                                         associated to the subsequent chunks restarted from that point.
                                       */
  static size_t CurrSpecInfoIdx; ///< Currently active CommonSpecInfo_t in WorkNode::SpecInfos
  static Ti Step;
  static Ti End;
  static size_t Absolute_chunk_size;

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
  struct StatsRunInfoInternal {
    unsigned long long int successes = 0;
    unsigned long long int failures = 0;
    unsigned long long int sequential = 0;
    StatsProfileTimers pt;

    void reset() noexcept {
      successes = 0;
      failures = 0;
      sequential = 0;
      pt.reset();
    }
  };

public:
  static std::mutex statsMutex;
  static StatsRunInfoInternal statsRT;
  static thread_local StatsRunInfoInternal statsR;
private:
#endif

#ifdef SLSIMULATE
public:
  static bool simulate_mode;
  static float simulate_ratio_successes;
private:
#endif

  static CommonSpecInfo_t& CurrentSpecInfo() noexcept { return SpecInfos[CurrSpecInfoIdx]; }

  static void PrepareSpecInfo(ThreadPoolHandler<PosStep, F, Ti, ArgT...> * const tph, const Ti step, const Ti end, const size_t absolute_chunk_size) noexcept
  {
    TPH = tph;
    CurrSpecInfoIdx = 0;
    CurrentSpecInfo().reset(TPH->nthreads() + 1);
    SpecInfos_sync[0].store(static_cast<std::uintptr_t>(0u), std::memory_order_relaxed);
    SpecInfos_sync[1].store(static_cast<std::uintptr_t>(0u), std::memory_order_relaxed);
    Step = step;
    End = end;
    Absolute_chunk_size = absolute_chunk_size;
#if defined(SLSTATS) || defined(SLMINIMALSTATS)
    statsR.reset();
    statsRT.reset();
#endif
  }

  static void RecoverFromFailure(WorkNode * const curr_head)
  {
#ifdef SLSTATS
    const profile_clock_t::time_point t0 = profile_clock_t::now();
#endif
    assert(curr_head != nullptr);
    WorkNode * const last_correct_chunk = reinterpret_cast<WorkNode *>(CurrentSpecInfo().cancelled_node());
    assert(curr_head != last_correct_chunk);
#ifdef SLSTATS
    const profile_clock_t::time_point t1 = profile_clock_t::now();
#endif
    curr_head->trigger_validation();
#ifdef SLSTATS
    const profile_clock_t::time_point t2 = profile_clock_t::now();
#endif
    while(SpecInfos_sync[1u - CurrSpecInfoIdx]);
    CurrSpecInfoIdx = 1u - CurrSpecInfoIdx;
    CurrentSpecInfo().reset(TPH->nthreads() + 1);
    WorkNode * const new_head = Pool.defaultmalloc();
#ifdef SLSTATS
    const profile_clock_t::time_point t3 = profile_clock_t::now();
#endif
    new_head->fill(last_correct_chunk, false);
    while (last_correct_chunk->validation_state_.load(std::memory_order_relaxed) == 0);
    last_correct_chunk->free();
#ifdef SLSTATS
    const profile_clock_t::time_point t4 = profile_clock_t::now();
    statsR.pt.gwtimeOF += std::chrono::duration<double>(t1 - t0).count() + std::chrono::duration<double>(t3 - t2).count() + std::chrono::duration<double>(t4 - new_head->wts5).count();
#endif
  }

  template<std::size_t... Is>
  static void copy_back_array_chunks_helper(const TupleVal_t& v, std::index_sequence<Is...>)
  {
    (void)std::initializer_list<int>{(spec_copy_back_array_chunks(std::get<Is>(v)), 0)...};
  }

  /// Copies back speculative SpecConsecVector chunks to their source
  static inline void copy_back_array_chunks(const TupleVal_t& v)
  {
    copy_back_array_chunks_helper(v, std::index_sequence_for<ArgT...>{});
  }

  template<std::size_t... Is>
  static void reduce_ReductionVars_helper(TupleVal_t& v, std::index_sequence<Is...>)
  {
    (void)std::initializer_list<int>{(reduce_if_ReductionVar(std::get<Is>(v)), 0)...};
  }

  /// Invokes reduction() on ReductionVars
  static inline void reduce_ReductionVars(TupleVal_t& v)
  {
    reduce_ReductionVars_helper(v, std::index_sequence_for<ArgT...>{});
  }

  template<std::size_t... Is>
  static void initialize_ReductionVars_helper(TupleVal_t& v, std::index_sequence<Is...>)
  {
    (void)std::initializer_list<int>{(initialize_if_ReductionVar(std::get<Is>(v)), 0)...};
  }

  /// Invokes reduction() on ReductionVars
  static inline void initialize_ReductionVars(TupleVal_t& v)
  {
    initialize_ReductionVars_helper(v, std::index_sequence_for<ArgT...>{});
  }

  template<std::size_t... Is>
  static void unlink_SpecVectors_helper(TupleVal_t& v, std::index_sequence<Is...>)
  {
    (void)std::initializer_list<int>{(unlink_if_SpecVector(std::get<Is>(v)), 0)...};
  }

  /// Invokes unlink() on SpecVectors
  static inline void unlink_SpecVectors(TupleVal_t& v)
  {
    unlink_SpecVectors_helper(v, std::index_sequence_for<ArgT...>{});
  }

  /// Invokes unlink() on SpecVectors
  static inline void unlink_SpecVectors(ChunkVals_t<ArgT...>& cv)
  {
    unlink_SpecVectors(cv.seqVals_);
    unlink_SpecVectors(cv.specVals_);
  }

  template<std::size_t... Is>
#if __cplusplus >= 201703L
  void fill_next_val_helper(const TupleVal_t& v, [[maybe_unused]] const Ti begin, [[maybe_unused]] const size_t size, std::index_sequence<Is...>)
  {
#else
  void fill_next_val_helper(const TupleVal_t& v, const Ti begin, const size_t size, std::index_sequence<Is...>)
  {
    static_cast<void>(begin); static_cast<void>(size);	// [[maybe_unused]]
#endif
    (void)std::initializer_list<int>{(next_spec_version<PosStep>(std::get<Is>(v), std::get<Is>(chunk_vals_.seqVals_), begin, size), 0)...};
  }

  /// Get speculative version of the data \c v for the next chunk, of size \c size.
  /** Only affects SpecConsecVector objects */
  void fill_next_val(const TupleVal_t& v, const Ti begin, const size_t size)
  {
    fill_next_val_helper(v, begin, size, std::index_sequence_for<ArgT...>{});
  }

  template<typename F2, std::size_t... Is>
  static std::enable_if_t<!Deduct_ExCommonSpecInfo_t<F2>::type::value> apply_helper(const Ti begin, const Ti end, const F2& f, const ExCommonSpecInfo_t& exspec_info, std::tuple<ArgT...>& args, std::index_sequence<Is...>)
  {
    const Ti step = Step;
#if __cplusplus >= 201703L
    if constexpr(PosStep) {
#else
    if (PosStep) {
#endif
      for (Ti i = begin; (i < end) && !exspec_info.cancelled(); i += step) {
        f(i, std::get<Is>(args)...);
      }
    } else {
      for (Ti i = begin; (i > end) && !exspec_info.cancelled(); i += step) {
        f(i, std::get<Is>(args)...);
      }
    }
  }

  template<typename F2, std::size_t... Is>
  static std::enable_if_t<Deduct_ExCommonSpecInfo_t<F2>::type::value> apply_helper(const Ti begin, const Ti end, const F2& f, const ExCommonSpecInfo_t& exspec_info, std::tuple<ArgT...>& args, std::index_sequence<Is...>)
  {
    f(exspec_info, begin, end, Step, std::get<Is>(args)...);
  }

  /// Run iterations \c begin to \c end for function \c f on the arguments \c args
  static inline void apply(const Ti begin, const Ti end, const F&f, const ExCommonSpecInfo_t& exspec_info, std::tuple<ArgT...>& args)
  {
    apply_helper(begin, end, f, exspec_info, args, std::index_sequence_for<ArgT...>{});
  }

  /// Computes the length of the speculative chunk that starts at \c begin
  static inline size_t spec_size(const Ti begin) noexcept { return ((PosStep) ? static_cast<size_t>(std::min(End, begin + static_cast<Ti>(Absolute_chunk_size)) - begin) : static_cast<size_t>(begin - std::max(End, begin - static_cast<Ti>(Absolute_chunk_size)))); }

  bool enabled() const noexcept { return !(in_threads_.load(std::memory_order_relaxed) & Disabled); }

  void common_fill(Ti begin, const int validation_state, const int pre_val_state)
  {
    assert(!enabled());
    chunk_vals_.specVals_ = chunk_vals_.seqVals_;
    spec_info_idx_ = CurrSpecInfoIdx;
    const size_t available_paral_threads = CurrentSpecInfo().nthreads_.load(std::memory_order_relaxed) - 1;
    paral_threads_ = std::max(available_paral_threads, TPH->min_paral_nthreads());
    begin_ = begin;
    end_ = (PosStep) ? (std::min(End, begin + static_cast<Ti>(Absolute_chunk_size))) : (std::max(End, begin - static_cast<Ti>(Absolute_chunk_size)));
    grain_ = static_cast<size_t>((PosStep) ? ((end_ - begin_ + Step - 1) / Step) : ((end_ - begin_ + Step + 1) / Step));
    grainD_ = (grain_ / paral_threads_);
    grainM_ = (grain_ % paral_threads_);
    seq_valid = false;
    out_threads_.store(1);
    validation_state_.store(validation_state);
    pre_val_state_ = pre_val_state;
#ifdef THREADINSTRUMENT
    forced_parallelization_ = (paral_threads_ > available_paral_threads);
#endif
    next = nullptr;
    SpecInfos_sync[spec_info_idx_].fetch_xor(reinterpret_cast<std::uintptr_t>(this), std::memory_order_relaxed);
#ifdef SLSTATS
    wts5adj = 0.0;
    wtimeW6 = 0.0;
    wtimeW1o = 0.0;
    awt2.resize(paral_threads_);
    awt3.resize(paral_threads_);
    awt4.resize(paral_threads_);
    wtimeOP.resize(paral_threads_);
    wtimeRP.resize(paral_threads_);
    wtimeOW.resize(paral_threads_);
    wtimeW1.resize(paral_threads_);
#endif
  }

  // Only performed by main thread invoked from ThreadPoolHandler
  void push_process()
  {
    pre_push_chunk();
    in_threads_.store(0); // enables the WorkNode
    TPH->push_chunk(this);
    post_push_chunk();
  }

  void pre_push_chunk()
  {
    CurrentSpecInfo().nthreads_.fetch_sub(1);
  }

  void post_push_chunk()
  {
    paral_run(static_cast<size_t>(0)); // Runs the first parallel portion of the chunk

    // Wait for the other parallel threads to finish their portion of the chunk (doing work if that can help)
    while (out_threads_.load(std::memory_order_relaxed) < (paral_threads_+1)) {
      const size_t aux_n = in_threads_.load(std::memory_order_relaxed);
      if (aux_n < paral_threads_ && aux_n) {
#ifdef SLSTATS
        const profile_clock_t::time_point t0 = profile_clock_t::now();
#endif
        const size_t my_n = in_threads_.fetch_add(1);
        if (my_n && my_n < paral_threads_) {
#ifdef SLSTATS
          awt2[my_n] = t0;
          wtimeW1[my_n] = 0.0;
#endif
          paral_run(my_n);
#ifdef SLSTATS
          const profile_clock_t::time_point t1 = profile_clock_t::now();
          wtimeOW[my_n] = std::chrono::duration<double>(t1 - awt4[my_n]).count();
          wts5adj += std::chrono::duration<double>(t1 - t0).count();
#endif
        }
      }
    }
#ifdef SLSTATS
    if (out_threads_.load(std::memory_order_relaxed) >= (paral_threads_+1)) {
      aux_statstiming_flag = 0;
    }
#endif
  }

  CommonSpecInfo_t& mySpecInfo() const noexcept { return SpecInfos[spec_info_idx_]; }

#ifdef SLSTATS
  void slstats_gather(const bool failed_val) {
    wtimeVV = std::chrono::duration<double>(wtv1 - wtv0).count() + std::chrono::duration<double>(wtv3 - wtv2).count();
    if (failed_val) {
      statsR.pt.gwtimeFF += (wtimeRSs + wtimeVV + wtimeW6 + wtimeOWs + wtimeOPs + wtimeW1o);
      while(aux_statstiming_flag);
      const size_t m_paral_threads_ = paral_threads_;
      for (size_t i = 0; i < m_paral_threads_; i++) {
        statsR.pt.gwtimeFF += (wtimeRP[i] + wtimeOW[i] + wtimeOP[i] + wtimeW1[i]);
      }
    } else {
      statsR.pt.gwtimeRSs += wtimeRSs;
      statsR.pt.gwtimeVV += wtimeVV;
      statsR.pt.gwtimeW6 += wtimeW6;
      statsR.pt.gwtimeOWs += wtimeOWs;
      statsR.pt.gwtimeOPs += wtimeOPs;
      while(aux_statstiming_flag);
      double ltimeW1 = wtimeW1o;
      const size_t m_paral_threads_ = paral_threads_;
      for (size_t i = 0; i < m_paral_threads_; i++) {
        statsR.pt.gwtimeRP += wtimeRP[i];
        statsR.pt.gwtimeOW += wtimeOW[i];
        statsR.pt.gwtimeOP += wtimeOP[i];
        ltimeW1 += wtimeW1[i];
      }
      switch(pre_val_state_) {
        case 0:
          statsR.pt.gwtimeOPi += ltimeW1;
          break;
        case 1:
          statsR.pt.gwtimeFF += ltimeW1;
          break;
        case 2:
          statsR.pt.gwtimeW1 += ltimeW1;
      }
    }
  }
#endif

  void trigger_validation()
  {
    if (validation_state_.fetch_sub(1) == 1) {
#ifdef THREADINSTRUMENT
      ThreadInstrument::log(VERIFY, BEGIN, true);
#endif

#ifdef SLSTATS
      wtv0 = profile_clock_t::now();
      bool prefailed;
#endif
      CommonSpecInfo_t& myspecinfo = mySpecInfo();
      if (!myspecinfo.failed()) {
#ifdef SLSTATS
        prefailed = false;
#endif
        copy_back_array_chunks(chunk_vals_.seqVals_);

        // The last validation is always successful because the seqVals_
        //were obtained from previously correct speculated values anyway
        if (seq_valid) {
#if defined(SLSTATS) || defined(SLMINIMALSTATS)
          ++statsR.sequential;
#endif
#ifdef SLSIMULATE
        } else if (((PosStep) ? (end_ < End) : (end_ > End)) && ((simulate_mode && (real_rand_gen() >= simulate_ratio_successes)) || (!simulate_mode && (chunk_vals_.seqVals_ != chunk_vals_.specVals_)))) {
#else
        } else if (((PosStep) ? (end_ < End) : (end_ > End)) && (chunk_vals_.seqVals_ != chunk_vals_.specVals_)) {
#endif
          myspecinfo.cancel(this);
#if defined(SLSTATS) || defined(SLMINIMALSTATS)
          ++statsR.failures;
        } else {
          ++statsR.successes;
#endif
        }
#ifdef SLSTATS
      } else {
        prefailed = true;
#endif
      }
#ifdef THREADINSTRUMENT
      ThreadInstrument::log(VERIFY, END, true);
#endif

      if (next != nullptr) {
        unlink_SpecVectors(next->chunk_vals_);
#ifdef SLSTATS
        wtv1 = profile_clock_t::now();
#endif
        next->trigger_validation();
#ifdef SLSTATS
        wtv2 = profile_clock_t::now();
      } else {
        wtv1 = profile_clock_t::now();
        wtv2 = wtv1;
#endif
      }

      const auto cancelled_ptr = reinterpret_cast<WorkNode *>(myspecinfo.cancelled_node());
      if( ( ((PosStep) ? (end_ < End) : (end_ > End)) && (cancelled_ptr != this) ) ||
//          ( ((PosStep) ? (end_ >= End) : (end_ == End)) && (cancelled_ptr != nullptr) ) ) {
          ( ((PosStep) ? (end_ >= End) : (end_ <= End)) && (cancelled_ptr != nullptr) ) ) {
        SpecInfos_sync[spec_info_idx_].fetch_xor(reinterpret_cast<std::uintptr_t>(this), std::memory_order_relaxed);
#ifdef SLSTATS
        wtv3 = profile_clock_t::now();
        slstats_gather(prefailed);
#endif
        free();
      } else {
        SpecInfos_sync[spec_info_idx_].fetch_xor(reinterpret_cast<std::uintptr_t>(this), std::memory_order_relaxed);
        validation_state_.store(-1); // signal RecoverFromFailure that it can free this
#ifdef SLSTATS
        wtv3 = profile_clock_t::now();
        slstats_gather(prefailed);
#endif
      }
    }
  }

  friend class ThreadPoolHandler<PosStep, F, Ti, ArgT...>;
  friend class LinkedListPool<WorkNode<PosStep, F, Ti, ArgT...>>;

public:

  WorkNode() :
  spec_info_idx_{0},
  paral_threads_{0},
  begin_{0},
  end_{0},
  grain_{0},
  grainD_{0},
  grainM_{0},
  seq_valid{false},
  in_threads_{Disabled},
  out_threads_{1},
  validation_state_{0},
  pre_val_state_{0},
  next{nullptr}
  { }

  // Only used for the first chunk
  template<typename... ArgT2>
  void fill(Ti begin, ArgT2&&... args)
  {
#ifndef SLSTATS
    chunk_vals_.seqVals_ = TupleVal_t(spec_version<PosStep>(args, begin, spec_size(begin))...);
    unlink_SpecVectors(chunk_vals_.seqVals_);
    common_fill(begin, 2, 0);
    push_process();
#else
    wts0 = profile_clock_t::now();
    chunk_vals_.seqVals_ = TupleVal_t(spec_version<PosStep>(args, begin, spec_size(begin))...);
    unlink_SpecVectors(chunk_vals_.seqVals_);
    common_fill(begin, 2, 0);
    awt2[0] = wts0;
    push_process();
    wts5 = profile_clock_t::now();
    wtimeOW[0] = std::chrono::duration<double>(wts5-awt4[0]).count() - wts5adj;
#endif
  }

  void fill(WorkNode* const prev, const bool from_speculative)
  {
#ifndef SLSTATS
    fill_next_val(from_speculative ? prev->chunk_vals_.specVals_ : prev->chunk_vals_.seqVals_, prev->end_, spec_size(prev->end_));
    if (!from_speculative) {
      unlink_SpecVectors(chunk_vals_.seqVals_);
    }
    common_fill(prev->end_, 2 + from_speculative, 1 + from_speculative);
    if (from_speculative) {
      prev->next = this;
      prev->trigger_validation();
    }
    push_process();
#else
    wts0 = profile_clock_t::now();
    fill_next_val(from_speculative ? prev->chunk_vals_.specVals_ : prev->chunk_vals_.seqVals_, prev->end_, spec_size(prev->end_));
    if (!from_speculative) {
      unlink_SpecVectors(chunk_vals_.seqVals_);
    }
    common_fill(prev->end_, 2 + from_speculative, 1 + from_speculative);
    if (from_speculative) {
      prev->next = this;
      wts1 = profile_clock_t::now();
      prev->trigger_validation();
      wts2 = profile_clock_t::now();
    } else {
      wts1 = profile_clock_t::now();
      wts2 = wts1;
    }
    awt2[0] = wts2;
    push_process();
    wts5 = profile_clock_t::now();
    wtimeOP[0] += std::chrono::duration<double>(wts1-wts0).count();
    wtimeOW[0] = std::chrono::duration<double>(wts5-awt4[0]).count() - wts5adj;
#endif
  }

  Ti begin() const noexcept { return begin_; }

  Ti end() const noexcept { return end_; }

  /// Sequentially run a full chunk
#ifdef SLSTATS
  void seq_run(const profile_clock_t::time_point& t1)
#else
  void seq_run()
#endif
  {
#ifdef THREADINSTRUMENT
    const auto thread_instrument_code = forced_parallelization_ ? SEQCOMPF : SEQCOMP;
    ThreadInstrument::log(thread_instrument_code, (((int) begin_) << 1) | BEGIN, true);
#endif

    const ExCommonSpecInfo_t exMySpecInfo(mySpecInfo(), false, (pre_val_state_ >= 2));
#ifdef SLSTATS
    wtp0 = profile_clock_t::now();
#endif
    initialize_ReductionVars(chunk_vals_.seqVals_);
    apply(begin_, end_, TPH->f(), exMySpecInfo, chunk_vals_.seqVals_);
    reduce_ReductionVars(chunk_vals_.seqVals_);
#ifdef SLSTATS
    wtp1 = profile_clock_t::now();
#endif

    mySpecInfo().nthreads_.fetch_add(1);
    if (!mySpecInfo().failed()) {
      if (out_threads_.load(std::memory_order_relaxed) < (paral_threads_+1)) {
        if (out_threads_.fetch_sub(1, std::memory_order_relaxed) < (paral_threads_+1)) {
          mySpecInfo().seq_cancel();
          seq_valid = true;
          while (out_threads_.load(std::memory_order_relaxed) < paral_threads_);
          mySpecInfo().end_seq_cancel();
          chunk_vals_.specVals_ = chunk_vals_.seqVals_;
        }
#ifdef SLSTATS
        if (aux_statstiming_flag && out_threads_.load(std::memory_order_relaxed) >= paral_threads_) {
          aux_statstiming_flag = 0;
        }
#endif
        out_threads_.fetch_add(1, std::memory_order_relaxed);
      }
    }

#ifdef SLSTATS
    wtp2 = profile_clock_t::now();
    wtimeOPs = std::chrono::duration<double>(wtp0 - t1).count();
    wtimeRSs = std::chrono::duration<double>(wtp1 - wtp0).count();
    wtimeOWs = std::chrono::duration<double>(wtp2 - wtp1).count();
#endif

#ifdef THREADINSTRUMENT
    ThreadInstrument::log(thread_instrument_code, (((int) begin_) << 1) | END, true);
#endif

    trigger_validation();

  }

  /// Run a parallel portion of a chunk according to the number of \c thread
  void paral_run(const size_t nthread)
  {
#ifdef THREADINSTRUMENT
    const auto thread_instrument_code = forced_parallelization_ ? PARCOMPF : PARCOMP;
    ThreadInstrument::log(thread_instrument_code, (((int) begin_) << 1) | BEGIN, true);
#endif
    const Ti b = (Ti) (nthread*(grainD_) + std::min(nthread, grainM_));
    const Ti e = (Ti) ((nthread+1)*(grainD_) + std::min((nthread+1), grainM_));

    const Ti begin = (PosStep) ? std::min(End, begin_ + b*Step) : std::max(End, begin_ + b*Step);
    const Ti end = (PosStep) ? std::min(End, begin_ + e*Step) : std::max(End, begin_ + e*Step);

#ifdef SLSTATS
    awt3[nthread] = profile_clock_t::now();
#endif
    const ExCommonSpecInfo_t exMySpecInfo(mySpecInfo(), true, (pre_val_state_ >= 2));
    initialize_ReductionVars(chunk_vals_.specVals_);
    apply(begin, end, TPH->f(), exMySpecInfo, chunk_vals_.specVals_);
    reduce_ReductionVars(chunk_vals_.specVals_);

#ifdef SLSTATS
    awt4[nthread] = profile_clock_t::now();
    wtimeOP[nthread] = std::chrono::duration<double>(awt3[nthread] - awt2[nthread]).count();
    wtimeRP[nthread] = std::chrono::duration<double>(awt4[nthread] - awt3[nthread]).count();
#endif
    out_threads_.fetch_add(1);

#ifdef THREADINSTRUMENT
    ThreadInstrument::log(thread_instrument_code, (((int) begin_) << 1) | END, true);
#endif
  }

  /// Wait for the execution of the last chunk and return whether everything was ok
  bool wait_success()
  {
#ifdef SLSTATS
    const profile_clock_t::time_point t0 = profile_clock_t::now();
#endif

    while (!mySpecInfo().failed() && (validation_state_.load(std::memory_order_relaxed) > 1));
//    while (!mySpecInfo().failed() && (validation_state_.load(std::memory_order_relaxed) != 1));

    const bool was_failed = mySpecInfo().failed();
    
    // we do not care if the last validation fails because if all the previous validations
    // are correct, the result of the sequential execution of the last chunk, which is
    // the final result, is also correct and so there is no need to repeat any execution

#ifdef SLSTATS
    const profile_clock_t::time_point t1 = profile_clock_t::now();
    wtimeW6 = std::chrono::duration<double>(t1 - t0).count();
#endif

    if (was_failed) {
      RecoverFromFailure(this);
    } else {
      trigger_validation();
    }

    return !was_failed;
  }
  
  /// Return result of the speculative execution
  const TupleVal_t& result() const noexcept
  {
    return chunk_vals_.seqVals_;
  }
  
  void free() noexcept
  {
    in_threads_.store(Disabled); // disables WorkNode
    Pool.free(this);
  }

  ~WorkNode()
  { }
  
};

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
LinkedListPool<WorkNode<PosStep, F, Ti, ArgT...>> WorkNode<PosStep, F, Ti, ArgT...>::Pool(4);

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
ThreadPoolHandler<PosStep, F, Ti, ArgT...> *WorkNode<PosStep, F, Ti, ArgT...>::TPH;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
CommonSpecInfo_t WorkNode<PosStep, F, Ti, ArgT...>::SpecInfos[2];

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
std::array<std::atomic<std::uintptr_t>, 2> WorkNode<PosStep, F, Ti, ArgT...>::SpecInfos_sync;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
size_t WorkNode<PosStep, F, Ti, ArgT...>::CurrSpecInfoIdx;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
Ti WorkNode<PosStep, F, Ti, ArgT...>::Step;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
Ti WorkNode<PosStep, F, Ti, ArgT...>::End;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
size_t WorkNode<PosStep, F, Ti, ArgT...>::Absolute_chunk_size;

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
template <const bool PosStep, typename F, typename Ti, typename... ArgT>
thread_local typename WorkNode<PosStep, F, Ti, ArgT...>::StatsRunInfoInternal WorkNode<PosStep, F, Ti, ArgT...>::statsR;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
typename WorkNode<PosStep, F, Ti, ArgT...>::StatsRunInfoInternal WorkNode<PosStep, F, Ti, ArgT...>::statsRT;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
std::mutex WorkNode<PosStep, F, Ti, ArgT...>::statsMutex;
#endif

#ifdef SLSIMULATE
template <const bool PosStep, typename F, typename Ti, typename... ArgT>
bool WorkNode<PosStep, F, Ti, ArgT...>::simulate_mode;

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
float WorkNode<PosStep, F, Ti, ArgT...>::simulate_ratio_successes;
#endif

template <const bool PosStep, typename F, typename Ti, typename... ArgT>
class ThreadPoolHandler {
  using My_WorkNode_t = WorkNode<PosStep, F, Ti, ArgT...>;

  using TupleVal_t = typename ChunkVals_t<ArgT...>::TupleVal_t;

  const size_t min_paral_nthreads_;
  volatile bool finish_;
  My_WorkNode_t * volatile head_;
  const F& f_;

  void main()
  {
#if defined(SLSTATS) || defined(SLMINIMALSTATS)
    My_WorkNode_t::statsR.reset();
#endif
#if SLSTATS
    profile_clock_t::time_point t0 = profile_clock_t::now();
    profile_clock_t::time_point t1;
#endif
    while (!finish_) {
      const auto curr_head = head_;
      if ((curr_head != nullptr) && (curr_head->in_threads_.load(std::memory_order_relaxed) < curr_head->paral_threads_)) {
        const size_t my_n = curr_head->in_threads_.fetch_add(1);
        if (!my_n) {
#ifdef SLSTATS
          t1 = profile_clock_t::now();
          curr_head->wtimeW1[my_n] = std::chrono::duration<double>(t1 - t0).count();
          curr_head->seq_run(t1); // Runs the chunk sequentially
#else
          curr_head->seq_run(); // Runs the chunk sequentially
#endif
#ifdef SLSTATS
          t0 = profile_clock_t::now();
#endif
        } else if (my_n < curr_head->paral_threads_) {
#ifdef SLSTATS
          t1 = profile_clock_t::now();
          curr_head->wtimeW1[my_n] = std::chrono::duration<double>(t1 - t0).count();
          curr_head->awt2[my_n] = t1;
#endif
          curr_head->paral_run(my_n); //Runs the non-first parallel portions of the chunk
#ifdef SLSTATS
          t0 = profile_clock_t::now();
          curr_head->wtimeOW[my_n] = std::chrono::duration<double>(t0 - curr_head->awt4[my_n]).count();
          if (curr_head->out_threads_.load(std::memory_order_relaxed) >= (curr_head->paral_threads_+1)) {
            curr_head->aux_statstiming_flag = 0;
          }
        } else {
          t1 = profile_clock_t::now();
          curr_head->wtimeW1o += std::chrono::duration<double>(t1 - t0).count();
          t0 = profile_clock_t::now();
#endif
        }
      }
    }
#ifdef SLSTATS
    t1 = profile_clock_t::now();
    const double lwtimeW3 = std::chrono::duration<double>(t1 - t0).count();
    {
      std::lock_guard<std::mutex> lck(My_WorkNode_t::statsMutex);
      My_WorkNode_t::statsRT.successes += My_WorkNode_t::statsR.successes;
      My_WorkNode_t::statsRT.failures += My_WorkNode_t::statsR.failures;
      My_WorkNode_t::statsRT.sequential += My_WorkNode_t::statsR.sequential;
      My_WorkNode_t::statsRT.pt += My_WorkNode_t::statsR.pt;
      My_WorkNode_t::statsRT.pt.gwtimeW3 += lwtimeW3;
    }
#else
#ifdef SLMINIMALSTATS
    {
      std::lock_guard<std::mutex> lck(My_WorkNode_t::statsMutex);
      My_WorkNode_t::statsRT.successes += My_WorkNode_t::statsR.successes;
      My_WorkNode_t::statsRT.failures += My_WorkNode_t::statsR.failures;
      My_WorkNode_t::statsRT.sequential += My_WorkNode_t::statsR.sequential;
    }
#endif
#endif
  }

  const F& f() const noexcept { return f_; }

  void push_chunk(WorkNode<PosStep, F, Ti, ArgT...> * const p) noexcept
  {
    head_ = p;
  }

  friend class WorkNode<PosStep, F, Ti, ArgT...>;

public:

  template<typename... ArgT2>
  ThreadPoolHandler(const size_t nthreads, const size_t min_paral_nthreads,
                    Ti begin, Ti end, Ti step,
                    size_t absolute_chunk_size, const F& f, ArgT2&&... args) :
  min_paral_nthreads_{min_paral_nthreads},
  finish_{false},
  head_{nullptr},
  f_{f}
  {
#ifdef SLSTATS
    const profile_clock_t::time_point t0 = profile_clock_t::now();
#endif

    SpecLibThreadPool().resize(nthreads);
    SpecLibThreadPool().setFunction(&ThreadPoolHandler::main, this);
    SpecLibThreadPool().launchTheads();

    My_WorkNode_t::PrepareSpecInfo(this, step, end, absolute_chunk_size);

#ifdef SLSTATS
    const profile_clock_t::time_point t1 = profile_clock_t::now();
    const double lwtimeOPi = std::chrono::duration<double>(t1 - t0).count();
#endif

    My_WorkNode_t::Pool.defaultmalloc()->fill(begin, std::forward<ArgT2>(args)...);

    do {
      while((PosStep) ? (head_->end() < end) : (head_->end() > end)) {
        if (My_WorkNode_t::CurrentSpecInfo().failed()) {
          My_WorkNode_t::RecoverFromFailure(head_);
        } else {
          My_WorkNode_t::Pool.defaultmalloc()->fill(head_, true);
        }
      }
    } while (!head_->wait_success());
#ifdef SLSTATS
    {
      std::lock_guard<std::mutex> lck(My_WorkNode_t::statsMutex);
      My_WorkNode_t::statsRT.successes += My_WorkNode_t::statsR.successes;
      My_WorkNode_t::statsRT.failures += My_WorkNode_t::statsR.failures;
      My_WorkNode_t::statsRT.sequential += My_WorkNode_t::statsR.sequential;
      My_WorkNode_t::statsRT.pt += My_WorkNode_t::statsR.pt;
      My_WorkNode_t::statsRT.pt.gwtimeOPi += lwtimeOPi;
    }
#else
#ifdef SLMINIMALSTATS
    {
      std::lock_guard<std::mutex> lck(My_WorkNode_t::statsMutex);
      My_WorkNode_t::statsRT.successes += My_WorkNode_t::statsR.successes;
      My_WorkNode_t::statsRT.failures += My_WorkNode_t::statsR.failures;
      My_WorkNode_t::statsRT.sequential += My_WorkNode_t::statsR.sequential;
    }
#endif
#endif
  }

  size_t nthreads() const noexcept { return SpecLibThreadPool().nthreads(); }

  size_t min_paral_nthreads() const noexcept { return min_paral_nthreads_; }

  const TupleVal_t& result() const noexcept
  {
    assert(!(My_WorkNode_t::CurrentSpecInfo()).failed());
    assert(head_ != nullptr);
    return head_->result();
  }

  ~ThreadPoolHandler()
  {
    assert(head_ != nullptr);
    head_->free();
    finish_ = true;
    SpecLibThreadPool().wait();
  }

};

} //namespace internal
                      
/// \brief Runs the speculative loop
/// \tparam    F         Type of the function that provides the body of the loop
/// \tparam    Args      Types of the objects on which the speculation is performed
/// \param[in] config    Configuration of the parallel execution
/// \param[in] begin     Starting point for the loop
/// \param[in] end       Limit of the loop in C style
/// \param[in] step      Step of the loop
/// \param[in] specChunk Number of iterations in each speculative chunk
/// \param[in] f         Function that implements the body of the loop
/// \param[in,out] args  Objects on which the speculation is performed
template <typename F, typename Ti, typename... ArgT>
#if !defined(SLSTATS) && !defined(SLMINIMALSTATS)
void specRun(Configuration config, const typename std::remove_reference<Ti>::type begin, const Ti end, const typename std::remove_reference<Ti>::type step, size_t specChunk, const F& f, ArgT&&... args)
#else
StatsRunInfo specRun(Configuration config, const typename std::remove_reference<Ti>::type begin, const Ti end, const typename std::remove_reference<Ti>::type step, size_t specChunk, const F& f, ArgT&&... args)
#endif
{
#if defined(SLSTATS) || defined(SLMINIMALSTATS) || defined(SLSIMULATE)
  using My_Work_Node_Pos_t = internal::WorkNode<true, F, Ti, typename internal::SpecType<ArgT>::type...>;
  using My_Work_Node_Neg_t = internal::WorkNode<false, F, Ti, typename internal::SpecType<ArgT>::type...>;
#endif
  static_assert(std::is_integral<Ti>::value, "Integer required.");
#ifdef SLSTATS
  const profile_clock_t::time_point start_time = profile_clock_t::now();
#endif
  const bool posStep = (std::is_unsigned<std::remove_cv_t<std::remove_reference_t<Ti>>>::value || (step >= 0));
  if (posStep) {
    if (begin < end) {
      config.nthreads_ = std::max(config.nthreads_, static_cast<size_t>(3));
      config.min_paral_nthreads_ = std::max(std::min(config.min_paral_nthreads_, config.nthreads_), static_cast<size_t>(2));
      specChunk = std::max(specChunk, static_cast<size_t>(1));
#ifdef SLSIMULATE
      if (config.simulate_ratio_successes_ < 0.0f) {
        config.simulate_ratio_successes_ = -1.0f;
        My_Work_Node_Pos_t::simulate_mode = false;
      } else {
        My_Work_Node_Pos_t::simulate_mode = true;
      }
      My_Work_Node_Pos_t::simulate_ratio_successes = config.simulate_ratio_successes_;
#endif
      {
        internal::ThreadPoolHandler<true, F, Ti, typename internal::SpecType<ArgT>::type...> thread_pool(config.nthreads_ - 1, config.min_paral_nthreads_, begin, end, step, specChunk * (size_t)(step), f, std::forward<ArgT>(args)...);
        std::tie(internal::final_spec_assign(args)...) = thread_pool.result();
      }
#ifdef SLSTATS
      const profile_clock_t::time_point end_time = profile_clock_t::now();
      const double total_time = std::chrono::duration<double>(end_time - start_time).count();
      return StatsRunInfo(config.nthreads_, My_Work_Node_Pos_t::statsRT.successes, My_Work_Node_Pos_t::statsRT.failures, My_Work_Node_Pos_t::statsRT.sequential, total_time, My_Work_Node_Pos_t::statsRT.pt);
    }
    return StatsRunInfo(0, 0, 0, 0);
#else
#ifdef SLMINIMALSTATS
      return StatsRunInfo(config.nthreads_, My_Work_Node_Pos_t::statsRT.successes, My_Work_Node_Pos_t::statsRT.failures, My_Work_Node_Pos_t::statsRT.sequential);
    }
    return StatsRunInfo(0, 0, 0, 0);
#else
    }
#endif
#endif
  } else {
    if (begin > end) {
      config.nthreads_ = std::max(config.nthreads_, static_cast<size_t>(3));
      config.min_paral_nthreads_ = std::max(std::min(config.min_paral_nthreads_, config.nthreads_), static_cast<size_t>(2));
      specChunk = std::max(specChunk, static_cast<size_t>(1));
#ifdef SLSIMULATE
      if (config.simulate_ratio_successes_ < 0.0f) {
        config.simulate_ratio_successes_ = -1.0f;
        My_Work_Node_Neg_t::simulate_mode = false;
      } else {
        My_Work_Node_Neg_t::simulate_mode = true;
      }
      My_Work_Node_Neg_t::simulate_ratio_successes = config.simulate_ratio_successes_;
#endif
      {
        internal::ThreadPoolHandler<false, F, Ti, typename internal::SpecType<ArgT>::type...> thread_pool(config.nthreads_ - 1, config.min_paral_nthreads_, begin, end, step, specChunk * (size_t)(-step), f, std::forward<ArgT>(args)...);
        std::tie(internal::final_spec_assign(args)...) = thread_pool.result();
      }
#ifdef SLSTATS
      const profile_clock_t::time_point end_time = profile_clock_t::now();
      const double total_time = std::chrono::duration<double>(end_time - start_time).count();
      return StatsRunInfo(config.nthreads_, My_Work_Node_Neg_t::statsRT.successes, My_Work_Node_Neg_t::statsRT.failures, My_Work_Node_Neg_t::statsRT.sequential, total_time, My_Work_Node_Neg_t::statsRT.pt);
    }
    return StatsRunInfo(0, 0, 0, 0);
#else
#ifdef SLMINIMALSTATS
      return StatsRunInfo(config.nthreads_, My_Work_Node_Neg_t::statsRT.successes, My_Work_Node_Neg_t::statsRT.failures, My_Work_Node_Neg_t::statsRT.sequential);
    }
    return StatsRunInfo(0, 0, 0, 0);
#else
    }
#endif
#endif
  }
}

} //namespace SpecLib

#endif
