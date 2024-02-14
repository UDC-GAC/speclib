/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     maxmin_vec_test.cpp
/// \brief    Test based on finding the largest and the smallest integers in a vector and storing in a vector the largest element found up to that position
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include <random>
#include <algorithm>
#include <functional>
#include <limits>
#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"

constexpr int RAND_SEED = 981;

size_t N = 1000;
int *Vals;
std::vector<int> SeqVResult, MinMax(2);

void seq_test()
{ std::vector<int> v_result(N, 0);

  MinMax[0] = std::numeric_limits<int>::max();
  MinMax[1] = std::numeric_limits<int>::min();

  auto tseq_begin = profile_clock_t::now();
  for (size_t i = 0; i < N; i++) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[i] < MinMax[0]) {
      MinMax[0] = Vals[i];
    }
    if (Vals[i] > MinMax[1]) {
      MinMax[1] = Vals[i];
    }
    v_result[i] = MinMax[1];
  }
  auto tseq_end = profile_clock_t::now();
  
  SeqVResult = v_result;

  std::cout << "Seq   : " << MinMax[0] << " to " << MinMax[1] << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl;
}

std::vector<int> v_result, minmax(2);
double avg_time;

const auto reset_result = [] () {
  minmax[0] = std::numeric_limits<int>::max();
  minmax[1] = std::numeric_limits<int>::min();
  std::fill(v_result.begin(), v_result.end(), 0);
};

const auto test_f = [] () { return (minmax == MinMax) && (v_result == SeqVResult); };

/// Run test on std::vector using a lambda function
bool lambda_test()
{

  const auto loop_f = [&](const size_t iteration, std::vector<int>& minmax, SpecLib::SpecConsecVector<int>& v_result) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[iteration] < minmax[0]) {
      minmax[0] = Vals[iteration];
    }
    if (Vals[iteration] > minmax[1]) {
      minmax[1] = Vals[iteration];
    }
    v_result[iteration] = minmax[1];
  };
  
  const bool test_ok = bench(0, N, 1, loop_f, reset_result, test_f, avg_time, minmax, SpecLib::SpecConsecVector<int>(v_result));

  std::cout << "Lambda: " << minmax[0] << " to " << minmax[1] << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

static inline void sf(const size_t iteration, std::vector<int>& minmax, SpecLib::SpecConsecVector<int>& v_result)
{
#ifdef ENABLE_DELAY
  mywait(DelaySeconds);
#endif
  if (Vals[iteration] < minmax[0]) {
    minmax[0] = Vals[iteration];
  }
  if (Vals[iteration] > minmax[1]) {
    minmax[1] = Vals[iteration];
  }
  v_result[iteration] = minmax[1];
}

/// Run test on std::vector using a standard function
bool sf_test()
{
#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(0, N, 1, sf, reset_result, test_f, avg_time, minmax, SpecLib::SpecConsecVector<int>(v_result));

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  std::cout << "SF    : " << minmax[0] << " to " << minmax[1] << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;
  
  return test_ok;
}

/// Run test on an array based on a raw pointer using a standard function
bool sf_test_raw_ptr()
{ int * const v_result = new int [N];
  
  const auto reset_result = [v_result] () {
    minmax[0] = std::numeric_limits<int>::max();
    minmax[1] = std::numeric_limits<int>::min();
    std::fill(v_result, v_result + N, 0);
  };

  const auto test_f = [v_result] () { return (minmax == MinMax) && std::equal(SeqVResult.begin(), SeqVResult.end(), v_result); };
  
  const bool test_ok = bench(0, N, 1, sf, reset_result, test_f, avg_time, minmax, SpecLib::SpecConsecVector<int>(v_result));

  std::cout << "SF ptr: " << minmax[0] << " to " << minmax[1] << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;
  
  delete [] v_result;
  
  return test_ok;
}

int main(int argc, char **argv)
{
  process_args(argc, argv, "hc:d:m:N:n:t:s:v", N);

  Vals = new int[N];
  auto mt_rand_gen = std::bind(std::uniform_int_distribution<int>(std::numeric_limits<int>::min(), std::numeric_limits<int>::max()), std::mt19937(static_cast<std::mt19937::result_type>(static_cast<int>(RAND_SEED))));
  for (size_t i = 0; i < N; i++) {
    Vals[i] = mt_rand_gen();
  }
  Vals[N - std::max(2 * N/NChunks, N)] = * std::max_element(Vals, Vals + N) + 1;	//put the maximum value in the first position
  
  seq_test();
  
  do_preheat();	// Preheat

  v_result.resize(N, 0);
  return lambda_test() && sf_test() && sf_test_raw_ptr() ? 0 : -1;
}
