/*
 SpecLib: Library for speculative execution of loops
 Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna

 Distributed under the MIT License. (See accompanying file LICENSE)
*/

///
/// \file     atomicreal_test.cpp
/// \brief    Test on support of SpecAtomic, SpecReal and SpecRealInd
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

#ifndef NOLONGDOUBLE
  using ldouble = long double;
#else
  using ldouble = double;
#endif

constexpr int RAND_SEED = 981;

constexpr float epsAbsF = 10.1f;
constexpr double epsAbsD = 10.1;
constexpr ldouble epsAbsL = 10.1L;
constexpr float epsRelF = 0.001f;
constexpr double epsRelD = 0.001;
constexpr ldouble epsRelL = 0.001L;
constexpr size_t epsUps = 200;

using SpecRealAbsF = typename SpecLib::SpecReal<float, SpecLib::EPS_ABSOLUTE>;
using SpecRealAbsD = typename SpecLib::SpecReal<double, SpecLib::EPS_ABSOLUTE>;
using SpecRealAbsL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ABSOLUTE>;
using SpecRealIndAbsF = typename SpecLib::SpecReal<float, SpecLib::EPS_ABSOLUTE>;
using SpecRealIndAbsD = typename SpecLib::SpecReal<double, SpecLib::EPS_ABSOLUTE>;
using SpecRealIndAbsL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ABSOLUTE>;

using SpecRealRelF = typename SpecLib::SpecReal<float, SpecLib::EPS_RELATIVE>;
using SpecRealRelD = typename SpecLib::SpecReal<double, SpecLib::EPS_RELATIVE>;
using SpecRealRelL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_RELATIVE>;
using SpecRealIndRelF = typename SpecLib::SpecReal<float, SpecLib::EPS_RELATIVE>;
using SpecRealIndRelD = typename SpecLib::SpecReal<double, SpecLib::EPS_RELATIVE>;
using SpecRealIndRelL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_RELATIVE>;

using SpecRealUlpF = typename SpecLib::SpecReal<float, SpecLib::EPS_ULP>;
using SpecRealUlpD = typename SpecLib::SpecReal<double, SpecLib::EPS_ULP>;
using SpecRealUlpL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ULP>;
using SpecRealIndUlpF = typename SpecLib::SpecReal<float, SpecLib::EPS_ULP>;
using SpecRealIndUlpD = typename SpecLib::SpecReal<double, SpecLib::EPS_ULP>;
using SpecRealIndUlpL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ULP>;

using SpecRealAbsRelF = typename SpecLib::SpecReal<float, SpecLib::EPS_ABSOLUTE_AND_RELATIVE>;
using SpecRealAbsRelD = typename SpecLib::SpecReal<double, SpecLib::EPS_ABSOLUTE_AND_RELATIVE>;
using SpecRealAbsRelL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ABSOLUTE_AND_RELATIVE>;
using SpecRealIndAbsRelF = typename SpecLib::SpecReal<float, SpecLib::EPS_ABSOLUTE_AND_RELATIVE>;
using SpecRealIndAbsRelD = typename SpecLib::SpecReal<double, SpecLib::EPS_ABSOLUTE_AND_RELATIVE>;
using SpecRealIndAbsRelL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ABSOLUTE_AND_RELATIVE>;

using SpecRealAbsUlpF = typename SpecLib::SpecReal<float, SpecLib::EPS_ABSOLUTE_AND_ULP>;
using SpecRealAbsUlpD = typename SpecLib::SpecReal<double, SpecLib::EPS_ABSOLUTE_AND_ULP>;
using SpecRealAbsUlpL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ABSOLUTE_AND_ULP>;
using SpecRealIndAbsUlpF = typename SpecLib::SpecReal<float, SpecLib::EPS_ABSOLUTE_AND_ULP>;
using SpecRealIndAbsUlpD = typename SpecLib::SpecReal<double, SpecLib::EPS_ABSOLUTE_AND_ULP>;
using SpecRealIndAbsUlpL = typename SpecLib::SpecReal<ldouble, SpecLib::EPS_ABSOLUTE_AND_ULP>;

size_t N = 1000;
float *Vals;

char sres0_c = static_cast<char>(0); unsigned char sres0_uc = static_cast<unsigned char>(0u); int sres0_i = 0; unsigned int sres0_u = 0U; long long int sres0_ll = 0LL; unsigned long long int sres0_ull = 0ULL;
float sres2_f = 0.0f; double sres2_d = 0.0; ldouble sres2_l = 0.0L; float sres2i_f = 0.0f; double sres2i_d = 0.0; ldouble sres2i_l = 0.0L;
float sres3_f = 0.0f; double sres3_d = 0.0; ldouble sres3_l = 0.0L; float sres3i_f = 0.0f; double sres3i_d = 0.0; ldouble sres3i_l = 0.0L;
float sres4_f = 0.0f; double sres4_d = 0.0; ldouble sres4_l = 0.0L; float sres4i_f = 0.0f; double sres4i_d = 0.0; ldouble sres4i_l = 0.0L;
float sres5_f = 0.0f; double sres5_d = 0.0; ldouble sres5_l = 0.0L; float sres5i_f = 0.0f; double sres5i_d = 0.0; ldouble sres5i_l = 0.0L;
float sres6_f = 0.0f; double sres6_d = 0.0; ldouble sres6_l = 0.0L; float sres6i_f = 0.0f; double sres6i_d = 0.0; ldouble sres6i_l = 0.0L;

