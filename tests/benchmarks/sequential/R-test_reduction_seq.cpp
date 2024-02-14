#define NOSPECLIB
#include "../../common_files/common_test.cpp"
#include <iostream>
#include <chrono>
#include <type_traits>

using profile_clock_t = std::conditional<(std::chrono::high_resolution_clock::is_steady || !std::chrono::steady_clock::is_steady), std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

/*--------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
	unsigned long long int NITER = 1000000;

	process_args(argc, argv, "hd:n:", NITER);

	// speccode: Speculative loop
	double sumatorio = 3.123124;

	/************************************************************************/
	// speccode: Start profiling
	const auto seq_start = profile_clock_t::now();
	/***********************************************************************/

	for (unsigned long long int iteration=0; iteration < NITER; iteration++) {
#ifdef ENABLE_DELAY
		mywait(DelaySeconds);
#endif
		const double value = (double)(iteration % 2)*0.0012 + 0.0000003*(double)iteration;
		sumatorio += value;
	}

	/************************************************************************/
	// speccode: Stop measuring time
	const auto seq_stop = profile_clock_t::now();
	const double seq_time = std::chrono::duration<double>(seq_stop - seq_start).count();
	/************************************************************************/

	/****************************/
	/* Printing execution times */
	/****************************/
	std::cout << "Sumatorio = " << std::fixed << std::setprecision(10) << sumatorio << std::endl;
	std::cerr << "Speculative time: " << seq_time << " seconds." << std::endl;

	return 0;
}
