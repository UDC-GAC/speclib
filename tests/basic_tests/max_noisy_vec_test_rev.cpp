/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     max_noisy_vec_test_rev.cpp
/// \brief    Like max_vec_test.cpp but we speculate on the whole vector and it is noisy (reverse iteration)
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <random>
#include <algorithm>
#include <functional>
#include <limits>

constexpr int RAND_SEED = 981;

constexpr long long int DESPL = -500000;

long long int N = 1000;
int MaxSeq;
int *Vals;
std::vector<int> SeqVResult;

template<typename T>
class NoisyVector : public std::vector<T>
{
  static std::atomic<unsigned long> Build;
  static std::atomic<unsigned long> BuildC;
  static std::atomic<unsigned long> AssignC;
  static std::atomic<unsigned long> AssignM;
  static std::atomic<unsigned long> Destruction;

public:

  NoisyVector() : std::vector<T>()
  { Build++; }

  NoisyVector(const NoisyVector& other) : std::vector<T>(other)
  { BuildC++; }

  NoisyVector& operator=(const NoisyVector& other)
  {
    AssignC++;
    std::vector<T>::operator=(other);
    return *this;
  }

  NoisyVector& operator=(NoisyVector&& other)
  {
    AssignM++;
    std::vector<T>::operator=(std::move(other));
    return *this;
  }

  ~NoisyVector()
  { Destruction++; }

  static void ResetStats()
  {
    Build.store(0);
    BuildC.store(0);
    AssignC.store(0);
    AssignM.store(0);
    Destruction.store(0);
  }

  static void DumpStats()
  {
    std::cout << "=========================" << std::endl;
    std::cout << "Default   Builds=" << Build.load() << std::endl;
    std::cout << "Copy      Builds=" << BuildC.load() << std::endl;
    std::cout << "Copy Assignments=" << AssignC.load() << std::endl;
    std::cout << "Move Assignments=" << AssignM.load() << std::endl;
    std::cout << "    Destructions=" << Destruction.load() << std::endl;
    std::cout << "=========================" << std::endl;
  }
};

template<typename T>
std::atomic<unsigned long> NoisyVector<T>::Build{0};
template<typename T>
std::atomic<unsigned long> NoisyVector<T>::BuildC{0};
template<typename T>
std::atomic<unsigned long> NoisyVector<T>::AssignC{0};
template<typename T>
std::atomic<unsigned long> NoisyVector<T>::AssignM{0};
template<typename T>
std::atomic<unsigned long> NoisyVector<T>::Destruction{0};

void seq_test()
{ int max_seq = 0;
  std::vector<int> v_result(N, 0);

  auto tseq_begin = profile_clock_t::now();
  for (long long int i = N-1+DESPL; i >= 0+DESPL; i--) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[i-DESPL] > max_seq) {
      max_seq = Vals[i-DESPL];
    }
    v_result[i-DESPL] = max_seq;
  }
  auto tseq_end = profile_clock_t::now();
  
  MaxSeq = max_seq;
  SeqVResult = v_result;

  std::cout << "Seq   : " << max_seq << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl;
}

int max_spec;
NoisyVector<int> v_result;
double avg_time;

const auto reset_result = [] () { max_spec = 0; std::fill(v_result.begin(), v_result.end(), 0); };
const auto test_f = [] () { return (max_spec == MaxSeq) && (v_result == SeqVResult); };

/// Run test on std::vector using a lambda function
bool lambda_test()
{
  const auto loop_f = [&](const long long int iteration, int& result, NoisyVector<int>& v_result) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    if (Vals[iteration-DESPL] > result) {
      result = Vals[iteration-DESPL];
    }
    v_result[iteration-DESPL] = result;
  };

#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(N-1+DESPL, -1LL+DESPL, -1LL, loop_f, reset_result, test_f, avg_time, max_spec, v_result);

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  std::cout << "Lambda: " << max_spec << " Test_ok=" << (test_ok ? 'Y' : 'N') << std::endl;
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

  v_result.resize(N, 0);
  
  NoisyVector<int>::ResetStats();

  const auto result_ok = lambda_test();

  NoisyVector<int>::DumpStats();

  return result_ok ? 0 : -1;
}
