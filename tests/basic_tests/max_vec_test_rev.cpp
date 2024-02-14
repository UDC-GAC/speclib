/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     max_vec_test_rev.cpp
/// \brief    Test based on finding the largest integer in a vector and storing in a vector the largest element found up to that position (reverse iteration)
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <functional>
#include <limits>

constexpr int RAND_SEED = 981;

long long int N = 1000;
int MaxSeq;
int *Vals;
std::vector<int> SeqVResult;

void seq_test()
{ int max_seq = 0;
  std::vector<int> v_result(N, 0);

  auto tseq_begin = profile_clock_t::now();
  for (long long int i = N-1; i >= 0; i--) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[i] > max_seq) {
      max_seq = Vals[i];
    }
    v_result[i] = max_seq;
  }
  auto tseq_end = profile_clock_t::now();
  
  MaxSeq = max_seq;
  SeqVResult = v_result;

  std::cout << "Seq   : " << max_seq << std::endl;
  std::cout << "Time  : " << (std::chrono::duration<double>(tseq_end - tseq_begin).count()) << std::endl << std::endl;
}

int max_spec;
std::vector<int> v_result;
double avg_time;

const auto reset_result = [] () { max_spec = 0; std::fill(v_result.begin(), v_result.end(), 0); };
const auto test_f = [] () { return (max_spec == MaxSeq) && (v_result == SeqVResult); };

/// Run test on std::vector using a lambda function
bool lambda_test()
{
  const auto loop_f = [&](const long long int iteration, int& result, SpecLib::SpecConsecVector<int>& v_result) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[iteration] > result) {
      result = Vals[iteration];
    }
    v_result[iteration] = result;
  };

  const bool test_ok = bench(N-1, -1LL, -1LL, loop_f, reset_result, test_f, avg_time, max_spec, SpecLib::SpecConsecVector<int>(v_result));

  std::cout << "Lambda: " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

static inline void sf(const long long int iteration, int& result, SpecLib::SpecConsecVector<int>& v_result)
{
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
  if (Vals[iteration] > result) {
    result = Vals[iteration];
  }
  v_result[iteration] = result;
}

/// Run test on std::vector using a standard function
bool sf_test()
{
#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(N-1, -1LL, -1LL, sf, reset_result, test_f, avg_time, max_spec, SpecLib::SpecConsecVector<int>(v_result));

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  std::cout << "SF    : " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;
  
  return test_ok;
}

/// Run test on an array based on a raw pointer using a standard function
bool sf_test_raw_ptr()
{
  int * const v_result = new int [N];
  
  const auto reset_result = [v_result] () { max_spec = 0; std::fill(v_result, v_result + N, 0); };
  const auto test_f = [v_result] () { return (max_spec == MaxSeq) && std::equal(SeqVResult.begin(), SeqVResult.end(), v_result); };
  
  const bool test_ok = bench(N-1, -1LL, -1LL, sf, reset_result, test_f, avg_time, max_spec, SpecLib::SpecConsecVector<int>(v_result));

  std::cout << "SF ptr: " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;
  
  delete [] v_result;
  
  return test_ok;
}

int main(int argc, char **argv)
{
  process_args(argc, argv, "hc:d:m:N:n:t:s:v", N);

  Vals = new int[N];
  auto mt_rand_gen = std::bind(std::uniform_int_distribution<int>(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()), std::mt19937(static_cast<std::mt19937::result_type>(static_cast<int>(RAND_SEED))));
  for (long long int i = N-1; i >= 0; i--) {
    Vals[i] = mt_rand_gen();
  }
  Vals[N - 1] = * std::max_element(Vals, Vals + N) + 1;	//put the maximum value in the first position
  
  seq_test();
  
  do_preheat();	// Preheat

  v_result.resize(N, 0);
  return lambda_test() && sf_test() && sf_test_raw_ptr() ? 0 : -1;
}
