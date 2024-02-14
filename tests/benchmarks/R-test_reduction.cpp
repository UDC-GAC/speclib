#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <iostream>
#include <chrono>
#include <type_traits>
#include <limits>

using profile_clock_t = std::conditional<(std::chrono::high_resolution_clock::is_steady || !std::chrono::steady_clock::is_steady), std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

using IDouble_T = double;
using SDouble_T = SpecLib::SpecReal<IDouble_T, SpecLib::EPS_RELATIVE>;

#ifdef VALIDATE
template<typename T>
static size_t count_digits(T number) {
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

bool seq_test_and_check(const unsigned long long int NITER, const SpecLib::ReductionVar<SDouble_T>& sumatorioP) {
	IDouble_T sumatorioS = 3.123124;
	for (unsigned long long int iteration=0; iteration < NITER; iteration++) {
		const IDouble_T value = (IDouble_T)(iteration % 2)*static_cast<IDouble_T>(0.0012) + static_cast<IDouble_T>(0.0000003)*(IDouble_T)iteration;
		sumatorioS += value;
	}

	const bool check1 = ((sumatorioP.result().getValue()) == sumatorioS);
	const bool check2 = ((sumatorioP.result()) == static_cast<IDouble_T>(sumatorioS));
	std::cout << "[*VALIDATION*]: " << (check1 ? "OK!" : (check2 ? "OK! (small precision differences in the acceptable range)" : "FAILED!")) << std::endl;
	if (!check1) {
		const IDouble_T diff = ((sumatorioP.result().getValue() > sumatorioS) ? (sumatorioP.result().getValue() - sumatorioS) : (sumatorioS - sumatorioP.result().getValue()));
		const IDouble_T maxValue = std::abs(std::max(std::abs(sumatorioP.result().getValue()), std::abs(sumatorioS)));
		const IDouble_T tole = SDouble_T::getPrecisionThreshold() * maxValue;
		const int fieldWidth = static_cast<int>(count_digits(maxValue)) + 11;
		std::cout << "ValuePar: " << std::setw(fieldWidth) << std::fixed << std::setprecision(10) << std::right << sumatorioP.result().getValue() << std::endl;
		std::cout << "ValueSeq: " << std::setw(fieldWidth) << std::fixed << std::setprecision(10) << std::right << sumatorioS << std::endl;
		std::cout << "Diff -->  " << std::setw(fieldWidth) << std::fixed << std::setprecision(10) << std::right << diff << std::setprecision(18) << " (" << (diff * static_cast<IDouble_T>(100.0) / maxValue) << "%)" << std::endl;
		std::cout << "Tol  ==>  " << std::setw(fieldWidth) << std::fixed << std::setprecision(10) << std::right << tole << std::setprecision(18) << " (" << (tole * static_cast<IDouble_T>(100.0) / maxValue) << "%)" << std::endl;
	}
	return (check1 || check2);
}
#endif

auto sf_loop = [](const SpecLib::ExCommonSpecInfo_t& cs, const unsigned long long int begin, const unsigned long long int end, const unsigned long long int step, SpecLib::ReductionVar<SDouble_T>& sumatorio) {
	auto &sum_thread_val = sumatorio.thread_val();
	IDouble_T lsumatorio = sum_thread_val;
	for (unsigned long long int iteration = begin; iteration < end && (!cs.cancelled()); iteration += step) {
#ifdef ENABLE_DELAY
		mywait(DelaySeconds);
#endif
		const IDouble_T value = (IDouble_T)(iteration % 2)*static_cast<IDouble_T>(0.0012) + static_cast<IDouble_T>(0.0000003)*(IDouble_T)iteration;
		lsumatorio += value;
	}
	sum_thread_val = lsumatorio;
};

auto sf = [](const unsigned long long int iteration, SpecLib::ReductionVar<SDouble_T>& sumatorio) {
#ifdef ENABLE_DELAY
	mywait(DelaySeconds);
#endif
	const IDouble_T value = (IDouble_T)(iteration % 2)*static_cast<IDouble_T>(0.0012) + static_cast<IDouble_T>(0.0000003)*(IDouble_T)iteration;
	sumatorio.thread_val() += value;
};

int main(int argc, char *argv[]) {
	unsigned long long int NITER = 1000000;

	process_args(argc, argv, "P:hc:d:m:n:t:s:v", NITER, true, false);
	// Read additional parameters
	SDouble_T::value_type_nv spec_double_precision = static_cast<SDouble_T::value_type_nv>(std::numeric_limits<IDouble_T>::epsilon()*8000);
	for (int i = 1; i < argc; i++) {
		if ((argv[i][0] == '-') && (argv[i][1] == 'P') && (argv[i][2] == '\0')) {
			if (i+1 >= argc) {
				std::cout << "Error: Missing -P parameter value" << std::endl;
				return -1;
			} else {
				spec_double_precision = static_cast<SDouble_T::value_type_nv>(std::max(strtod(argv[i+1], nullptr), 0.0));
				std::cout << "Precision of every double comparison configured to: " << spec_double_precision << std::endl;
			}
		} else if ((argv[i][0] == '-') && (argv[i][1] == 'h') && (argv[i][2] == '\0')) {
			std::cout << "-P n    Configure precision of every speculative comparison (relative)" << std::endl;
			return 1;
		}
	}

	do_preheat();	// Preheat

	SDouble_T::setPrecisionThreshold(spec_double_precision);
	SpecLib::ReductionVar<SDouble_T> sumatorio(SDouble_T{static_cast<IDouble_T>(0.0)}, std::plus<SDouble_T>(), SDouble_T{static_cast<IDouble_T>(3.123124)});

	// speccode: Speculative loop

#ifdef THREADINSTRUMENT
	ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

	/************************************************************************/
	// speccode: Start profiling
	const auto par_start = profile_clock_t::now();
	/***********************************************************************/

#if !defined(SLSTATS) && !defined(SLMINIMALSTATS)
#ifdef SLSIMULATE
	SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, sumatorio);
#else
	SpecLib::specRun({NThreads, MinParalThreads}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, sumatorio);
#endif
#else
#ifdef SLSIMULATE
	SpecLib::StatsRunInfo statsRes = SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, sumatorio);
#else
	SpecLib::StatsRunInfo statsRes = SpecLib::specRun({NThreads, MinParalThreads}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, sumatorio);
#endif
#endif

	/************************************************************************/
	// speccode: Stop measuring time
	const auto par_stop = profile_clock_t::now();
	const double par_time = std::chrono::duration<double>(par_stop - par_start).count();
	/************************************************************************/

	/****************************/
	/* Printing execution times */
	/****************************/
	std::cout << "Sumatorio = " << std::fixed << std::setprecision(10) << (sumatorio.result()) << std::endl;
	std::cerr << "Speculative time: " << par_time << " seconds." << std::endl;

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
	printStatsRunInfo(statsRes);
#endif

#ifdef THREADINSTRUMENT
	ThreadInstrument::dumpLog("tilog.log");
#endif

#ifdef VALIDATE
	return (seq_test_and_check(NITER, sumatorio)) ? 0 : -1;
#else
	return 0;
#endif
}
