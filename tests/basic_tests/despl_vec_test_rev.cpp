/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     despl_vec_test_rev.cpp
/// \brief    Tests occasional access from one iteration to several consecutive elements of the array and also tests ReductionVar class (reverse iteration)
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <type_traits>
#include <random>
#include <algorithm>
#include <functional>
#include <limits>

constexpr unsigned int RAND_SEED = 1287361u;

constexpr long long int DESPL = -500000;

const unsigned int thres = (std::numeric_limits<unsigned int>::max() - std::numeric_limits<unsigned int>::min()) / 20;  //5%

long long int N = 500000000LL;

void seq_test(std::vector<unsigned int>& vec, unsigned int& acc)
{
  vec[N-1] = ((vec[N-1] ^ vec[N-2] ^ vec[N-3] ^ vec[N-4] ^ vec[N-5]) < thres) ? (vec[N-1] ^ vec[N-2] ^ vec[N-3] ^ vec[N-4] ^ vec[N-5]) : vec[N-1];
  const unsigned int accv0 = vec[N-1];
  acc = accv0;
  auto tseq_begin = profile_clock_t::now();
  for(long long int i = N+DESPL-1; i >= 5+DESPL; --i) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i-DESPL-1] ^ vec[i-DESPL-2] ^ vec[i-DESPL-3] ^ vec[i-DESPL-4] ^ vec[i-DESPL-5];
    if (d < thres) {
      vec[i-DESPL-1] = d;
    } else {
      acc += accv0 ^ static_cast<unsigned int>(i-DESPL);
    }
  }
  auto tseq_end = profile_clock_t::now();
  vec[3] = ((vec[3] ^ vec[2] ^ vec[1] ^ vec[0]) < thres) ? (vec[3] ^ vec[2] ^ vec[1] ^ vec[0]) : vec[3];
  vec[2] = ((vec[2] ^ vec[1] ^ vec[0]) < thres) ? (vec[2] ^ vec[1] ^ vec[0]) : vec[2];
  vec[1] = ((vec[1] ^ vec[2]) < thres) ? (vec[1] ^ vec[2]) : vec[1];
  
  unsigned int s_xor = 0;
  for (const unsigned int el : vec) {
    s_xor ^= el;
  }
  const unsigned int s_max = *std::max_element(vec.begin(), vec.end());
  std::cout << "Seq   : " << "red_res: " << acc  << " vec_xorsum: " << s_xor << " vec_max: " << s_max << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl; 
}

std::vector<unsigned int> bigvec;
SpecLib::ReductionVar<unsigned int> acci(0, std::plus<unsigned int>());
unsigned int acc0;

std::vector<unsigned int> vec0(bigvec);

std::vector<unsigned int> seqvec(vec0);
unsigned int seqacci = 0;

const auto reset_result = [] () {
  bigvec = vec0;
  bigvec[N-1] = ((bigvec[N-1] ^ bigvec[N-2] ^ bigvec[N-3] ^ bigvec[N-4] ^ bigvec[N-5]) < thres) ? (bigvec[N-1] ^ bigvec[N-2] ^ bigvec[N-3] ^ bigvec[N-4] ^ bigvec[N-5]) : bigvec[N-1];
  acc0 = bigvec[N-1];
  acci = SpecLib::ReductionVar<unsigned int>(0, std::plus<unsigned int>(), acc0);
};

const auto test_f = [] () {
  bigvec[3] = ((bigvec[3] ^ bigvec[2] ^ bigvec[1] ^ bigvec[0]) < thres) ? (bigvec[3] ^ bigvec[2] ^ bigvec[1] ^ bigvec[0]) : bigvec[3];
  bigvec[2] = ((bigvec[2] ^ bigvec[1] ^ bigvec[0]) < thres) ? (bigvec[2] ^ bigvec[1] ^ bigvec[0]) : bigvec[2];
  bigvec[1] = ((bigvec[1] ^ bigvec[2]) < thres) ? (bigvec[1] ^ bigvec[2]) : bigvec[1];
  return ((acci == seqacci) && (bigvec == seqvec));
};

double avg_time;

bool lambda_test()
{
  const auto loop_f = [&](const long long int i, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i-DESPL-1] ^ vec[i-DESPL-2] ^ vec[i-DESPL-3] ^ vec[i-DESPL-4] ^ vec[i-DESPL-5];
    if (d < thres) {
      vec[i-DESPL-1] = d;
    } else {
      acc.thread_val() += acc0 ^ static_cast<unsigned int>(i-DESPL);
    }
  };

  const bool test_ok = bench(N+DESPL-1LL, 4LL+DESPL, -1LL, loop_f, reset_result, test_f, avg_time, bigvec, acci);

  unsigned int s_xor = 0;
  for (const unsigned int el : bigvec) {
    s_xor ^= el;
  }
  const unsigned int s_max = *std::max_element(bigvec.begin(), bigvec.end());
  std::cout << "Lambda: " << "red_res: " << acci.result()  << " vec_xorsum: " << s_xor << " vec_max: " << s_max << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

