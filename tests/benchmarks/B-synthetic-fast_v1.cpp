#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <iostream>
#include <chrono>
#include <array>
#include <string>
#include <algorithm>
#include <cstdio>

/*******************************************/
/* User parameters set by the programmer:  */
/*******************************************/
// Array size 
#define MAX 4
static_assert(MAX>2, "MAX should be greater than 2");
// Number of iterations of the loop
int NITER = 30000;
int limit1;
int limit2;

/*--------------------------------------------------------------------------*/

#ifdef VALIDATE
bool seq_test_and_check(const long long int lO, const long long int jO, const std::array<long long int, MAX> arrayO, const long long int val1P, const long long int val2P, const std::array<long long int, MAX>& arrayP) {
	long long int val1 = lO;
	long long int val2 = jO;
	std::array<long long int, MAX> array = arrayO;
	for (int i = 0; i < NITER; i++) {
		int j9;
		if (i == NITER/59*19) {
			const long long int tmp = array[MAX-1]+1;
			for (int b = MAX-1; b > 0; b--) {
				array[b] = array[b-1]+1;
			}
			array[0] = tmp;
			j9 = i/4 + (int)array[(i+1)%MAX] % NITER;
		} else if (i == NITER/57*47) {
			const long long int tmp1 = array[0]+2;
			const long long int tmp2 = array[1]+2;
			for (int b = 0; b < MAX-2; b++) {
				array[b] = array[b+2]+2;
			}
			array[MAX-2] = tmp1;
			array[MAX-1] = tmp2;
			j9 = i/3 + (int)array[(i-2)%MAX] % NITER;
		} else {
			j9 = NITER+NITER/10;
		}
		long long int lval1 = i;
		long long int lval2 = i/2;
		for (int k = (j9+(int)array[(i+2)%MAX])-1; (k >= limit1) && (k >= (int)array[i%MAX]); k--) {
			lval1 += (long long int)k/10 + (long long int)i/80 + 43 - array[(i+k-1)%MAX];
		}
		for (int k = (int)array[i%MAX]; (k <= limit2) && (k < (j9+(int)array[(i+2)%MAX])); k++) {
			lval2 += (long long int)i/100 + (long long int)k/20 + 57 + array[(i+k+1)%MAX];
		}
		if (i % array[(i+1)%MAX] == 0) {
			lval1 -= lval2;
		}
		val1 += lval1;
		val2 += lval2;
	}
	bool check = ((val1 == val1P) && (val2 == val2P) && (array == arrayP));
	std::cout << "[*VALIDATION*]: " << (check ? "OK!" : "FAILED!") << std::endl;
	return check;
}
#endif

static inline void sf_loop(const SpecLib::ExCommonSpecInfo_t& cs, const int begin, const int end, const int step, SpecLib::ReductionVar<long long int>& val1, SpecLib::ReductionVar<long long int>& val2, std::array<long long int, MAX>& array) {
	for (int i = begin; i < end && !cs.cancelled(); i+=step) {
#ifdef ENABLE_DELAY
		mywait(DelaySeconds);
#endif
		int j9;
		if (i == NITER/59*19) {
			const long long int tmp = array[MAX-1]+1;
			for (int b = MAX-1; b > 0; b--) {
				array[b] = array[b-1]+1;
			}
			array[0] = tmp;
			j9 = i/4 + (int)array[(i+1)%MAX] % NITER;
		} else if (i == NITER/57*47) {
			const long long int tmp1 = array[0]+2;
			const long long int tmp2 = array[1]+2;
			for (int b = 0; b < MAX-2; b++) {
				array[b] = array[b+2]+2;
			}
			array[MAX-2] = tmp1;
			array[MAX-1] = tmp2;
			j9 = i/3 + (int)array[(i-2)%MAX] % NITER;
		} else {
			j9 = NITER+NITER/10;
		}
		long long int lval1 = i;
		long long int lval2 = i/2;
		for (int k = (j9+(int)array[(i+2)%MAX])-1; (k >= limit1) && (k >= (int)array[i%MAX]); k--) {
			lval1 += (long long int)k/10 + (long long int)i/80 + 43 - array[(i+k-1)%MAX];
		}
		for (int k = (int)array[i%MAX]; (k <= limit2) && (k < (j9+(int)array[(i+2)%MAX])); k++) {
			lval2 += (long long int)i/100 + (long long int)k/20 + 57 + array[(i+k+1)%MAX];
		}
		if (i % array[(i+1)%MAX] == 0) {
			lval1 -= lval2;
		}
		val1.thread_val() += lval1;
		val2.thread_val() += lval2;
	}
}