void seq_test()
{
  const auto tseq_begin = profile_clock_t::now();
  for (size_t i = 0; i < N; i++) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    sres0_c += static_cast<char>(Vals[i]); sres0_uc += static_cast<unsigned char>(Vals[i]); sres0_i += static_cast<int>(Vals[i]);
    sres0_u += static_cast<unsigned int>(Vals[i]); sres0_ll += static_cast<long long int>(Vals[i]); sres0_ull += static_cast<unsigned long long int>(Vals[i]);
    sres2_f += static_cast<float>(Vals[i]); sres2_d += static_cast<double>(Vals[i]); sres2_l += static_cast<ldouble>(Vals[i]);
    sres2i_f += static_cast<float>(Vals[i]); sres2i_d += static_cast<double>(Vals[i]); sres2i_l += static_cast<ldouble>(Vals[i]);
    sres3_f += static_cast<float>(Vals[i]); sres3_d += static_cast<double>(Vals[i]); sres3_l += static_cast<ldouble>(Vals[i]);
    sres3i_f += static_cast<float>(Vals[i]); sres3i_d += static_cast<double>(Vals[i]); sres3i_l += static_cast<ldouble>(Vals[i]);
    sres4_f += static_cast<float>(Vals[i]); sres4_d += static_cast<double>(Vals[i]); sres4_l += static_cast<ldouble>(Vals[i]);
    sres4i_f += static_cast<float>(Vals[i]); sres4i_d += static_cast<double>(Vals[i]); sres4i_l += static_cast<ldouble>(Vals[i]);
    sres5_f += static_cast<float>(Vals[i]); sres5_d += static_cast<double>(Vals[i]); sres5_l += static_cast<ldouble>(Vals[i]);
    sres5i_f += static_cast<float>(Vals[i]); sres5i_d += static_cast<double>(Vals[i]); sres5i_l += static_cast<ldouble>(Vals[i]);
    sres6_f += static_cast<float>(Vals[i]); sres6_d += static_cast<double>(Vals[i]); sres6_l += static_cast<ldouble>(Vals[i]);
    sres6i_f += static_cast<float>(Vals[i]); sres6i_d += static_cast<double>(Vals[i]); sres6i_l += static_cast<ldouble>(Vals[i]);
  }
  const auto tseq_end = profile_clock_t::now();
  
  long long int sum1 = static_cast<long long int>(sres0_c) + static_cast<long long int>(sres0_uc) + static_cast<long long int>(sres0_i) + static_cast<long long int>(sres0_u) + static_cast<long long int>(sres0_ll) + static_cast<long long int>(sres0_ull);
  ldouble sum2 = static_cast<ldouble>(sres2_f) + static_cast<ldouble>(sres2_d) + static_cast<ldouble>(sres2_l) + static_cast<ldouble>(sres2i_f) + static_cast<ldouble>(sres2i_d) + static_cast<ldouble>(sres2i_l) +
                     static_cast<ldouble>(sres3_f) + static_cast<ldouble>(sres3_d) + static_cast<ldouble>(sres3_l) + static_cast<ldouble>(sres3i_f) + static_cast<ldouble>(sres3i_d) + static_cast<ldouble>(sres3i_l) +
                     static_cast<ldouble>(sres4_f) + static_cast<ldouble>(sres4_d) + static_cast<ldouble>(sres4_l) + static_cast<ldouble>(sres4i_f) + static_cast<ldouble>(sres4i_d) + static_cast<ldouble>(sres4i_l) +
                     static_cast<ldouble>(sres5_f) + static_cast<ldouble>(sres5_d) + static_cast<ldouble>(sres5_l) + static_cast<ldouble>(sres5i_f) + static_cast<ldouble>(sres5i_d) + static_cast<ldouble>(sres5i_l) +
                     static_cast<ldouble>(sres6_f) + static_cast<ldouble>(sres6_d) + static_cast<ldouble>(sres6_l) + static_cast<ldouble>(sres6i_f) + static_cast<ldouble>(sres6i_d) + static_cast<ldouble>(sres6i_l);

  std::cout << "Seq   : " << "sum1: " << sum1 << " sum2: " << sum2 << std::endl;
  std::cout << "Time  : " << std::chrono::duration<double>(tseq_end - tseq_begin).count() << std::endl << std::endl;
}

SpecLib::SpecAtomic<char> res0_c; SpecLib::SpecAtomic<unsigned char> res0_uc; SpecLib::SpecAtomic<int> res0_i;
SpecLib::SpecAtomic<unsigned int> res0_u; SpecLib::SpecAtomic<long long int> res0_ll; SpecLib::SpecAtomic<unsigned long long int> res0_ull;
SpecLib::SpecAtomic<SpecRealAbsF> res2_f; SpecLib::SpecAtomic<SpecRealAbsD> res2_d; SpecLib::SpecAtomic<SpecRealAbsL> res2_l;
SpecLib::SpecAtomic<SpecRealIndAbsF> res2i_f; SpecLib::SpecAtomic<SpecRealIndAbsD> res2i_d; SpecLib::SpecAtomic<SpecRealIndAbsL> res2i_l;
SpecLib::SpecAtomic<SpecRealRelF> res3_f; SpecLib::SpecAtomic<SpecRealRelD> res3_d; SpecLib::SpecAtomic<SpecRealRelL> res3_l;
SpecLib::SpecAtomic<SpecRealIndRelF> res3i_f; SpecLib::SpecAtomic<SpecRealIndRelD> res3i_d; SpecLib::SpecAtomic<SpecRealIndRelL> res3i_l;
SpecLib::SpecAtomic<SpecRealUlpF> res4_f; SpecLib::SpecAtomic<SpecRealUlpD> res4_d; SpecLib::SpecAtomic<SpecRealUlpL> res4_l;
SpecLib::SpecAtomic<SpecRealIndUlpF> res4i_f; SpecLib::SpecAtomic<SpecRealIndUlpD> res4i_d; SpecLib::SpecAtomic<SpecRealIndUlpL> res4i_l;
SpecLib::SpecAtomic<SpecRealAbsRelF> res5_f; SpecLib::SpecAtomic<SpecRealAbsRelD> res5_d; SpecLib::SpecAtomic<SpecRealAbsRelL> res5_l;
SpecLib::SpecAtomic<SpecRealIndAbsRelF> res5i_f; SpecLib::SpecAtomic<SpecRealIndAbsRelD> res5i_d; SpecLib::SpecAtomic<SpecRealIndAbsRelL> res5i_l;
SpecLib::SpecAtomic<SpecRealAbsUlpF> res6_f; SpecLib::SpecAtomic<SpecRealAbsUlpD> res6_d; SpecLib::SpecAtomic<SpecRealAbsUlpL> res6_l;
SpecLib::SpecAtomic<SpecRealIndAbsUlpF> res6i_f; SpecLib::SpecAtomic<SpecRealIndAbsUlpD> res6i_d; SpecLib::SpecAtomic<SpecRealIndAbsUlpL> res6i_l;

double avg_time;