bool lambda_loop_test()
{
  const auto loop_f = [&](const SpecLib::ExCommonSpecInfo_t& cs, const long long int begin, const long long int end, const long long int step, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc) {
    for(long long int i = begin; i > end && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
      mywait(DelaySeconds);
#endif
      const unsigned int d = vec[i-DESPL-1] ^ vec[i-DESPL-2] ^ vec[i-DESPL-3] ^ vec[i-DESPL-4] ^ vec[i-DESPL-5];
      if (d < thres) {
        vec[i-DESPL-1] = d;
      } else {
        acc.thread_val() += acc0 ^ static_cast<unsigned int>(i-DESPL);
      }
    }
  };

  const bool test_ok = bench(N+DESPL-1LL, 4LL+DESPL, -1LL, loop_f, reset_result, test_f, avg_time, bigvec, acci);

  unsigned int s_xor = 0;
  for (const unsigned int el : bigvec) {
    s_xor ^= el;
  }
  const unsigned int s_max = *std::max_element(bigvec.begin(), bigvec.end());
  std::cout << "Lambda loop: " << "red_res: " << acci.result()  << " vec_xorsum: " << s_xor << " vec_max: " << s_max << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}


static inline void sf(const long long int i, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc)
{
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i-DESPL-1] ^ vec[i-DESPL-2] ^ vec[i-DESPL-3] ^ vec[i-DESPL-4] ^ vec[i-DESPL-5];
    if (d < thres) {
      vec[i-DESPL-1] = d;
    } else {
      acc.thread_val() += acc0 ^ static_cast<unsigned int>(i-DESPL);
    }
}

bool sf_test()
{
#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(N+DESPL-1LL, 4LL+DESPL, -1LL, sf, reset_result, test_f, avg_time, bigvec, acci);

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  unsigned int s_xor = 0;
  for (const unsigned int el : bigvec) {
    s_xor ^= el;
  }
  const unsigned int s_max = *std::max_element(bigvec.begin(), bigvec.end());
  std::cout << "SF    : " << "red_res: " << acci.result()  << " vec_xorsum: " << s_xor << " vec_max: " << s_max << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;
  
  return test_ok;
}

static inline void sf_loop(const SpecLib::ExCommonSpecInfo_t& cs, const long long int begin, const long long int end, const long long int step, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc)
{
  for(long long int i = begin; i > end && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i-DESPL-1] ^ vec[i-DESPL-2] ^ vec[i-DESPL-3] ^ vec[i-DESPL-4] ^ vec[i-DESPL-5];
    if (d < thres) {
      vec[i-DESPL-1] = d;
    } else {
      acc.thread_val() += acc0 ^ static_cast<unsigned int>(i-DESPL);
    }
  }
}

bool sf_loop_test()
{
  const bool test_ok = bench(N+DESPL-1LL, 4LL+DESPL, -1LL, sf_loop, reset_result, test_f, avg_time, bigvec, acci);

  unsigned int s_xor = 0;
  for (const unsigned int el : bigvec) {
    s_xor ^= el;
  }
  const unsigned int s_max = *std::max_element(bigvec.begin(), bigvec.end());
  std::cout << "SF loop: " << "red_res: " << acci.result()  << " vec_xorsum: " << s_xor << " vec_max: " << s_max << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

int main(int argc, char **argv)
{
  process_args(argc, argv, "hc:d:m:N:n:t:s:v", N);
  N = std::max(5LL, N);
  
  auto mt_rand_gen = std::bind(std::uniform_int_distribution<unsigned int>(std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max()), std::mt19937(static_cast<std::mt19937::result_type>(static_cast<unsigned int>(RAND_SEED))));
  
  bigvec.resize(N);
  for (long long int i = N-1; i >= 0; --i) {
    bigvec[i] = mt_rand_gen();
  }
  
  vec0 = bigvec;
  
  seqvec = vec0;
  seqacci = 0;
  
  bigvec[N-1] = ((bigvec[N-1] ^ bigvec[N-2] ^ bigvec[N-3] ^ bigvec[N-4] ^ bigvec[N-5]) < thres) ? (bigvec[N-1] ^ bigvec[N-2] ^ bigvec[N-3] ^ bigvec[N-4] ^ bigvec[N-5]) : bigvec[N-1];
  acc0 = bigvec[N-1];
  acci.set(acc0);
  
  seq_test(seqvec, seqacci);
  
  do_preheat(); // Preheat

  return lambda_test() && lambda_loop_test() && sf_test() && sf_loop_test() ? 0 : -1;
}
