#define NOSPECLIB
#include "../../common_files/common_test.cpp"
#include <iostream>
#include <chrono>
#include <array>
#include <string>
#include <algorithm>
#include <type_traits>
#include <cstdio>

using profile_clock_t = std::conditional<(std::chrono::high_resolution_clock::is_steady || !std::chrono::steady_clock::is_steady), std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

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

int main(int argc, char **argv) {
	std::string inputFileName("inputs/data.in");

	process_args(argc, argv, "hd:n:i:", NITER, inputFileName);

	/* Local variables:*/
	std::array<long long int, MAX> array;
	int aux;
	long long int pre_sum = 0;
	FILE *file;

	/* Open and read the file.  */
	if ((file = fopen(inputFileName.c_str(), "r")) == NULL) {
		if ((file = fopen(("../" + inputFileName).c_str(), "r")) == NULL) {
			if ((file = fopen(("tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
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

	/* Sum the elements of the array.  */
	for (int i = 0; i < MAX; i++){
		pre_sum = pre_sum + array[i];
		std::cout << "BEFORE:array[" << i << "]=" << array[i] << std::endl;
	}
	std::cout << " Sum of the elements of the array (before processing) = " << pre_sum << std::endl;

	limit1 = (int) (((double)NITER) * 0.99);
	limit2 = (int) (((double)NITER) * 0.01);

	long long int l = 5;

	const auto seq_init = profile_clock_t::now();

	// The loop is not parallelizable because indexes of
	// the array elements depends on the input values from a file
	for (int i = 0; i < NITER; i++) {
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
		l += lval1;
		j += lval2;
	}

	/* Stop profiling.  */
	const auto seq_end = profile_clock_t::now();
	const double seq_time = std::chrono::duration<double>(seq_end - seq_init).count();

	/********************/
	/* Writing results  */
	/********************/
	std::cout << "NITER = " << NITER << ", limit1 = " << limit1 << ", limit2 = " << limit2 << ", L= " << l << ", J = " << j << std::endl;
	long long int sum = 0;
	for (int i = 0; i < MAX; i++){
		sum += array[i];
		std::cout << "AFTER:array[" << i << "]=" << array[i] << std::endl;
	}
	std::cout << "[SEQ] Sum of the elements of the array = " << sum << std::endl;

	/****************************/
	/* Printing execution times */
	/****************************/
	std::cerr << "[SEQ] Execution time: " << seq_time << " seg." << std::endl;


	return 0;
}
