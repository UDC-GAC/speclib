/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
 
 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     common_test.cpp
/// \brief    Elements that are common to the tests
/// \author   Millan A. Martinez  <millan.alvarez@udc.es>
/// \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
/// \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
/// \author   Francisco F. Rivera <ff.rivera@usc.es>
///

#include <cstdlib>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <array>
#include <chrono>
#include "common_cgetopt.cpp"

using profile_clock_t = std::conditional<(std::chrono::high_resolution_clock::is_steady || !std::chrono::steady_clock::is_steady), std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

template<typename T>
using expr_type = std::remove_cv_t<std::remove_reference_t<T>>;

size_t NChunks = 20;

size_t NThreads = 4;
size_t MinParalThreads = 2;
#ifdef THREADINSTRUMENT
bool Verbose = false;
#endif
#ifdef ENABLE_DELAY
float DelaySeconds = 0.0f;
#endif
#ifdef SLSIMULATE
float SimulatedSuccessRatio = -1.0f;
#endif
size_t NReps = 1;

#ifdef ENABLE_DELAY
static inline void mywait(const float seconds)
{
  if(seconds > 0.f) {
    std::chrono::time_point<profile_clock_t> t1;

    const auto t0 = profile_clock_t::now();
    do {
      t1 = profile_clock_t::now();
    } while (std::chrono::duration<float>(t1 - t0).count() < seconds);
  }
}
#endif

static inline void preheat_pf(const size_t i, std::array<size_t, 100>& arr) {
  size_t Q = i % (100) + 1;
  const size_t aux = arr[Q-1];
  Q = (4 * aux) % (100) + 1;
  arr[Q-1] = aux;
}

static bool check_opt_present(const char *optstr, char ch) {
  size_t i = 0;
  while(optstr[i] != '\0') {
    if (optstr[i] == ch) return true;
    i++;
  }
  return false;
}

