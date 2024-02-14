/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     reduction_test.cpp
/// \brief    Test on support of ReductionVar
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <random>
#include <functional>
#include <limits>

constexpr int RAND_SEED = 981;

size_t N = 1000;
size_t SumSeq;
int *Vals;

void seq_test()
{ size_t sum_seq = 0;

  const auto tseq_begin = profile_clock_t::now();
  for (size_t i = 0; i < N; i++) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    sum_seq += Vals[i];
  }
  const auto tseq_end = profile_clock_t::now();

  SumSeq = sum_seq;

  std::cout << "Seq   : " << sum_seq << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl;
}

SpecLib::ReductionVar<size_t> red_spec((size_t)0, std::plus<size_t>());
double avg_time;

const auto reset_result = [] () { red_spec.set(0); };
const auto test_f = [] () { return (red_spec.result() == SumSeq); };

bool lambda_test()
{
  const auto loop_f = [&](const size_t iteration, SpecLib::ReductionVar<size_t>& red_spec) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    red_spec.thread_val() += Vals[iteration];
  };

#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(0, N, 1, loop_f, reset_result, test_f, avg_time, red_spec);

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  std::cout << "Lambda: " << red_spec.result() << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

static inline void sf(const size_t iteration, SpecLib::ReductionVar<size_t>& red_spec)
{
#ifdef ENABLE_DELAY
  mywait(DelaySeconds);
#endif
  red_spec.thread_val() += Vals[iteration];
}

bool sf_test()
{
  const bool test_ok = bench(0, N, 1, sf, reset_result, test_f, avg_time, red_spec);

  std::cout << "SF    : " << red_spec.result() << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

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

  do_preheat(); // Preheat

  return lambda_test() && sf_test() ? 0 : -1;
}