const auto reset_result = [] () {
  res2_f.load().setPrecisionThreshold(epsAbsF); res2_d.load().setPrecisionThreshold(epsAbsD); res2_l.load().setPrecisionThreshold(epsAbsL); res2i_f.load().setPrecisionThreshold(epsAbsF); res2i_d.load().setPrecisionThreshold(epsAbsD); res2i_l.load().setPrecisionThreshold(epsAbsL);
  res3_f.load().setPrecisionThreshold(epsRelF); res3_d.load().setPrecisionThreshold(epsRelD); res3_l.load().setPrecisionThreshold(epsRelL); res3i_f.load().setPrecisionThreshold(epsRelF); res3i_d.load().setPrecisionThreshold(epsRelD); res3i_l.load().setPrecisionThreshold(epsRelL);
  res4_f.load().setPrecisionThreshold(static_cast<SpecRealUlpF::ulp_type>(epsUps)); res4_d.load().setPrecisionThreshold(static_cast<SpecRealUlpD::ulp_type>(epsUps)); res4_l.load().setPrecisionThreshold(static_cast<SpecRealUlpL::ulp_type>(epsUps)); res4i_f.load().setPrecisionThreshold(static_cast<SpecRealIndUlpF::ulp_type>(epsUps)); res4i_d.load().setPrecisionThreshold(static_cast<SpecRealIndUlpD::ulp_type>(epsUps)); res4i_l.load().setPrecisionThreshold(static_cast<SpecRealIndUlpL::ulp_type>(epsUps));
  res5_f.load().setPrecisionThreshold(epsAbsF, epsRelF); res5_d.load().setPrecisionThreshold(epsAbsD, epsRelD); res5_l.load().setPrecisionThreshold(epsAbsL, epsRelL); res5i_f.load().setPrecisionThreshold(epsAbsF, epsRelF); res5i_d.load().setPrecisionThreshold(epsAbsD, epsRelD); res5i_l.load().setPrecisionThreshold(epsAbsL, epsRelL);
  res6_f.load().setPrecisionThreshold(epsAbsF, static_cast<SpecRealAbsUlpF::ulp_type>(epsUps)); res6_d.load().setPrecisionThreshold(epsAbsD, static_cast<SpecRealAbsUlpD::ulp_type>(epsUps)); res6_l.load().setPrecisionThreshold(epsAbsL, static_cast<SpecRealAbsUlpL::ulp_type>(epsUps)); res6i_f.load().setPrecisionThreshold(epsAbsF, static_cast<SpecRealIndAbsUlpF::ulp_type>(epsUps)); res6i_d.load().setPrecisionThreshold(epsAbsD, static_cast<SpecRealIndAbsUlpD::ulp_type>(epsUps)); res6i_l.load().setPrecisionThreshold(epsAbsL, static_cast<SpecRealIndAbsUlpL::ulp_type>(epsUps));
  res0_c = static_cast<char>(0); res0_uc = static_cast<unsigned char>(0u); res0_i = 0; res0_u = 0U; res0_ll = 0LL; res0_ull = 0ULL;
  res2_f = 0.0f; res2_d = 0.0; res2_l = 0.0L; res2i_f = 0.0f; res2i_d = 0.0; res2i_l = 0.0L;
  res3_f = 0.0f; res3_d = 0.0; res3_l = 0.0L; res3i_f = 0.0f; res3i_d = 0.0; res3i_l = 0.0L;
  res4_f = 0.0f; res4_d = 0.0; res4_l = 0.0L; res4i_f = 0.0f; res4i_d = 0.0; res4i_l = 0.0L;
  res5_f = 0.0f; res5_d = 0.0; res5_l = 0.0L; res5i_f = 0.0f; res5i_d = 0.0; res5i_l = 0.0L;
  res6_f = 0.0f; res6_d = 0.0; res6_l = 0.0L; res6i_f = 0.0f; res6i_d = 0.0; res6i_l = 0.0L;
};
const auto test_f = [] () {
  res2_f.load().setPrecisionThreshold(epsAbsF*static_cast<float>(NChunks+1)); res2_d.load().setPrecisionThreshold(epsAbsD*static_cast<double>(NChunks+1)); res2_l.load().setPrecisionThreshold(epsAbsL*static_cast<ldouble>(NChunks+1)); res2i_f.load().setPrecisionThreshold(epsAbsF*static_cast<float>(NChunks+1)); res2i_d.load().setPrecisionThreshold(epsAbsD*static_cast<double>(NChunks+1)); res2i_l.load().setPrecisionThreshold(epsAbsL*static_cast<ldouble>(NChunks+1));
  res3_f.load().setPrecisionThreshold(epsRelF*static_cast<float>(NChunks+1)); res3_d.load().setPrecisionThreshold(epsRelD*static_cast<double>(NChunks+1)); res3_l.load().setPrecisionThreshold(epsRelL*static_cast<ldouble>(NChunks+1)); res3i_f.load().setPrecisionThreshold(epsRelF*static_cast<float>(NChunks+1)); res3i_d.load().setPrecisionThreshold(epsRelD*static_cast<double>(NChunks+1)); res3i_l.load().setPrecisionThreshold(epsRelL*static_cast<ldouble>(NChunks+1));
  res4_f.load().setPrecisionThreshold(static_cast<SpecRealUlpF::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res4_d.load().setPrecisionThreshold(static_cast<SpecRealUlpD::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res4_l.load().setPrecisionThreshold(static_cast<SpecRealUlpL::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res4i_f.load().setPrecisionThreshold(static_cast<SpecRealIndUlpF::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res4i_d.load().setPrecisionThreshold(static_cast<SpecRealIndUlpD::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res4i_l.load().setPrecisionThreshold(static_cast<SpecRealIndUlpL::ulp_type>(epsUps*static_cast<size_t>(NChunks+1)));
  res5_f.load().setPrecisionThreshold(epsAbsF*static_cast<float>(NChunks+1), epsRelF*static_cast<float>(NChunks+1)); res5_d.load().setPrecisionThreshold(epsAbsD*static_cast<double>(NChunks+1), epsRelD*static_cast<double>(NChunks+1)); res5_l.load().setPrecisionThreshold(epsAbsL*static_cast<ldouble>(NChunks+1), epsRelL*static_cast<ldouble>(NChunks+1)); res5i_f.load().setPrecisionThreshold(epsAbsF*static_cast<float>(NChunks+1), epsRelF*static_cast<float>(NChunks+1)); res5i_d.load().setPrecisionThreshold(epsAbsD*static_cast<double>(NChunks+1), epsRelD*static_cast<double>(NChunks+1)); res5i_l.load().setPrecisionThreshold(epsAbsL*static_cast<ldouble>(NChunks+1), epsRelL*static_cast<ldouble>(NChunks+1));
  res6_f.load().setPrecisionThreshold(epsAbsF*static_cast<float>(NChunks+1), static_cast<SpecRealAbsUlpF::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res6_d.load().setPrecisionThreshold(epsAbsD*static_cast<double>(NChunks+1), static_cast<SpecRealAbsUlpD::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res6_l.load().setPrecisionThreshold(epsAbsL*static_cast<ldouble>(NChunks+1), static_cast<SpecRealAbsUlpL::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res6i_f.load().setPrecisionThreshold(epsAbsF*static_cast<float>(NChunks+1), static_cast<SpecRealIndAbsUlpF::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res6i_d.load().setPrecisionThreshold(epsAbsD*static_cast<double>(NChunks+1), static_cast<SpecRealIndAbsUlpD::ulp_type>(epsUps*static_cast<size_t>(NChunks+1))); res6i_l.load().setPrecisionThreshold(epsAbsL*static_cast<ldouble>(NChunks+1), static_cast<SpecRealIndAbsUlpL::ulp_type>(epsUps*static_cast<size_t>(NChunks+1)));
  const bool bool0 = res0_c.load() == sres0_c && res0_uc.load() == sres0_uc && res0_i.load() == sres0_i && res0_u.load() == sres0_u && res0_ll.load() == sres0_ll && res0_ull.load() == sres0_ull;
  const bool bool2 = res2_f.load() == sres2_f && res2_d.load() == sres2_d && res2_l.load() == sres2_l && res2i_f.load() == sres2i_f && res2i_d.load() == sres2i_d && res2i_l.load() == sres2i_l;
  const bool bool3 = res3_f.load() == sres3_f && res3_d.load() == sres3_d && res3_l.load() == sres3_l && res3i_f.load() == sres3i_f && res3i_d.load() == sres3i_d && res3i_l.load() == sres3i_l;
  const bool bool4 = res4_f.load() == sres4_f && res4_d.load() == sres4_d && res4_l.load() == sres4_l && res4i_f.load() == sres4i_f && res4i_d.load() == sres4i_d && res4i_l.load() == sres4i_l;
  const bool bool5 = res5_f.load() == sres5_f && res5_d.load() == sres5_d && res5_l.load() == sres5_l && res5i_f.load() == sres5i_f && res5i_d.load() == sres5i_d && res5i_l.load() == sres5i_l;
  const bool bool6 = res6_f.load() == sres6_f && res6_d.load() == sres6_d && res6_l.load() == sres6_l && res6i_f.load() == sres6i_f && res6i_d.load() == sres6i_d && res6i_l.load() == sres6i_l;
  return(bool0 && bool2 && bool3 && bool4 && bool5 && bool6);
};

bool lambda_test()
{
  const auto loop_f = [&](const size_t i, SpecLib::SpecAtomic<char>& res0_c, SpecLib::SpecAtomic<unsigned char>& res0_uc, SpecLib::SpecAtomic<int>& res0_i, SpecLib::SpecAtomic<unsigned int>& res0_u, SpecLib::SpecAtomic<long long int>& res0_ll, SpecLib::SpecAtomic<unsigned long long int>& res0_ull,
                                          SpecLib::SpecAtomic<SpecRealAbsF>& res2_f, SpecLib::SpecAtomic<SpecRealAbsD>& res2_d, SpecLib::SpecAtomic<SpecRealAbsL>& res2_l, SpecLib::SpecAtomic<SpecRealIndAbsF>& res2i_f, SpecLib::SpecAtomic<SpecRealIndAbsD>& res2i_d, SpecLib::SpecAtomic<SpecRealIndAbsL>& res2i_l,
                                          SpecLib::SpecAtomic<SpecRealRelF>& res3_f, SpecLib::SpecAtomic<SpecRealRelD>& res3_d, SpecLib::SpecAtomic<SpecRealRelL>& res3_l, SpecLib::SpecAtomic<SpecRealIndRelF>& res3i_f, SpecLib::SpecAtomic<SpecRealIndRelD>& res3i_d, SpecLib::SpecAtomic<SpecRealIndRelL>& res3i_l,
                                          SpecLib::SpecAtomic<SpecRealUlpF>& res4_f, SpecLib::SpecAtomic<SpecRealUlpD>& res4_d, SpecLib::SpecAtomic<SpecRealUlpL>& res4_l, SpecLib::SpecAtomic<SpecRealIndUlpF>& res4i_f, SpecLib::SpecAtomic<SpecRealIndUlpD>& res4i_d, SpecLib::SpecAtomic<SpecRealIndUlpL>& res4i_l,
                                          SpecLib::SpecAtomic<SpecRealAbsRelF>& res5_f, SpecLib::SpecAtomic<SpecRealAbsRelD>& res5_d, SpecLib::SpecAtomic<SpecRealAbsRelL>& res5_l, SpecLib::SpecAtomic<SpecRealIndAbsRelF>& res5i_f, SpecLib::SpecAtomic<SpecRealIndAbsRelD>& res5i_d, SpecLib::SpecAtomic<SpecRealIndAbsRelL>& res5i_l,
                                          SpecLib::SpecAtomic<SpecRealAbsUlpF>& res6_f, SpecLib::SpecAtomic<SpecRealAbsUlpD>& res6_d, SpecLib::SpecAtomic<SpecRealAbsUlpL>& res6_l, SpecLib::SpecAtomic<SpecRealIndAbsUlpF>& res6i_f, SpecLib::SpecAtomic<SpecRealIndAbsUlpD>& res6i_d, SpecLib::SpecAtomic<SpecRealIndAbsUlpL>& res6i_l
                                               ) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
//  overloaded operators '+=' and '+' from 'SpecAtomic' class uses 'std::memory_order_seq_cst' by default
//  res0_c += static_cast<char>(Vals[i]); res0_uc += static_cast<unsigned char>(Vals[i]); res0_i += static_cast<int>(Vals[i]);
//  res0_u += static_cast<unsigned int>(Vals[i]); res0_ll += static_cast<long long int>(Vals[i]); res0_ull += static_cast<unsigned long long int>(Vals[i]);
//  res2_f += static_cast<float>(Vals[i]); res2_d += static_cast<double>(Vals[i]); res2_l += static_cast<ldouble>(Vals[i]);
//  res2i_f += static_cast<float>(Vals[i]); res2i_d += static_cast<double>(Vals[i]); res2i_l += static_cast<ldouble>(Vals[i]);
//  res3_f += static_cast<float>(Vals[i]); res3_d += static_cast<double>(Vals[i]); res3_l += static_cast<ldouble>(Vals[i]);
//  res3i_f += static_cast<float>(Vals[i]); res3i_d += static_cast<double>(Vals[i]); res3i_l += static_cast<ldouble>(Vals[i]);
//  res4_f += static_cast<float>(Vals[i]); res4_d += static_cast<double>(Vals[i]); res4_l += static_cast<ldouble>(Vals[i]);
//  res4i_f += static_cast<float>(Vals[i]); res4i_d += static_cast<double>(Vals[i]); res4i_l += static_cast<ldouble>(Vals[i]);
//  res5_f += static_cast<float>(Vals[i]); res5_d += static_cast<double>(Vals[i]); res5_l += static_cast<ldouble>(Vals[i]);
//  res5i_f += static_cast<float>(Vals[i]); res5i_d += static_cast<double>(Vals[i]); res5i_l += static_cast<ldouble>(Vals[i]);
//  res6_f += static_cast<float>(Vals[i]); res6_d += static_cast<double>(Vals[i]); res6_l += static_cast<ldouble>(Vals[i]);
//  res6i_f += static_cast<float>(Vals[i]); res6i_d += static_cast<double>(Vals[i]); res6i_l += static_cast<ldouble>(Vals[i]);
    res0_c.fetch_add(static_cast<char>(Vals[i]), std::memory_order_relaxed); res0_uc.fetch_add(static_cast<unsigned char>(Vals[i]), std::memory_order_relaxed); res0_i.fetch_add(static_cast<int>(Vals[i]), std::memory_order_relaxed);
    res0_u.fetch_add(static_cast<unsigned int>(Vals[i]), std::memory_order_relaxed); res0_ll.fetch_add(static_cast<long long int>(Vals[i]), std::memory_order_relaxed); res0_ull.fetch_add(static_cast<unsigned long long int>(Vals[i]), std::memory_order_relaxed);
    res2_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res2i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res3_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res3i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res4_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res4i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res5_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res5i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res6_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res6i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  };

#ifdef THREADINSTRUMENT
  ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

  const bool test_ok = bench(0, N, 1, loop_f, reset_result, test_f, avg_time, res0_c, res0_uc, res0_i, res0_u, res0_ll, res0_ull,
                                                                              res2_f, res2_d, res2_l, res2i_f, res2i_d, res2i_l,
                                                                              res3_f, res3_d, res3_l, res3i_f, res3i_d, res3i_l,
                                                                              res4_f, res4_d, res4_l, res4i_f, res4i_d, res4i_l,
                                                                              res5_f, res5_d, res5_l, res5i_f, res5i_d, res5i_l,
                                                                              res6_f, res6_d, res6_l, res6i_f, res6i_d, res6i_l);

#ifdef THREADINSTRUMENT
  ThreadInstrument::dumpLog("tilog.log");
#endif

  long long int sum1 = static_cast<long long int>(res0_c.load()) + static_cast<long long int>(res0_uc.load()) + static_cast<long long int>(res0_i.load()) + static_cast<long long int>(res0_u.load()) + static_cast<long long int>(res0_ll.load()) + static_cast<long long int>(res0_ull.load());
  ldouble sum2 = static_cast<ldouble>(res2_f.load()) + static_cast<ldouble>(res2_d.load()) + static_cast<ldouble>(res2_l.load()) + static_cast<ldouble>(res2i_f.load()) + static_cast<ldouble>(res2i_d.load()) + static_cast<ldouble>(res2i_l.load()) +
                     static_cast<ldouble>(res3_f.load()) + static_cast<ldouble>(res3_d.load()) + static_cast<ldouble>(res3_l.load()) + static_cast<ldouble>(res3i_f.load()) + static_cast<ldouble>(res3i_d.load()) + static_cast<ldouble>(res3i_l.load()) +
                     static_cast<ldouble>(res4_f.load()) + static_cast<ldouble>(res4_d.load()) + static_cast<ldouble>(res4_l.load()) + static_cast<ldouble>(res4i_f.load()) + static_cast<ldouble>(res4i_d.load()) + static_cast<ldouble>(res4i_l.load()) +
                     static_cast<ldouble>(res5_f.load()) + static_cast<ldouble>(res5_d.load()) + static_cast<ldouble>(res5_l.load()) + static_cast<ldouble>(res5i_f.load()) + static_cast<ldouble>(res5i_d.load()) + static_cast<ldouble>(res5i_l.load()) +
                     static_cast<ldouble>(res6_f.load()) + static_cast<ldouble>(res6_d.load()) + static_cast<ldouble>(res6_l.load()) + static_cast<ldouble>(res6i_f.load()) + static_cast<ldouble>(res6i_d.load()) + static_cast<ldouble>(res6i_l.load());

  std::cout << "Lambda: " << "sum1: " << sum1 << " sum2: " << sum2 << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

bool lambda_loop_test()
{
  const auto loop_f = [&](const SpecLib::ExCommonSpecInfo_t& cs, const size_t begin, const size_t end, const size_t step, SpecLib::SpecAtomic<char>& res0_c, SpecLib::SpecAtomic<unsigned char>& res0_uc, SpecLib::SpecAtomic<int>& res0_i, SpecLib::SpecAtomic<unsigned int>& res0_u, SpecLib::SpecAtomic<long long int>& res0_ll, SpecLib::SpecAtomic<unsigned long long int>& res0_ull,
                                                                                                                          SpecLib::SpecAtomic<SpecRealAbsF>& res2_f, SpecLib::SpecAtomic<SpecRealAbsD>& res2_d, SpecLib::SpecAtomic<SpecRealAbsL>& res2_l, SpecLib::SpecAtomic<SpecRealIndAbsF>& res2i_f, SpecLib::SpecAtomic<SpecRealIndAbsD>& res2i_d, SpecLib::SpecAtomic<SpecRealIndAbsL>& res2i_l,
                                                                                                                          SpecLib::SpecAtomic<SpecRealRelF>& res3_f, SpecLib::SpecAtomic<SpecRealRelD>& res3_d, SpecLib::SpecAtomic<SpecRealRelL>& res3_l, SpecLib::SpecAtomic<SpecRealIndRelF>& res3i_f, SpecLib::SpecAtomic<SpecRealIndRelD>& res3i_d, SpecLib::SpecAtomic<SpecRealIndRelL>& res3i_l,
                                                                                                                          SpecLib::SpecAtomic<SpecRealUlpF>& res4_f, SpecLib::SpecAtomic<SpecRealUlpD>& res4_d, SpecLib::SpecAtomic<SpecRealUlpL>& res4_l, SpecLib::SpecAtomic<SpecRealIndUlpF>& res4i_f, SpecLib::SpecAtomic<SpecRealIndUlpD>& res4i_d, SpecLib::SpecAtomic<SpecRealIndUlpL>& res4i_l,
                                                                                                                          SpecLib::SpecAtomic<SpecRealAbsRelF>& res5_f, SpecLib::SpecAtomic<SpecRealAbsRelD>& res5_d, SpecLib::SpecAtomic<SpecRealAbsRelL>& res5_l, SpecLib::SpecAtomic<SpecRealIndAbsRelF>& res5i_f, SpecLib::SpecAtomic<SpecRealIndAbsRelD>& res5i_d, SpecLib::SpecAtomic<SpecRealIndAbsRelL>& res5i_l,
                                                                                                                          SpecLib::SpecAtomic<SpecRealAbsUlpF>& res6_f, SpecLib::SpecAtomic<SpecRealAbsUlpD>& res6_d, SpecLib::SpecAtomic<SpecRealAbsUlpL>& res6_l, SpecLib::SpecAtomic<SpecRealIndAbsUlpF>& res6i_f, SpecLib::SpecAtomic<SpecRealIndAbsUlpD>& res6i_d, SpecLib::SpecAtomic<SpecRealIndAbsUlpL>& res6i_l
                                                                                                                               ) {
    for (size_t i = begin; (i < end) && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
      mywait(DelaySeconds);
#endif
      res0_c.fetch_add(static_cast<char>(Vals[i]), std::memory_order_relaxed); res0_uc.fetch_add(static_cast<unsigned char>(Vals[i]), std::memory_order_relaxed); res0_i.fetch_add(static_cast<int>(Vals[i]), std::memory_order_relaxed);
      res0_u.fetch_add(static_cast<unsigned int>(Vals[i]), std::memory_order_relaxed); res0_ll.fetch_add(static_cast<long long int>(Vals[i]), std::memory_order_relaxed); res0_ull.fetch_add(static_cast<unsigned long long int>(Vals[i]), std::memory_order_relaxed);
      res2_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res2i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res3_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res3i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res4_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res4i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res5_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res5i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res6_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
      res6i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    }
  };

  const bool test_ok = bench(0, N, 1, loop_f, reset_result, test_f, avg_time, res0_c, res0_uc, res0_i, res0_u, res0_ll, res0_ull,
                                                                              res2_f, res2_d, res2_l, res2i_f, res2i_d, res2i_l,
                                                                              res3_f, res3_d, res3_l, res3i_f, res3i_d, res3i_l,
                                                                              res4_f, res4_d, res4_l, res4i_f, res4i_d, res4i_l,
                                                                              res5_f, res5_d, res5_l, res5i_f, res5i_d, res5i_l,
                                                                              res6_f, res6_d, res6_l, res6i_f, res6i_d, res6i_l);

  long long int sum1 = static_cast<long long int>(res0_c.load()) + static_cast<long long int>(res0_uc.load()) + static_cast<long long int>(res0_i.load()) + static_cast<long long int>(res0_u.load()) + static_cast<long long int>(res0_ll.load()) + static_cast<long long int>(res0_ull.load());
  ldouble sum2 = static_cast<ldouble>(res2_f.load()) + static_cast<ldouble>(res2_d.load()) + static_cast<ldouble>(res2_l.load()) + static_cast<ldouble>(res2i_f.load()) + static_cast<ldouble>(res2i_d.load()) + static_cast<ldouble>(res2i_l.load()) +
                     static_cast<ldouble>(res3_f.load()) + static_cast<ldouble>(res3_d.load()) + static_cast<ldouble>(res3_l.load()) + static_cast<ldouble>(res3i_f.load()) + static_cast<ldouble>(res3i_d.load()) + static_cast<ldouble>(res3i_l.load()) +
                     static_cast<ldouble>(res4_f.load()) + static_cast<ldouble>(res4_d.load()) + static_cast<ldouble>(res4_l.load()) + static_cast<ldouble>(res4i_f.load()) + static_cast<ldouble>(res4i_d.load()) + static_cast<ldouble>(res4i_l.load()) +
                     static_cast<ldouble>(res5_f.load()) + static_cast<ldouble>(res5_d.load()) + static_cast<ldouble>(res5_l.load()) + static_cast<ldouble>(res5i_f.load()) + static_cast<ldouble>(res5i_d.load()) + static_cast<ldouble>(res5i_l.load()) +
                     static_cast<ldouble>(res6_f.load()) + static_cast<ldouble>(res6_d.load()) + static_cast<ldouble>(res6_l.load()) + static_cast<ldouble>(res6i_f.load()) + static_cast<ldouble>(res6i_d.load()) + static_cast<ldouble>(res6i_l.load());

  std::cout << "Lambda loop: " << "sum1: " << sum1 << " sum2: " << sum2 << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

static inline void sf(const size_t i, SpecLib::SpecAtomic<char>& res0_c, SpecLib::SpecAtomic<unsigned char>& res0_uc, SpecLib::SpecAtomic<int>& res0_i, SpecLib::SpecAtomic<unsigned int>& res0_u, SpecLib::SpecAtomic<long long int>& res0_ll, SpecLib::SpecAtomic<unsigned long long int>& res0_ull,
                                      SpecLib::SpecAtomic<SpecRealAbsF>& res2_f, SpecLib::SpecAtomic<SpecRealAbsD>& res2_d, SpecLib::SpecAtomic<SpecRealAbsL>& res2_l, SpecLib::SpecAtomic<SpecRealIndAbsF>& res2i_f, SpecLib::SpecAtomic<SpecRealIndAbsD>& res2i_d, SpecLib::SpecAtomic<SpecRealIndAbsL>& res2i_l,
                                      SpecLib::SpecAtomic<SpecRealRelF>& res3_f, SpecLib::SpecAtomic<SpecRealRelD>& res3_d, SpecLib::SpecAtomic<SpecRealRelL>& res3_l, SpecLib::SpecAtomic<SpecRealIndRelF>& res3i_f, SpecLib::SpecAtomic<SpecRealIndRelD>& res3i_d, SpecLib::SpecAtomic<SpecRealIndRelL>& res3i_l,
                                      SpecLib::SpecAtomic<SpecRealUlpF>& res4_f, SpecLib::SpecAtomic<SpecRealUlpD>& res4_d, SpecLib::SpecAtomic<SpecRealUlpL>& res4_l, SpecLib::SpecAtomic<SpecRealIndUlpF>& res4i_f, SpecLib::SpecAtomic<SpecRealIndUlpD>& res4i_d, SpecLib::SpecAtomic<SpecRealIndUlpL>& res4i_l,
                                      SpecLib::SpecAtomic<SpecRealAbsRelF>& res5_f, SpecLib::SpecAtomic<SpecRealAbsRelD>& res5_d, SpecLib::SpecAtomic<SpecRealAbsRelL>& res5_l, SpecLib::SpecAtomic<SpecRealIndAbsRelF>& res5i_f, SpecLib::SpecAtomic<SpecRealIndAbsRelD>& res5i_d, SpecLib::SpecAtomic<SpecRealIndAbsRelL>& res5i_l,
                                      SpecLib::SpecAtomic<SpecRealAbsUlpF>& res6_f, SpecLib::SpecAtomic<SpecRealAbsUlpD>& res6_d, SpecLib::SpecAtomic<SpecRealAbsUlpL>& res6_l, SpecLib::SpecAtomic<SpecRealIndAbsUlpF>& res6i_f, SpecLib::SpecAtomic<SpecRealIndAbsUlpD>& res6i_d, SpecLib::SpecAtomic<SpecRealIndAbsUlpL>& res6i_l
                                           )
{
#ifdef ENABLE_DELAY
  mywait(DelaySeconds);
#endif
  res0_c.fetch_add(static_cast<char>(Vals[i]), std::memory_order_relaxed); res0_uc.fetch_add(static_cast<unsigned char>(Vals[i]), std::memory_order_relaxed); res0_i.fetch_add(static_cast<int>(Vals[i]), std::memory_order_relaxed);
  res0_u.fetch_add(static_cast<unsigned int>(Vals[i]), std::memory_order_relaxed); res0_ll.fetch_add(static_cast<long long int>(Vals[i]), std::memory_order_relaxed); res0_ull.fetch_add(static_cast<unsigned long long int>(Vals[i]), std::memory_order_relaxed);
  res2_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res2i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res3_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res3i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res4_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res4i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res5_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res5i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res6_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  res6i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
}

bool sf_test()
{
  const bool test_ok = bench(0, N, 1, sf, reset_result, test_f, avg_time, res0_c, res0_uc, res0_i, res0_u, res0_ll, res0_ull,
                                                                          res2_f, res2_d, res2_l, res2i_f, res2i_d, res2i_l,
                                                                          res3_f, res3_d, res3_l, res3i_f, res3i_d, res3i_l,
                                                                          res4_f, res4_d, res4_l, res4i_f, res4i_d, res4i_l,
                                                                          res5_f, res5_d, res5_l, res5i_f, res5i_d, res5i_l,
                                                                          res6_f, res6_d, res6_l, res6i_f, res6i_d, res6i_l);

  long long int sum1 = static_cast<long long int>(res0_c.load()) + static_cast<long long int>(res0_uc.load()) + static_cast<long long int>(res0_i.load()) + static_cast<long long int>(res0_u.load()) + static_cast<long long int>(res0_ll.load()) + static_cast<long long int>(res0_ull.load());
  ldouble sum2 = static_cast<ldouble>(res2_f.load()) + static_cast<ldouble>(res2_d.load()) + static_cast<ldouble>(res2_l.load()) + static_cast<ldouble>(res2i_f.load()) + static_cast<ldouble>(res2i_d.load()) + static_cast<ldouble>(res2i_l.load()) +
                     static_cast<ldouble>(res3_f.load()) + static_cast<ldouble>(res3_d.load()) + static_cast<ldouble>(res3_l.load()) + static_cast<ldouble>(res3i_f.load()) + static_cast<ldouble>(res3i_d.load()) + static_cast<ldouble>(res3i_l.load()) +
                     static_cast<ldouble>(res4_f.load()) + static_cast<ldouble>(res4_d.load()) + static_cast<ldouble>(res4_l.load()) + static_cast<ldouble>(res4i_f.load()) + static_cast<ldouble>(res4i_d.load()) + static_cast<ldouble>(res4i_l.load()) +
                     static_cast<ldouble>(res5_f.load()) + static_cast<ldouble>(res5_d.load()) + static_cast<ldouble>(res5_l.load()) + static_cast<ldouble>(res5i_f.load()) + static_cast<ldouble>(res5i_d.load()) + static_cast<ldouble>(res5i_l.load()) +
                     static_cast<ldouble>(res6_f.load()) + static_cast<ldouble>(res6_d.load()) + static_cast<ldouble>(res6_l.load()) + static_cast<ldouble>(res6i_f.load()) + static_cast<ldouble>(res6i_d.load()) + static_cast<ldouble>(res6i_l.load());

  std::cout << "SF    : " << "sum1: " << sum1 << " sum2: " << sum2 << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

static inline void sf_loop(const SpecLib::ExCommonSpecInfo_t& cs, const size_t begin, const size_t end, const size_t step, SpecLib::SpecAtomic<char>& res0_c, SpecLib::SpecAtomic<unsigned char>& res0_uc, SpecLib::SpecAtomic<int>& res0_i, SpecLib::SpecAtomic<unsigned int>& res0_u, SpecLib::SpecAtomic<long long int>& res0_ll, SpecLib::SpecAtomic<unsigned long long int>& res0_ull,
                                                                                                                           SpecLib::SpecAtomic<SpecRealAbsF>& res2_f, SpecLib::SpecAtomic<SpecRealAbsD>& res2_d, SpecLib::SpecAtomic<SpecRealAbsL>& res2_l, SpecLib::SpecAtomic<SpecRealIndAbsF>& res2i_f, SpecLib::SpecAtomic<SpecRealIndAbsD>& res2i_d, SpecLib::SpecAtomic<SpecRealIndAbsL>& res2i_l,
                                                                                                                           SpecLib::SpecAtomic<SpecRealRelF>& res3_f, SpecLib::SpecAtomic<SpecRealRelD>& res3_d, SpecLib::SpecAtomic<SpecRealRelL>& res3_l, SpecLib::SpecAtomic<SpecRealIndRelF>& res3i_f, SpecLib::SpecAtomic<SpecRealIndRelD>& res3i_d, SpecLib::SpecAtomic<SpecRealIndRelL>& res3i_l,
                                                                                                                           SpecLib::SpecAtomic<SpecRealUlpF>& res4_f, SpecLib::SpecAtomic<SpecRealUlpD>& res4_d, SpecLib::SpecAtomic<SpecRealUlpL>& res4_l, SpecLib::SpecAtomic<SpecRealIndUlpF>& res4i_f, SpecLib::SpecAtomic<SpecRealIndUlpD>& res4i_d, SpecLib::SpecAtomic<SpecRealIndUlpL>& res4i_l,
                                                                                                                           SpecLib::SpecAtomic<SpecRealAbsRelF>& res5_f, SpecLib::SpecAtomic<SpecRealAbsRelD>& res5_d, SpecLib::SpecAtomic<SpecRealAbsRelL>& res5_l, SpecLib::SpecAtomic<SpecRealIndAbsRelF>& res5i_f, SpecLib::SpecAtomic<SpecRealIndAbsRelD>& res5i_d, SpecLib::SpecAtomic<SpecRealIndAbsRelL>& res5i_l,
                                                                                                                           SpecLib::SpecAtomic<SpecRealAbsUlpF>& res6_f, SpecLib::SpecAtomic<SpecRealAbsUlpD>& res6_d, SpecLib::SpecAtomic<SpecRealAbsUlpL>& res6_l, SpecLib::SpecAtomic<SpecRealIndAbsUlpF>& res6i_f, SpecLib::SpecAtomic<SpecRealIndAbsUlpD>& res6i_d, SpecLib::SpecAtomic<SpecRealIndAbsUlpL>& res6i_l
                                                                                                                                )
{
  for (size_t i = begin; (i < end) && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
    mywait(DelaySeconds);
#endif
    res0_c.fetch_add(static_cast<char>(Vals[i]), std::memory_order_relaxed); res0_uc.fetch_add(static_cast<unsigned char>(Vals[i]), std::memory_order_relaxed); res0_i.fetch_add(static_cast<int>(Vals[i]), std::memory_order_relaxed);
    res0_u.fetch_add(static_cast<unsigned int>(Vals[i]), std::memory_order_relaxed); res0_ll.fetch_add(static_cast<long long int>(Vals[i]), std::memory_order_relaxed); res0_ull.fetch_add(static_cast<unsigned long long int>(Vals[i]), std::memory_order_relaxed);
    res2_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res2i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res2i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res2i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res3_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res3i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res3i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res3i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res4_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res4i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res4i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res4i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res5_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res5i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res5i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res5i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res6_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
    res6i_f.fetch_add(static_cast<float>(Vals[i]), std::memory_order_relaxed); res6i_d.fetch_add(static_cast<double>(Vals[i]), std::memory_order_relaxed); res6i_l.fetch_add(static_cast<ldouble>(Vals[i]), std::memory_order_relaxed);
  }
}

bool sf_loop_test()
{
  const bool test_ok = bench(0, N, 1, sf_loop, reset_result, test_f, avg_time, res0_c, res0_uc, res0_i, res0_u, res0_ll, res0_ull,
                                                                               res2_f, res2_d, res2_l, res2i_f, res2i_d, res2i_l,
                                                                               res3_f, res3_d, res3_l, res3i_f, res3i_d, res3i_l,
                                                                               res4_f, res4_d, res4_l, res4i_f, res4i_d, res4i_l,
                                                                               res5_f, res5_d, res5_l, res5i_f, res5i_d, res5i_l,
                                                                               res6_f, res6_d, res6_l, res6i_f, res6i_d, res6i_l);
  
  long long int sum1 = static_cast<long long int>(res0_c.load()) + static_cast<long long int>(res0_uc.load()) + static_cast<long long int>(res0_i.load()) + static_cast<long long int>(res0_u.load()) + static_cast<long long int>(res0_ll.load()) + static_cast<long long int>(res0_ull.load());
  ldouble sum2 = static_cast<ldouble>(res2_f.load()) + static_cast<ldouble>(res2_d.load()) + static_cast<ldouble>(res2_l.load()) + static_cast<ldouble>(res2i_f.load()) + static_cast<ldouble>(res2i_d.load()) + static_cast<ldouble>(res2i_l.load()) +
                     static_cast<ldouble>(res3_f.load()) + static_cast<ldouble>(res3_d.load()) + static_cast<ldouble>(res3_l.load()) + static_cast<ldouble>(res3i_f.load()) + static_cast<ldouble>(res3i_d.load()) + static_cast<ldouble>(res3i_l.load()) +
                     static_cast<ldouble>(res4_f.load()) + static_cast<ldouble>(res4_d.load()) + static_cast<ldouble>(res4_l.load()) + static_cast<ldouble>(res4i_f.load()) + static_cast<ldouble>(res4i_d.load()) + static_cast<ldouble>(res4i_l.load()) +
                     static_cast<ldouble>(res5_f.load()) + static_cast<ldouble>(res5_d.load()) + static_cast<ldouble>(res5_l.load()) + static_cast<ldouble>(res5i_f.load()) + static_cast<ldouble>(res5i_d.load()) + static_cast<ldouble>(res5i_l.load()) +
                     static_cast<ldouble>(res6_f.load()) + static_cast<ldouble>(res6_d.load()) + static_cast<ldouble>(res6_l.load()) + static_cast<ldouble>(res6i_f.load()) + static_cast<ldouble>(res6i_d.load()) + static_cast<ldouble>(res6i_l.load());

  std::cout << "SF loop: " << "sum1: " << sum1 << " sum2: " << sum2 << " " << (test_ok ? 'Y' : 'N') << std::endl;
  std::cout << "Time  : " << avg_time << std::endl << std::endl;

  return test_ok;
}

int main(int argc, char **argv)
{
  process_args(argc, argv, "hc:d:m:N:n:t:s:v", N);

  Vals = new float[N];
  auto mt_rand_gen = std::bind(std::uniform_real_distribution<float>(static_cast<float>(std::numeric_limits<char>::min()), static_cast<float>(std::numeric_limits<char>::max())), std::mt19937(static_cast<std::mt19937::result_type>(static_cast<ldouble>(RAND_SEED))));
  for (size_t i = 0; i < N; i++) {
    Vals[i] = mt_rand_gen();
  }
  Vals[N - std::max(2 * N/NChunks, N)] = * std::max_element(Vals, Vals + N) + 1;	//put the maximum value in the first position
  
  seq_test();

  do_preheat(); // Preheat

  return lambda_test() && lambda_loop_test() && sf_test() && sf_loop_test() ? 0 : -1;
}
