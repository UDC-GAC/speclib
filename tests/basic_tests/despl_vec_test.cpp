/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     despl_vec_test.cpp
/// \brief    Tests occasional access from one iteration to several consecutive elements of the array and also tests ReductionVar class
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

const unsigned int thres = (std::numeric_limits<unsigned int>::max() - std::numeric_limits<unsigned int>::min()) / 20;  //5%

unsigned int N = 500000000u;

void seq_test(std::vector<unsigned int>& vec, unsigned int& acc)
{
  vec[0] = ((vec[0] ^ vec[1] ^ vec[2] ^ vec[3] ^ vec[4]) < thres) ? (vec[0] ^ vec[1] ^ vec[2] ^ vec[3] ^ vec[4]) : vec[0];
  const unsigned int accv0 = vec[0];
  acc = accv0;
  auto tseq_begin = profile_clock_t::now();
  for(unsigned int i = 0; i < N-5; ++i) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i+1] ^ vec[i+2] ^ vec[i+3] ^ vec[i+4] ^ vec[i+5];
    if (d < thres) {
      vec[i+1] = d;
    } else {
      acc += accv0 ^ i;
    }
  }
  auto tseq_end = profile_clock_t::now();
  vec[N-5+1] = ((vec[N-5+1] ^ vec[N-5+2] ^ vec[N-5+3] ^ vec[N-5+4]) < thres) ? (vec[N-5+1] ^ vec[N-5+2] ^ vec[N-5+3] ^ vec[N-5+4]) : vec[N-5+1];
  vec[N-5+2] = ((vec[N-5+2] ^ vec[N-5+3] ^ vec[N-5+4]) < thres) ? (vec[N-5+2] ^ vec[N-5+3] ^ vec[N-5+4]) : vec[N-5+2];
  vec[N-5+3] = ((vec[N-5+3] ^ vec[N-5+2]) < thres) ? (vec[N-5+3] ^ vec[N-5+2]) : vec[N-5+3];
  
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
  bigvec[0] = ((bigvec[0] ^ bigvec[1] ^ bigvec[2] ^ bigvec[3] ^ bigvec[4]) < thres) ? (bigvec[0] ^ bigvec[1] ^ bigvec[2] ^ bigvec[3] ^ bigvec[4]) : bigvec[0];
  acc0 = bigvec[0];
  acci = SpecLib::ReductionVar<unsigned int>(0, std::plus<unsigned int>(), acc0);
};

const auto test_f = [] () {
  bigvec[N-5+1] = ((bigvec[N-5+1] ^ bigvec[N-5+2] ^ bigvec[N-5+3] ^ bigvec[N-5+4]) < thres) ? (bigvec[N-5+1] ^ bigvec[N-5+2] ^ bigvec[N-5+3] ^ bigvec[N-5+4]) : bigvec[N-5+1];
  bigvec[N-5+2] = ((bigvec[N-5+2] ^ bigvec[N-5+3] ^ bigvec[N-5+4]) < thres) ? (bigvec[N-5+2] ^ bigvec[N-5+3] ^ bigvec[N-5+4]) : bigvec[N-5+2];
  bigvec[N-5+3] = ((bigvec[N-5+3] ^ bigvec[N-5+2]) < thres) ? (bigvec[N-5+3] ^ bigvec[N-5+2]) : bigvec[N-5+3];
  return ((acci == seqacci) && (bigvec == seqvec));
};

double avg_time;

bool lambda_test()
{
  const auto loop_f = [&](const unsigned int i, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i+1] ^ vec[i+2] ^ vec[i+3] ^ vec[i+4] ^ vec[i+5];
    if (d < thres) {
      vec[i+1] = d;
    } else {
      acc.thread_val() += acc0 ^ i;
    }
  };

  const bool test_ok = bench(0, N-5, 1, loop_f, reset_result, test_f, avg_time, bigvec, acci);

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
  const auto loop_f = [&](const SpecLib::ExCommonSpecInfo_t& cs, const unsigned int begin, const unsigned int end, const unsigned int step, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc) {
    for (unsigned int i = begin; (i < end) && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
      mywait(DelaySeconds);
#endif
      const unsigned int d = vec[i+1] ^ vec[i+2] ^ vec[i+3] ^ vec[i+4] ^ vec[i+5];
      if (d < thres) {
        vec[i+1] = d;
      } else {
        acc.thread_val() += acc0 ^ i;
      }
    }
  };

  const bool test_ok = bench(0, N-5, 1, loop_f, reset_result, test_f, avg_time, bigvec, acci);

  unsigned int s_xor = 0;
  for (const unsigned int el : bigvec) {
    s_xor ^= el;
  }
  const unsigned int s_max = *std::max_element(bigvec.begin(), bigvec.end());
  std::cout << "Lambda loop: " << "red_res: " << acci.result()  << " vec_xorsum: " << s_xor << " vec_max: " << s_max << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}


static inline void sf(const unsigned int i, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc)
{
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i+1] ^ vec[i+2] ^ vec[i+3] ^ vec[i+4] ^ vec[i+5];
    if (d < thres) {
      vec[i+1] = d;
    } else {
      acc.thread_val() += acc0 ^ i;
    }
}

bool sf_test()
{
#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(0, N-5, 1, sf, reset_result, test_f, avg_time, bigvec, acci);

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

static inline void sf_loop(const SpecLib::ExCommonSpecInfo_t& cs, const unsigned int begin, const unsigned int end, const unsigned int step, std::vector<unsigned int>& vec, SpecLib::ReductionVar<unsigned int>& acc)
{
  for (unsigned int i = begin; (i < end) && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    const unsigned int d = vec[i+1] ^ vec[i+2] ^ vec[i+3] ^ vec[i+4] ^ vec[i+5];
    if (d < thres) {
      vec[i+1] = d;
    } else {
      acc.thread_val() += acc0 ^ i;
    }
  }
}

bool sf_loop_test()
{
  const bool test_ok = bench(0, N-5, 1, sf_loop, reset_result, test_f, avg_time, bigvec, acci);

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
  N = std::max(5u, N);
  
  auto mt_rand_gen = std::bind(std::uniform_int_distribution<unsigned int>(std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max()), std::mt19937(static_cast<std::mt19937::result_type>(static_cast<unsigned int>(RAND_SEED))));
  
  bigvec.resize(N);
  for (unsigned int i = 0; i < N; i++) {
    bigvec[i] = mt_rand_gen();
  }
  
  vec0 = bigvec;
  
  seqvec = vec0;
  seqacci = 0;
  
  bigvec[0] = ((bigvec[0] ^ bigvec[1] ^ bigvec[2] ^ bigvec[3] ^ bigvec[4]) < thres) ? (bigvec[0] ^ bigvec[1] ^ bigvec[2] ^ bigvec[3] ^ bigvec[4]) : bigvec[0];
  acc0 = bigvec[0];
  acci.set(acc0);
  
  seq_test(seqvec, seqacci);
  
  do_preheat(); // Preheat

  return lambda_test() && lambda_loop_test() && sf_test() && sf_loop_test() ? 0 : -1;
}