template<typename Ti = size_t>
int process_args(int argc, char **argv, const char *optstr, Ti& n, std::string& inputFile, const bool cout_param_info = true, const bool exit_on_help = true)
{
  int c;
  const std::string inputFileDefault = inputFile;
  GetOptClass optClass;
  char *& optarg = optClass.optarg;

  while( (c = optClass.getopt(argc, argv, optstr)) != -1 ) {
    switch (c) {
      case 'c':
        NChunks = (size_t) strtoul(optarg, nullptr, 0);
        break;
#ifdef ENABLE_DELAY
      case 'd':
        DelaySeconds = strtof(optarg, nullptr) / 1000.f;
        break;
#endif
#ifdef SLSIMULATE
      case 's':
        SimulatedSuccessRatio = strtof(optarg, nullptr);
        break;
#endif
      case 'm':
        MinParalThreads = (size_t) strtoul(optarg, nullptr, 0);
        break;
      case 'N':
        NReps = (size_t) strtoul(optarg, nullptr, 0);
        break;
      case 'n':
        if (typeid(Ti) == typeid(int) || typeid(Ti) == typeid(long int) || typeid(Ti) == typeid(short int) || typeid(Ti) == typeid(char)) {
          n = (Ti) strtol(optarg, nullptr, 0);
        } else if (typeid(Ti) == typeid(long long int)) {
          n = (Ti) strtoll(optarg, nullptr, 0);
        } else if (typeid(Ti) == typeid(unsigned int) || typeid(Ti) == typeid(unsigned long int) || typeid(Ti) == typeid(unsigned short int) || typeid(Ti) == typeid(unsigned char)) {
          n = (Ti) strtoul(optarg, nullptr, 0);
        } else if (typeid(Ti) == typeid(unsigned long long int)) {
          n = (Ti) strtoull(optarg, nullptr, 0);
        }
        break;
      case 't':
        NThreads = (size_t) strtoul(optarg, nullptr, 0);
        break;
      case 'i':
        inputFile = optarg;
        break;
#ifdef THREADINSTRUMENT
      case 'v':
        Verbose = true;
        break;
#endif
      case 'h':
        if (check_opt_present(optstr, 'i'))
          std::cout << "-i s    Input file path" << ((inputFileDefault != "") ? (" (default: " + inputFileDefault + ")") : "") << std::endl;
        if (check_opt_present(optstr, 'c'))
          std::cout << "-c n    Number of chunks" << std::endl;
#ifdef ENABLE_DELAY
        if (check_opt_present(optstr, 'd'))
          std::cout << "-d n    Milliseconds of wait" << std::endl;
#endif
        if (check_opt_present(optstr, 'm'))
          std::cout << "-m n    Minimum number of parallel portions in a chunk" << std::endl;
        if (check_opt_present(optstr, 'n'))
          std::cout << "-n n    Number of iterations" << std::endl;
        if (check_opt_present(optstr, 'N'))
          std::cout << "-N n    Number of repetitions of the test" << std::endl;
        if (check_opt_present(optstr, 't'))
          std::cout << "-t n    Number of threads" << std::endl;
#ifdef SLSIMULATE
        if (check_opt_present(optstr, 's'))
          std::cout << "-s n    Simulated ratio of successes" << std::endl;
#endif
#ifdef THREADINSTRUMENT
        if (check_opt_present(optstr, 'v'))
          std::cout << "-v      Verbose" << std::endl;
#endif
        if (check_opt_present(optstr, 'h')) {
          std::cout << "-h      Print this message and exit" << std::endl;
          if (exit_on_help) {
            exit(1);
          } else {
            return 1;
          }
        }
    }
  }
  if (cout_param_info) {
    if (check_opt_present(optstr, 't'))
      std::cout << NThreads << " threads ";
    if (check_opt_present(optstr, 'm'))
      std::cout << MinParalThreads << " min paral threads ";
#ifdef ENABLE_DELAY
    if (check_opt_present(optstr, 'd'))
      std::cout  << DelaySeconds * 1000.f << " delay ms. ";
#endif
    if (check_opt_present(optstr, 'n'))
      std::cout  << n << " iters ";
    if (check_opt_present(optstr, 'c')) {
      std::cout  << NChunks << " chunks ";
      if (check_opt_present(optstr, 'n'))
        std::cout << "(chunksize of " << ((size_t)n / NChunks) << " iters) ";
    }
    if (check_opt_present(optstr, 'N'))
      std::cout << NReps << " reps";
#ifdef SLSIMULATE
    if (check_opt_present(optstr, 'N'))
      std::cout << SimulatedSuccessRatio << " sim.success.rate";
#endif
    std::cout << std::endl;
  }
  return 0;
}
template<typename Ti = size_t>
int process_args(int argc, char **argv, const char *optstr = "hc:d:m:N:n:t:s:v", const bool cout_param_info = true, const bool exit_on_help = true) {
  Ti tmpn = 0;
  std::string tmpstr("");
  return process_args(argc, argv, optstr, tmpn, tmpstr, cout_param_info, exit_on_help);
}
template<typename Ti = size_t>
int process_args(int argc, char **argv, const char *optstr, Ti& n, const bool cout_param_info = true, const bool exit_on_help = true) {
  std::string tmpstr("");
  return process_args(argc, argv, optstr, n, tmpstr, cout_param_info, exit_on_help);
}
template<typename Ti = size_t>
int process_args(int argc, char **argv, const char *optstr, std::string& inputFile, const bool cout_param_info = true, const bool exit_on_help = true) {
  Ti tmpn = 0;
  return process_args(argc, argv, optstr, tmpn, inputFile, cout_param_info, exit_on_help);
}
template<typename Ti = size_t>
int process_args(int argc, char **argv, const char *optstr, std::string& inputFile, Ti& n, const bool cout_param_info = true, const bool exit_on_help = true) {
  return process_args(argc, argv, optstr, n, inputFile, cout_param_info, exit_on_help);
}

#ifndef NOSPECLIB
void do_preheat() {
  const size_t NM = 120000000;
  std::array<size_t, 100> res_arr = {
    388897, 659026, 204145, 727426, 17247, 684665, 194800, 197608, 384051, 608292,
    352171, 40032, 876894, 907020, 438571, 909967, 424277, 129659, 51050, 856832,
    901411, 236116, 609564, 419535, 706712, 221842, 295960, 461806, 839906, 51841,
    803201, 58703, 132088, 120043, 22693, 171178, 953032, 288539, 265937, 822537,
    789237, 262579, 550227, 256335, 94990, 647909, 413353, 612587, 847259, 766064,
    437592, 480924, 977181, 222431, 436767, 916701, 68506, 869258, 773104, 831899,
    877222, 906569, 235990, 909235, 585113, 465456, 526880, 286314, 939897, 919250,
    429209, 585096, 235373, 473366, 81186, 453585, 876082, 162768, 798743, 35812,
    120558, 422476, 608966, 840484, 548059, 958547, 160866, 129506, 487544, 908539,
    957566, 746082, 315236, 14953, 176655, 537965, 725173, 986848, 356309, 168009
  };
#ifdef THREADINSTRUMENT
  ThreadInstrument::LockLog();
#endif
  SpecLib::specRun({NThreads, MinParalThreads}, 0, NM, 1, SpecLib::getChunkSize(NM, NChunks), preheat_pf, res_arr);
#ifdef THREADINSTRUMENT
  ThreadInstrument::UnlockLog();
#endif
}

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
template<typename T>
static size_t count_digit(T number) {
  if ((!std::is_unsigned<expr_type<T>>::value) && (number < static_cast<T>(0))) {
    number = -number;
  }
  size_t count = static_cast<size_t>(0u);
  do {
    number /= static_cast<T>(10);
    count++;
  }while(number >= static_cast<T>(1));
  return count;
}
#endif

