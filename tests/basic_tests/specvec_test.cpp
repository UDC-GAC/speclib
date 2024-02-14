/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     specvec_test.cpp
/// \brief    Test SpecVector class
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include <numeric>
#include <algorithm>
#include <random>
#include <functional>
#include <limits>
#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"

constexpr int RAND_SEED = 981;

size_t N = 1000;
int *Vals;
std::vector<int> SeqResult;

void seq_test()
{
  std::fill(SeqResult.begin(), SeqResult.end(), 0);

  auto tseq_begin = profile_clock_t::now();
  for (size_t i = 0; i < N; i++) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    SeqResult[Vals[i]]++;
  }
  auto tseq_end = profile_clock_t::now();

  const int s_sum = std::accumulate(SeqResult.begin(), SeqResult.end(), 0);
  const int s_max = *std::max_element(SeqResult.begin(), SeqResult.end());

  std::cout << "Seq sum: " << s_sum << " max: " << s_max << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl;
}

std::vector<int> v_result;
double avg_time;

const auto reset_result = [] () {
  std::fill(v_result.begin(), v_result.end(), 0);
};

const auto test_f = [] () { return SeqResult == v_result; };

static inline void sf(size_t iteration, SpecLib::SpecVector<int>& result)
{
#ifdef ENABLE_DELAY
  mywait(DelaySeconds);
#endif
  result[Vals[iteration]]++;
}

static inline void sf_full(size_t iteration, std::vector<int>& result)
{
#ifdef ENABLE_DELAY
  mywait(DelaySeconds);
#endif
  result[Vals[iteration]]++;
}

bool sf_test()
{
  reset_result();

#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(0, N, 1, sf, reset_result, test_f, avg_time,  SpecLib::SpecVector<int>(v_result, static_cast<int>(static_cast<float>(N)/static_cast<float>(NChunks) * 1.1f)));

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  const int s_sum = std::accumulate(v_result.begin(), v_result.end(), 0);
  const int s_max = *std::max_element(v_result.begin(), v_result.end());

  std::cout << "SF SpecVector sum: " << s_sum << " max: " << s_max << " Test Ok=" << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

bool sf_full_test()
{
  reset_result();

  const bool test_ok = bench(0, N, 1, sf_full, reset_result, test_f, avg_time,  v_result);

  const int s_sum = std::accumulate(v_result.begin(), v_result.end(), 0);
  const int s_max = *std::max_element(v_result.begin(), v_result.end());

  std::cout << "SF Full Vector sum: " << s_sum << " max: " << s_max << " Test Ok=" << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

int main(int argc, char **argv)
{
  process_args(argc, argv, "hc:d:m:N:n:t:s:v", N);

  Vals = new int[N];

  SeqResult.resize(2*N);
  v_result.resize(2*N);

  ////////////////////////////////////////////////////////////////
  std::cout << "**** Random indirection test (v[i]=rand()) :" << std::endl;

  auto mt_rand_gen = std::bind(std::uniform_int_distribution<int>(0, std::numeric_limits<int>::max()), std::mt19937(static_cast<std::mt19937::result_type>(static_cast<int>(RAND_SEED))));
  for (size_t i = 0; i < N; i++) {
    Vals[i] = mt_rand_gen() % (2 * static_cast<int>(N));
  }

  seq_test();
  
  do_preheat(); // Preheat

  if (!sf_test() || !sf_full_test()) {
    return -1;
  }

  ////////////////////////////////////////////////////////////////
  std::cout << "**** Fixed indirection test (v[i]=0) :" << std::endl;
  
  std::fill(Vals, Vals + N, 0);

  seq_test();

  if (!sf_test() || !sf_full_test()) {
    return -1;
  }

  ////////////////////////////////////////////////////////////////
  std::cout << "**** Successive indirection test (v[i]=i) :" << std::endl;

  std::iota(Vals, Vals + N, 0);

  seq_test();

  if (!sf_test() || !sf_full_test()) {
    return -1;
  }

  return 0;
}