auto sf = [](const int i, SpecLib::ReductionVar<long long int>& val1, SpecLib::ReductionVar<long long int>& val2, std::array<long long int, MAX>& array) {
#ifdef ENABLE_DELAY
	mywait(DelaySeconds);
#endif
	int j9;
	if (i == NITER/59*19) {
		const long long int tmp = array[MAX-1]+1;
		for (int b = MAX-1; b > 0; b--) {
			array[b] = array[b-1]+1;
		}
		array[0] = tmp;
		j9 = i/4 + (int)array[(i+1)%MAX] % NITER;
	} else if (i == NITER/57*47) {
		const long long int tmp1 = array[0]+2;
		const long long int tmp2 = array[1]+2;
		for (int b = 0; b < MAX-2; b++) {
			array[b] = array[b+2]+2;
		}
		array[MAX-2] = tmp1;
		array[MAX-1] = tmp2;
		j9 = i/3 + (int)array[(i-2)%MAX] % NITER;
	} else {
		j9 = NITER+NITER/10;
	}
	long long int lval1 = i;
	long long int lval2 = i/2;
	for (int k = (j9+(int)array[(i+2)%MAX])-1; (k >= limit1) && (k >= (int)array[i%MAX]); k--) {
		lval1 += (long long int)k/10 + (long long int)i/80 + 43 - array[(i+k-1)%MAX];
	}
	for (int k = (int)array[i%MAX]; (k <= limit2) && (k < (j9+(int)array[(i+2)%MAX])); k++) {
		lval2 += (long long int)i/100 + (long long int)k/20 + 57 + array[(i+k+1)%MAX];
	}
	if (i % array[(i+1)%MAX] == 0) {
		lval1 -= lval2;
	}
	val1.thread_val() += lval1;
	val2.thread_val() += lval2;
};

int main(int argc, char **argv) {
	std::array<long long int, MAX> array;

	std::string inputFileName("inputs/data.in");

	process_args(argc, argv, "hc:d:m:n:t:s:vi:", NITER, inputFileName);

	limit1 = (int) (((double)NITER) * 0.99);
	limit2 = (int) (((double)NITER) * 0.01);

	/* Local variables:*/
	int aux;
	long long int pre_sum = 0;
	FILE *file;

	/* Open and read the file.  */
	if ((file = fopen(inputFileName.c_str(), "r")) == NULL) {
		if ((file = fopen(("tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
			if ((file = fopen(("../" + inputFileName).c_str(), "r")) == NULL) {
				if ((file = fopen(("../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
					if ((file = fopen(("../../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
						if ((file = fopen(("../../../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
							if ((file = fopen(("../../../../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
								std::cout << "Error opening the file" << std::endl;
								exit(0);
							}
						}
					}
				}
			}
		}
	}
	long long int j;
	for (int i = 0; i < MAX; i++) {
		if (((j = fscanf(file, "%d", &aux)) < 0) && ferror(file)) {
			std::cout << "Error reading numbers from file" << std::endl;
			return -1;
		}
		array[i] = (long long int)aux;
	}
	fclose(file);

	/* Sum the elements of the array.	*/
	for (int i = 0; i < MAX; i++){
		pre_sum = pre_sum + array[i];
		std::cout << "BEFORE:array[" << i << "]=" << array[i] << std::endl;
	}
	std::cout << " Sum of the elements of the array (before processing) = " << pre_sum << std::endl;

	do_preheat();	// Preheat

	long long int l = 5;
#ifdef VALIDATE
	std::array<long long int, MAX> arrayO = array;
	long long int lO = l;
	long long int jO = j;
#endif

#ifdef THREADINSTRUMENT
	ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

	/* Start profiling.  */
	const auto par_init = profile_clock_t::now();

	// The loop is not parallelizable because indexes of
	// the array elements depends on the input values from a file

	SpecLib::ReductionVar<long long int> lR(0, std::plus<long long int>(), l);
	SpecLib::ReductionVar<long long int> jR(0, std::plus<long long int>(), j);
#if !defined(SLSTATS) && !defined(SLMINIMALSTATS)
#ifdef SLSIMULATE
	SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, lR, jR, array);
#else
	SpecLib::specRun({NThreads, MinParalThreads}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, lR, jR, array);
#endif
#else
#ifdef SLSIMULATE
	SpecLib::StatsRunInfo statsRes = SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, lR, jR, array);
#else
	SpecLib::StatsRunInfo statsRes = SpecLib::specRun({NThreads, MinParalThreads}, 0, NITER, 1, SpecLib::getChunkSize(NITER, NChunks), sf_loop, lR, jR, array);
#endif
#endif
	j = jR.result();
	l = lR.result();

	/* Stop profiling.  */
	const auto par_end = profile_clock_t::now();
	const double par_time = std::chrono::duration<double>(par_end - par_init).count();

	/********************/
	/* Writing results  */
	/********************/
	std::cout << "NITER = " << NITER << ", limit1 = " << limit1 << ", limit2 = " << limit2 << ", L= " << l << ", J = " << j << std::endl;
	long long int sum = 0;
	for (int i = 0; i < MAX; i++){
		sum += array[i];
		std::cout << "AFTER:array[" << i << "]=" << array[i] << std::endl;
	}
	std::cout << "[PAR] Sum of the elements of the array = " << sum << std::endl;

	/****************************/
	/* Printing execution times */
	/****************************/
	std::cerr << "[PAR]Execution time: " << par_time << " seg." << std::endl;

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
	printStatsRunInfo(statsRes);
#endif

#ifdef THREADINSTRUMENT
	ThreadInstrument::dumpLog("tilog.log");
#endif

#ifdef VALIDATE
	return (seq_test_and_check(lO, jO, arrayO, l, j, array)) ? 0 : -1;
#else
	return 0;
#endif
}
