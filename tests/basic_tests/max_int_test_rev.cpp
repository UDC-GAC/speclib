/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     max_int_test_rev.cpp
/// \brief    Test based on finding the largest integer in a vector (reverse iteration)
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

constexpr long long int DESPL = -500000;

long long int N = 1000;
int MaxSeq;
int *Vals;

void seq_test()
{ int max_seq = 0;
  
  auto tseq_begin = profile_clock_t::now();
  for (long long int i = N-1+DESPL; i >= 0+DESPL; i--) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[i-DESPL] > max_seq) {
      max_seq = Vals[i-DESPL];
    }
  }
  auto tseq_end = profile_clock_t::now();
  
  MaxSeq = max_seq;

  std::cout << "Seq   : " <<  max_seq << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl;
}

int max_spec;
double avg_time;

const auto reset_result = [] () { max_spec = 0; };
const auto test_f = [] () { return (max_spec == MaxSeq); };

bool lambda_test()
{
  const auto loop_f = [&](const long long int iteration, int& result) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[iteration-DESPL] > result) {
      result = Vals[iteration-DESPL];
    }
  };

  const bool test_ok = bench(N-1+DESPL, -1LL+DESPL, -1LL, loop_f, reset_result, test_f, avg_time, max_spec);

  std::cout << "Lambda: " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

bool lambda_loop_test()
{
  const auto loop_f = [&](const SpecLib::ExCommonSpecInfo_t& cs, const long long int begin, const long long int end, const long long int step, int& result) {
    for (long long int i = begin; (i > end) && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
      mywait(DelaySeconds);
#endif
      if (Vals[i-DESPL] > result) {
        result = Vals[i-DESPL];
      }
    }
  };

  const bool test_ok = bench(N-1+DESPL, -1LL+DESPL, -1LL, loop_f, reset_result, test_f, avg_time, max_spec);

  std::cout << "Lambda loop: " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}


static inline void sf(const long long int iteration, int& result)
{
#ifdef ENABLE_DELAY
  mywait(DelaySeconds);
#endif
  if (Vals[iteration-DESPL] > result) {
    result = Vals[iteration-DESPL];
  }
}

bool sf_test()
{
#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(N-1+DESPL, -1LL+DESPL, -1LL, sf, reset_result, test_f, avg_time, max_spec);

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  std::cout << "SF    : " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;
  
  return test_ok;
}

static inline void sf_loop(const SpecLib::ExCommonSpecInfo_t& cs, const long long int begin, const long long int end, const long long int step, int& result)
{
  for (long long int i = begin; (i > end) && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[i-DESPL] > result) {
      result = Vals[i-DESPL];
    }
  }
}

bool sf_loop_test()
{
  const bool test_ok = bench(N-1+DESPL, -1LL+DESPL, -1LL, sf_loop, reset_result, test_f, avg_time, max_spec);

  std::cout << "SF loop: " << max_spec << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

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
  
  do_preheat(); // Preheat

  return lambda_test() && lambda_loop_test() && sf_test() && sf_loop_test() ? 0 : -1;
}