#ifdef SLSTATS
void printStatsRunInfoTimings(const SpecLib::StatsRunInfo& statsRes) {
  const std::ios::fmtflags cout_ori_flags = std::cout.flags();
  const decltype(std::cout.precision()) cout_ori_precision = std::cout.precision();
  const double total_measures = statsRes.pt.gwtimeOPi + statsRes.pt.gwtimeOF + statsRes.pt.gwtimeFF + statsRes.pt.gwtimeOPs +
                                statsRes.pt.gwtimeRSs + statsRes.pt.gwtimeOWs + statsRes.pt.gwtimeVV +
                                statsRes.pt.gwtimeOP + statsRes.pt.gwtimeRP + statsRes.pt.gwtimeOW +
                                statsRes.pt.gwtimeW1 + statsRes.pt.gwtimeW3 + statsRes.pt.gwtimeW6;
  std::vector<double> tmpvec = { statsRes.pt.gwtimeOPi, statsRes.pt.gwtimeOF, statsRes.pt.gwtimeFF, statsRes.pt.gwtimeOPs,
                                 statsRes.pt.gwtimeRSs, statsRes.pt.gwtimeOWs, statsRes.pt.gwtimeVV,
                                 statsRes.pt.gwtimeOP, statsRes.pt.gwtimeRP, statsRes.pt.gwtimeOW,
                                 statsRes.pt.gwtimeW1, statsRes.pt.gwtimeW3, statsRes.pt.gwtimeW6 };
  const int fieldWidth = static_cast<int>(count_digit(*std::max_element(tmpvec.begin(), tmpvec.end()))) + 9;

  const double total_time = statsRes.total_exec_time*(double)statsRes.totalNthreads;
  std::cout << "====== Timings =====" << std::endl;
  std::cout << "gwtimeRSs: " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeRSs << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeRSs*100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeRP:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeRP  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeRP *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeFF:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeFF  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeFF *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeVV:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeVV  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeVV *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeW1:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeW1  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeW1 *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeW3:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeW3  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeW3 *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeW6:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeW6  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeW6 *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeOF:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeOF  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeOF *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeOWs: " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeOWs << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeOWs*100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeOW:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeOW  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeOW *100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeOPi: " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeOPi << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeOPi*100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeOPs: " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeOPs << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeOPs*100.0/(total_time) << "%)" << std::endl;
  std::cout << "gwtimeOP:  " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << statsRes.pt.gwtimeOP  << "\t(" << std::setw(5) << std::right << std::setprecision(2) << statsRes.pt.gwtimeOP *100.0/(total_time) << "%)" << std::endl;
  const double errDiff = total_time - total_measures;
  std::cout << "errDiff:   " << std::setw(fieldWidth) << std::right << std::fixed << std::setprecision(8) << errDiff << "\t(" << std::setw(5) << std::right << std::setprecision(2) << errDiff*100.0/(total_time) << "%)" << std::endl;
  std::cout << "====================" << std::endl;
  std::cout.flags(cout_ori_flags);
  std::cout.precision(cout_ori_precision);
}
#endif

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
void printStatsRunInfo(const SpecLib::StatsRunInfo& statsRes, const int n = -1) {
  const std::ios::fmtflags cout_ori_flags = std::cout.flags();
  const decltype(std::cout.precision()) cout_ori_precision = std::cout.precision();
  std::cout << "===== TEST INFO ====" << std::endl;
  if (n >= 0) {
    std::cout << "N: " << n << std::endl;
  }
  std::cout << "NThreads: " << statsRes.totalNthreads << std::endl;
#ifdef SLSTATS
  std::cout << "Total Time: " << statsRes.total_exec_time << std::endl;
  if (statsRes.pt.gwtimeRSs > 0.0) {
    std::cout << "Estimated Speedup: " << std::fixed << std::setprecision(2) << (statsRes.pt.gwtimeRSs / statsRes.total_exec_time) << "x" << std::endl;
    std::cout << "Max Estimated Speedup: " << std::fixed << std::setprecision(2) << ((double)statsRes.totalNthreads / 2.0) * (1.0 - ((statsRes.pt.gwtimeW3 + statsRes.pt.gwtimeW6) / (statsRes.total_exec_time*(double)statsRes.totalNthreads))) << "x" << std::endl;
  }
#endif
  std::cout << "--------------------" << std::endl;
  if (statsRes.total > 0) {
    const int fieldWidth = static_cast<int>(count_digit(statsRes.total));
    const int percWidht = (statsRes.successes < statsRes.total) ? ((statsRes.failures < statsRes.total) ? (((statsRes.sequential < statsRes.total) ? 5 : 6)) : 6) : 6;
    std::cout << "successes:  " << std::setw(fieldWidth) << std::right << statsRes.successes << "\t(" << std::setw(percWidht) << std::right << std::fixed << std::setprecision(2) << static_cast<double>(statsRes.successes)*100.0/static_cast<double>(statsRes.total) << "%)" << std::endl;
    std::cout << "failures:   " << std::setw(fieldWidth) << std::right << statsRes.failures << "\t(" << std::setw(percWidht) << std::right << std::fixed << std::setprecision(2) << static_cast<double>(statsRes.failures)*100.0/static_cast<double>(statsRes.total) << "%)" << std::endl;
    std::cout << "sequential: " << std::setw(fieldWidth) << std::right << statsRes.sequential << "\t(" << std::setw(percWidht) << std::right << std::fixed << std::setprecision(2) << static_cast<double>(statsRes.sequential)*100.0/static_cast<double>(statsRes.total) << "%)" << std::endl;
    std::cout << "TOTAL:      " << std::setw(fieldWidth) << std::right << statsRes.total << std::endl;
  } else {
    std::cout << "successes:  " << std::setw(1) << std::right << statsRes.successes << std::endl;
    std::cout << "failures:   " << std::setw(1) << std::right << statsRes.failures << std::endl;
    std::cout << "sequential: " << std::setw(1) << std::right << statsRes.sequential << std::endl;
    std::cout << "TOTAL:      " << std::setw(1) << std::right << statsRes.total << std::endl;
  }
  std::cout << "====================" << std::endl;
  std::cout.flags(cout_ori_flags);
  std::cout.precision(cout_ori_precision);
#ifdef SLSTATS
  printStatsRunInfoTimings(statsRes);
#endif
}
#endif

template<typename Ti, typename FRun, typename FReset, typename FTest, typename... ArgT>
bool bench(const typename std::remove_reference<Ti>::type begin, const Ti end, const typename std::remove_reference<Ti>::type step, const FRun& f_run, const FReset& f_reset, const FTest& f_test, double& avg_time , ArgT&&... args)
{ size_t i;
  bool test_ok = true;
  const size_t calcChunk = SpecLib::getChunkSize(static_cast<size_t>((step >= 0) ? ((end - begin + step - 1) / step) : ((end - begin + step + 1) / step)), NChunks);
  avg_time = 0.f;
#if defined(SLSTATS) || defined(SLMINIMALSTATS)
  SpecLib::StatsRunInfo statsRes;
#endif
  for (i = 0; (i < NReps) && test_ok; i++) {
    f_reset();
    auto tpar_begin = profile_clock_t::now();

#if !defined(SLSTATS) && !defined(SLMINIMALSTATS)
#ifdef SLSIMULATE
    SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, begin, end, step, calcChunk, f_run, std::forward<ArgT>(args)...);
#else
    SpecLib::specRun({NThreads, MinParalThreads}, begin, end, step, calcChunk, f_run, std::forward<ArgT>(args)...);
#endif
#else
#ifdef SLSIMULATE
    SpecLib::StatsRunInfo tmpStatsRes = SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, begin, end, step, calcChunk, f_run, std::forward<ArgT>(args)...);
#else
    SpecLib::StatsRunInfo tmpStatsRes = SpecLib::specRun({NThreads, MinParalThreads}, begin, end, step, calcChunk, f_run, std::forward<ArgT>(args)...);
#endif
    statsRes += tmpStatsRes;
#endif

    auto tpar_end = profile_clock_t::now();
    avg_time += std::chrono::duration<double>(tpar_end - tpar_begin).count();

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
    printStatsRunInfo(statsRes);
#endif

    test_ok = f_test();
  }
  
  avg_time /= static_cast<double>(i);

  return test_ok;

}
#endif
