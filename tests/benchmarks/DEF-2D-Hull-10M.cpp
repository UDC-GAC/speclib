#include "speclib/speclib.h"
#include "../common_files/common_test.cpp"
#include <iostream>
#include <chrono>
#include <type_traits>
#include <array>
#include <string>
#include <cstdio>
#include <algorithm>

using profile_clock_t = std::conditional<(std::chrono::high_resolution_clock::is_steady || !std::chrono::steady_clock::is_steady), std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

/*
 * incr-2d: C version of 2-dimensional, incremental convex hull
 * Alvaro Garcia-Yaguez
 * University of Valladolid
 * Started in Sat Oct 9 16:15:00 CET 2010
 */

// Header file for measuring performance with Sun compiler:
// #include "libcollector.h"

constexpr size_t source = 1;
constexpr size_t target = 2;
constexpr size_t succ = 3;
constexpr size_t pred = 4;
constexpr size_t inch = 5;

constexpr size_t x = 1;
constexpr size_t y = 2;

// N: total number of points  (min. 3)
#if defined(DISC)
// For 2D-Hull, Disc, 10M = 10.000.000
constexpr size_t N = 10000000;
#elif defined(SQUARE)
// For 2D-Hull, Square, 10M = 10.000.000
constexpr size_t N = 10000000;
#elif defined(KUZMIN)
// For 2D-Hull, Kuzmin, 10M = 10.000.000
constexpr size_t N = 10000000;
#else
//#error The type of problem must be defined. Accepted values: DISC, SQUARE, KUZMIN
// Default type is DISC: => For 2D-Hull, Disc, 10M = 10.000.000
constexpr size_t N = 10000000;
#endif

// TOP: Number of points in the Convex Hull
#if defined(DISC)
// For 2D-Hull, Disc, 10M = 4393 (4400?)
constexpr size_t TOP = 4400;
#elif defined(SQUARE)
// For 2D-Hull, Square, 10M = 650
constexpr size_t TOP = 650;
#elif defined(KUZMIN)
// For 2D-Hull, Kuzmin, 10M = 200
constexpr size_t TOP = 200;
#else
//#error The type of problem must be defined. Accepted values: DISC, SQUARE, KUZMIN
// Default type is DISC: => For 2D-Hull, Disc, 10M = 4393 (4400?)
constexpr size_t TOP = 4400;
#endif

// Max Number of Loops Allowed
#if defined(DISC)
// For 2D-Hull, Disc, 10M = 4393 (4400?)
// 40, 5, 3
constexpr int loop1c = 40;
constexpr int loop2c = 5;
constexpr int loop3c = 3;
#elif defined(SQUARE)
// For 2D-Hull, Square, 10M = 650
// 39	6	3
constexpr int loop1c = 39;
constexpr int loop2c = 6;
constexpr int loop3c = 3;
#elif defined(KUZMIN)
// For 2D-Hull, Kuzmin, 10M = 200
// 36	4	2
constexpr int loop1c = 36;
constexpr int loop2c = 4;
constexpr int loop3c = 2;
#else
//#error The type of problem must be defined. Accepted values: DISC, SQUARE, KUZMIN
// Default type is DISC: => For 2D-Hull, Disc, 10M = 4393 (4400?)
// 40, 5, 3
constexpr int loop1c = 40;
constexpr int loop2c = 5;
constexpr int loop3c = 3;
#endif

std::array<std::array<double, N+1>, 2+1> input;

// Function
constexpr double rightturn(const double& dx, const double& dy, const double& px, const double& py, const double& qx, const double& qy) {
	return (dx*py + dy*qx + px*qy -qx*py -dx*qy-px*dy);
}

#ifdef VALIDATE
bool seq_test_and_check(const std::array<std::array<size_t, 5+1>, TOP+1>& hullP) {
	//////
	std::array<std::array<size_t, 5+1>, TOP+1> hull = {};
	{
		const double dx = input[x][1];
		const double dy = input[y][1];
		const double px = input[x][2];
		const double py = input[y][2];
		const double qx = input[x][3];
		const double qy = input[y][3];

		const double determ = rightturn(dx,dy,px,py,qx,qy);

		if (determ == 0) {
			std::cout << "incremental: The first three points are aligned." << std::endl;
			std::cout << "incremental: We aree too lazy to proceed. Stop." << std::endl;
			return -1;
		}

		// If determinant < 0, reorder points two and three
		if (determ < 0) {
			input[x][2] = qx;
			input[y][2] = qy;
			input[x][3] = px;
			input[y][3] = py;
		}
	}

	// Links the first edge
	hull[1][source] = 1;
	hull[1][target] = 2;
	hull[1][pred] = 3;
	hull[1][succ] = 2;
	hull[1][inch] = 1;

	// Links the second edge
	hull[2][source] = 2;
	hull[2][target] = 3;
	hull[2][pred] = 1;
	hull[2][succ] = 3;
	hull[2][inch] = 1;

	// Links the third edge
	hull[3][source] = 3;
	hull[3][target] = 1;
	hull[3][pred] = 2;
	hull[3][succ] = 1;
	hull[3][inch] = 1;

	hull[TOP][1] = 1;
	hull[TOP][5] = 3;
	// Put -1 (or 0) in all remaining elements of hull, just in case.
	for (size_t i = 4 ; i < TOP ; i++) {
		for (size_t j = 1 ; j < 6 ; j++ ) {
			hull[i][j] = 0;
		}
	}
	//printf("INI: %d\n",i);
	//printf("TOP: %d\n",hull[TOP][5]);

	// Main loop (iterating over remaining points)
	for (size_t i = 4 ; i < N+1 ; i++) {
		size_t e;
		bool outside = false;
		for (e = 1 ; e < 4 ; e++) {
			const size_t psource = hull[e][source];
			assert(psource < N+1);
			const double dx = input[x][psource];
			const double dy = input[y][psource];

			const size_t ptarget = hull[e][target];
			assert(ptarget < N+1);
			const double px = input[x][ptarget];
			const double py = input[y][ptarget];

			const double qx = input[x][i];
			const double qy = input[y][i];

			const double determ = rightturn(dx,dy,px,py,qx,qy);
			if (determ < 0) {
				// The point is outside
				outside = true;
				break;
			}
		}

		while((hull[e][inch] == 0) && (outside)) {
			const size_t eprev = hull[e][pred];
			assert(eprev < TOP+1);

			const size_t psource = hull[eprev][source];
			assert(psource < N+1);
			const double dx = input[x][psource];
			const double dy = input[y][psource];

			const size_t ptarget = hull[eprev][target];
			assert(ptarget < N+1);
			const double px = input[x][ptarget];
			const double py = input[y][ptarget];

			const double qx = input[x][i];
			const double qy = input[y][i];

			const double determ = rightturn(dx,dy,px,py,qx,qy);

			if (determ < 0) {
				assert(e != eprev);
				e = eprev;
			} else {
				const size_t enext = hull[e][succ];
				assert(enext < TOP+1);

				const size_t psource = hull[enext][source];
				assert(psource < N+1);
				const double dx2 = input[x][psource];
				const double dy2 = input[y][psource];

				const size_t ptarget = hull[enext][target];
				assert(ptarget < N+1);
				const double px2 = input[x][ptarget];
				const double py2 = input[y][ptarget];

				const double qx2 = input[x][i];
				const double qy2 = input[y][i];

				const double determ2 = rightturn(dx2,dy2,px2,py2,qx2,qy2);

				if (determ2 >= 0) {
					outside = false;
				} else {
					assert(e != enext);
					e = enext;
				}
			}
		}

		if (outside) {
			//Compute upper tangent
			size_t enext = hull[e][succ];
			assert(enext < TOP+1);

			size_t lastedge;
			bool flag;
			do {
				flag = false;
				const size_t psource = hull[enext][source];
				assert(psource < N+1);
				const double dx = input[x][psource];
				const double dy = input[y][psource];

				const size_t ptarget = hull[enext][target];
				assert(ptarget < N+1);
				const double px = input[x][ptarget];
				const double py = input[y][ptarget];

				const double qx = input[x][i];
				const double qy = input[y][i];

				const double determ = rightturn(dx,dy,px,py,qx,qy);

				lastedge = hull[TOP][5];
				assert(lastedge+2 < TOP+1);

				if (determ <= 0) {
					hull[enext][inch] = 0;
					hull[enext][pred] = lastedge+1;
					const size_t ea = hull[enext][succ];
					assert(ea < TOP+1);
					hull[enext][succ] = lastedge+2;
					assert(enext != ea);
					enext = ea;
					flag = true;
				}
			}while(flag);
			// Compute lower tangent

			size_t eprev = hull[e][pred];
			assert(eprev < TOP+1);

			do {
				flag = false;
				assert(hull[eprev][source] < N+1);
				assert(hull[eprev][target] < N+1);
				const double dx2 = input[x][hull[eprev][source]];
				const double dy2 = input[y][hull[eprev][source]];
				const double px2 = input[x][hull[eprev][target]];
				const double py2 = input[y][hull[eprev][target]];
				const double qx2 = input[x][i];
				const double qy2 = input[y][i];

				const double determ2 = rightturn(dx2,dy2,px2,py2,qx2,qy2);

				if (determ2 <= 0) {
					hull[eprev][inch] = 0;
					const size_t ea = hull[eprev][pred];
					assert(ea < TOP+1);
					hull[eprev][pred] = lastedge+1;
					hull[eprev][succ] = lastedge+2;
					assert(eprev != ea);
					eprev = ea;
					flag = true;
				}
			}while(flag);

			hull[e][inch] = 0;
			hull[e][pred] = lastedge+1;
			hull[e][succ] = lastedge+2;

			// Add new edges to convex hull
			lastedge++;
			assert(lastedge+1 < TOP+1);
			hull[TOP][5] = lastedge;
			assert(hull[eprev][target] < N+1);
			hull[lastedge][source] = hull[eprev][target];
			hull[lastedge][target] = i;
			hull[lastedge][succ] = lastedge + 1;
			hull[lastedge][pred] = eprev;
			hull[lastedge][inch] = 1;

			lastedge++;
			assert(lastedge < TOP+1);
			hull[TOP][5] = lastedge;
			hull[lastedge][source] = i;
			assert(hull[enext][source] < N+1);
			hull[lastedge][target] = hull[enext][source];
			hull[lastedge][succ] = enext;
			hull[lastedge][pred] = lastedge - 1;
			hull[lastedge][inch] = 1;

			// Connect to old convex hull
			hull[enext][pred] = lastedge;
			hull[eprev][succ] = lastedge - 1;

			//Update first
			hull[TOP][1] = lastedge;
		}
		// End of process, proceed with next point
	}
	//////
	bool check = (hull == hullP);
//	if (!check) {
//		for (size_t i = 0; i < (TOP+1); i++) {
//			for (size_t j = 0; j < (5+1); j++) {
//				if (hull[i][j] != hullP[i][j]) {
//					std::cout << "[" << i << "][" << j << "]: " << hull[i][j] << " != " << hullP[i][j] << std::endl;
//				}
//			}
//		}
//	}
	std::cout << "[*VALIDATION*]: " << (check ? "OK!" : "FAILED!") << std::endl;
	return check;
}
#endif

static inline void sf_loop(const SpecLib::ExCommonSpecInfo_t& cs, const size_t begin, const size_t end, const size_t step, std::array<std::array<size_t, 5+1>, TOP+1>& hull) {
	for (size_t i = begin; (i < end) && !cs.cancelled(); i += step) {
#ifdef ENABLE_DELAY
		mywait(DelaySeconds);
#endif
		size_t e;
		bool outside = false;
		for (e = 1 ; e < 4 ; e++) {
			const size_t psource = hull[e][source];
			assert_spec(psource < N+1);
			const double dx = input[x][psource];
			const double dy = input[y][psource];

			const size_t ptarget = hull[e][target];
			assert_spec(ptarget < N+1);
			const double px = input[x][ptarget];
			const double py = input[y][ptarget];

			const double qx = input[x][i];
			const double qy = input[y][i];

			const double determ = rightturn(dx,dy,px,py,qx,qy);
			if (determ < 0) {
				// The point is outside
				outside = true;
				break;
			}
		}

#ifndef SLNSPECDEBUG
		int count_loop1 = 0;
#endif
		while((hull[e][inch] == 0) && (outside)) {
#ifndef SLNSPECDEBUG
			assert_spec(count_loop1 < loop1c);
			++count_loop1;
#endif
			const size_t eprev = hull[e][pred];
			assert_spec(eprev < TOP+1);

			const size_t psource = hull[eprev][source];
			assert_spec(psource < N+1);
			const double dx = input[x][psource];
			const double dy = input[y][psource];

			const size_t ptarget = hull[eprev][target];
			assert_spec(ptarget < N+1);
			const double px = input[x][ptarget];
			const double py = input[y][ptarget];

			const double qx = input[x][i];
			const double qy = input[y][i];

			const double determ = rightturn(dx,dy,px,py,qx,qy);

			if (determ < 0) {
				assert_spec(e != eprev);
				e = eprev;
			} else {
				const size_t enext = hull[e][succ];
				assert_spec(enext < TOP+1);

				const size_t psource = hull[enext][source];
				assert_spec(psource < N+1);
				const double dx2 = input[x][psource];
				const double dy2 = input[y][psource];

				const size_t ptarget = hull[enext][target];
				assert_spec(ptarget < N+1);
				const double px2 = input[x][ptarget];
				const double py2 = input[y][ptarget];

				const double qx2 = input[x][i];
				const double qy2 = input[y][i];

				const double determ2 = rightturn(dx2,dy2,px2,py2,qx2,qy2);

				if (determ2 >= 0) {
					outside = false;
				} else {
					assert_spec(e != enext);
					e = enext;
				}
			}
		}

		if (outside) {
			//Compute upper tangent
			size_t enext = hull[e][succ];
			assert_spec(enext < TOP+1);

			size_t lastedge;
			bool flag;
#ifndef SLNSPECDEBUG
			int count_loop2 = 0;
#endif
			do {
#ifndef SLNSPECDEBUG
				assert_spec(count_loop2 < loop2c);
				++count_loop2;
#endif
				flag = false;
				const size_t psource = hull[enext][source];
				assert_spec(psource < N+1);
				const double dx = input[x][psource];
				const double dy = input[y][psource];

				const size_t ptarget = hull[enext][target];
				assert_spec(ptarget < N+1);
				const double px = input[x][ptarget];
				const double py = input[y][ptarget];

				const double qx = input[x][i];
				const double qy = input[y][i];

				const double determ = rightturn(dx,dy,px,py,qx,qy);

				lastedge = hull[TOP][5];
				assert_spec(lastedge+2 < TOP+1);

				if (determ <= 0) {
					hull[enext][inch] = 0;
					hull[enext][pred] = lastedge+1;
					const size_t ea = hull[enext][succ];
					assert_spec(ea < TOP+1);
					hull[enext][succ] = lastedge+2;
					assert_spec(enext != ea);
					enext = ea;
					flag = true;
				}
			}while(flag);
			// Compute lower tangent

			size_t eprev = hull[e][pred];
			assert_spec(eprev < TOP+1);

#ifndef SLNSPECDEBUG
			int count_loop3 = 0;
#endif
			do {
#ifndef SLNSPECDEBUG
				assert_spec(count_loop3 < loop3c);
				++count_loop3;
#endif
				flag = false;
				assert_spec(hull[eprev][source] < N+1);
				assert_spec(hull[eprev][target] < N+1);
				const double dx2 = input[x][hull[eprev][source]];
				const double dy2 = input[y][hull[eprev][source]];
				const double px2 = input[x][hull[eprev][target]];
				const double py2 = input[y][hull[eprev][target]];
				const double qx2 = input[x][i];
				const double qy2 = input[y][i];

				const double determ2 = rightturn(dx2,dy2,px2,py2,qx2,qy2);

				if (determ2 <= 0) {
					hull[eprev][inch] = 0;
					const size_t ea = hull[eprev][pred];
					assert_spec(ea < TOP+1);
					hull[eprev][pred] = lastedge+1;
					hull[eprev][succ] = lastedge+2;
					assert_spec(eprev != ea);
					eprev = ea;
					flag = true;
				}
			}while(flag);

			hull[e][inch] = 0;
			hull[e][pred] = lastedge+1;
			hull[e][succ] = lastedge+2;

			// Add new edges to convex hull
			lastedge++;
			assert_spec(lastedge+1 < TOP+1);
			hull[TOP][5] = lastedge;
			assert_spec(hull[eprev][target] < N+1);
			hull[lastedge][source] = hull[eprev][target];
			hull[lastedge][target] = i;
			hull[lastedge][succ] = lastedge + 1;
			hull[lastedge][pred] = eprev;
			hull[lastedge][inch] = 1;

			lastedge++;
			assert_spec(lastedge < TOP+1);
			hull[TOP][5] = lastedge;
			hull[lastedge][source] = i;
			assert_spec(hull[enext][source] < N+1);
			hull[lastedge][target] = hull[enext][source];
			hull[lastedge][succ] = enext;
			hull[lastedge][pred] = lastedge - 1;
			hull[lastedge][inch] = 1;

			// Connect to old convex hull
			hull[enext][pred] = lastedge;
			hull[eprev][succ] = lastedge - 1;

			//Update first
			hull[TOP][1] = lastedge;
		}
		// End of process, proceed with next point
	}
}

auto sf = [](const size_t i, std::array<std::array<size_t, 5+1>, TOP+1>& hull) {
#ifdef ENABLE_DELAY
	mywait(DelaySeconds);
#endif
	size_t e;
	bool outside = false;
	for (e = 1 ; e < 4 ; e++) {
		const size_t psource = hull[e][source];
		assert_spec(psource < N+1);
		const double dx = input[x][psource];
		const double dy = input[y][psource];

		const size_t ptarget = hull[e][target];
		assert_spec(ptarget < N+1);
		const double px = input[x][ptarget];
		const double py = input[y][ptarget];

		const double qx = input[x][i];
		const double qy = input[y][i];

		const double determ = rightturn(dx,dy,px,py,qx,qy);
		if (determ < 0) {
			// The point is outside
			outside = true;
			break;
		}
	}

	while((hull[e][inch] == 0) && (outside)) {
		const size_t eprev = hull[e][pred];
		assert_spec(eprev < TOP+1);

		const size_t psource = hull[eprev][source];
		assert_spec(psource < N+1);
		const double dx = input[x][psource];
		const double dy = input[y][psource];

		const size_t ptarget = hull[eprev][target];
		assert_spec(ptarget < N+1);
		const double px = input[x][ptarget];
		const double py = input[y][ptarget];

		const double qx = input[x][i];
		const double qy = input[y][i];

		const double determ = rightturn(dx,dy,px,py,qx,qy);

		if (determ < 0) {
			assert_spec(e != eprev);
			e = eprev;
		} else {
			const size_t enext = hull[e][succ];
			assert_spec(enext < TOP+1);

			const size_t psource = hull[enext][source];
			assert_spec(psource < N+1);
			const double dx2 = input[x][psource];
			const double dy2 = input[y][psource];

			const size_t ptarget = hull[enext][target];
			assert_spec(ptarget < N+1);
			const double px2 = input[x][ptarget];
			const double py2 = input[y][ptarget];

			const double qx2 = input[x][i];
			const double qy2 = input[y][i];

			const double determ2 = rightturn(dx2,dy2,px2,py2,qx2,qy2);

			if (determ2 >= 0) {
				outside = false;
			} else {
				assert_spec(e != enext);
				e = enext;
			}
		}
	}

	if (outside) {
		//Compute upper tangent
		size_t enext = hull[e][succ];
		assert_spec(enext < TOP+1);

		size_t lastedge;
		bool flag;
		do {
			flag = false;
			const size_t psource = hull[enext][source];
			assert_spec(psource < N+1);
			const double dx = input[x][psource];
			const double dy = input[y][psource];

			const size_t ptarget = hull[enext][target];
			assert_spec(ptarget < N+1);
			const double px = input[x][ptarget];
			const double py = input[y][ptarget];

			const double qx = input[x][i];
			const double qy = input[y][i];

			const double determ = rightturn(dx,dy,px,py,qx,qy);

			lastedge = hull[TOP][5];
			assert_spec(lastedge < TOP+1);

			if (determ <= 0) {
				hull[enext][inch] = 0;
				hull[enext][pred] = lastedge+1;
				const size_t ea = hull[enext][succ];
				assert_spec(ea < TOP+1);
				hull[enext][succ] = lastedge+2;
				assert_spec(enext != ea);
				enext = ea;
				flag = true;
			}
		}while(flag);
		// Compute lower tangent

		size_t eprev = hull[e][pred];
		assert_spec(eprev < TOP+1);

		do {
			flag = false;
			assert_spec(hull[eprev][source] < N+1);
			assert_spec(hull[eprev][target] < N+1);
			const double dx2 = input[x][hull[eprev][source]];
			const double dy2 = input[y][hull[eprev][source]];
			const double px2 = input[x][hull[eprev][target]];
			const double py2 = input[y][hull[eprev][target]];
			const double qx2 = input[x][i];
			const double qy2 = input[y][i];

			const double determ2 = rightturn(dx2,dy2,px2,py2,qx2,qy2);

			if (determ2 <= 0) {
				hull[eprev][inch] = 0;
				const size_t ea = hull[eprev][pred];
				assert_spec(ea < TOP+1);
				hull[eprev][pred] = lastedge+1;
				hull[eprev][succ] = lastedge+2;
				assert_spec(eprev != ea);
				eprev = ea;
				flag = true;
			}
		}while(flag);

		hull[e][inch] = 0;
		hull[e][pred] = lastedge+1;
		hull[e][succ] = lastedge+2;

		// Add new edges to convex hull
		lastedge++;
		assert_spec(lastedge < TOP+1);
		hull[TOP][5] = lastedge;
		assert_spec(hull[eprev][target] < N+1);
		hull[lastedge][source] = hull[eprev][target];
		hull[lastedge][target] = i;
		hull[lastedge][succ] = lastedge + 1;
		hull[lastedge][pred] = eprev;
		hull[lastedge][inch] = 1;

		lastedge++;
		assert_spec(lastedge < TOP+1);
		hull[TOP][5] = lastedge;
		hull[lastedge][source] = i;
		assert_spec(hull[enext][source] < N+1);
		hull[lastedge][target] = hull[enext][source];
		hull[lastedge][succ] = enext;
		hull[lastedge][pred] = lastedge - 1;
		hull[lastedge][inch] = 1;

		// Connect to old convex hull
		hull[enext][pred] = lastedge;
		hull[eprev][succ] = lastedge - 1;

		//Update first
		hull[TOP][1] = lastedge;
	}
	// End of process, proceed with next point
};

int main(int argc, char* argv[]) {
	std::array<std::array<size_t, 5+1>, TOP+1> hull = {};
	FILE *inputfile;

#if defined(DISC)
	std::string inputFileName("inputs/disc-10M.in");
#elif defined(SQUARE)
	std::string inputFileName("inputs/square-10M.in");
#elif defined(KUZMIN)
	std::string inputFileName("inputs/kuzmin_010M_limpio.dat");
#else
//#error The type of problem must be defined. Accepted values: DISC, SQUARE, KUZMIN
// Default type is DISC
	std::string inputFileName("inputs/disc-10M.in");
#endif

	process_args(argc, argv, "hc:d:m:t:s:vi:", inputFileName);

	//  Opens the input file
	if ((inputfile = fopen(inputFileName.c_str(), "r")) == NULL) {
		if ((inputfile = fopen(("../" + inputFileName).c_str(), "r")) == NULL) {
			if ((inputfile = fopen(("tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
				if ((inputfile = fopen(("../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
					if ((inputfile = fopen(("../../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
						if ((inputfile = fopen(("../../../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
							if ((inputfile = fopen(("../../../../tests/benchmarks/" + inputFileName).c_str(), "r")) == NULL) {
								std::cout << "Error reading from file" << std::endl;
								exit(-1);
							}
						}
					}
				}
			}
		}
	}
	for(size_t i = 1; i < N+1; i++) {
		if ((fscanf(inputfile, "%lf %lf", &input[x][i], &input[y][i]) < 0) && ferror(inputfile)) {
			std::cout << "Error reading numbers from file" << std::endl;
			return -1;
		}
		//std::cout << "New point: " << input[x][i] << " " << input[y][i] << std::endl;
	}

	fclose(inputfile);

	do_preheat();	// Preheat

#ifdef THREADINSTRUMENT
	ThreadInstrument::registerLogPrinter(Verbose ? SpecLib::internal::verbose_printer : SpecLib::internal::simple_printer);
#endif

	// Calculate the first determinant
	{
		const double dx = input[x][1];
		const double dy = input[y][1];
		const double px = input[x][2];
		const double py = input[y][2];
		const double qx = input[x][3];
		const double qy = input[y][3];

		const double determ = rightturn(dx,dy,px,py,qx,qy);

		if (determ == 0) {
			std::cout << "incremental: The first three points are aligned." << std::endl;
			std::cout << "incremental: We aree too lazy to proceed. Stop." << std::endl;
			return -1;
		}

		// If determinant < 0, reorder points two and three
		if (determ < 0) {
			input[x][2] = qx;
			input[y][2] = qy;
			input[x][3] = px;
			input[y][3] = py;
		}
	}

	// Links the first edge
	hull[1][source] = 1;
	hull[1][target] = 2;
	hull[1][pred] = 3;
	hull[1][succ] = 2;
	hull[1][inch] = 1;

	// Links the second edge
	hull[2][source] = 2;
	hull[2][target] = 3;
	hull[2][pred] = 1;
	hull[2][succ] = 3;
	hull[2][inch] = 1;

	// Links the third edge
	hull[3][source] = 3;
	hull[3][target] = 1;
	hull[3][pred] = 2;
	hull[3][succ] = 1;
	hull[3][inch] = 1;

	hull[TOP][1] = 1;
	hull[TOP][5] = 3;
	// Put -1 (or 0) in all remaining elements of hull, just in case.
	for (size_t i = 4 ; i < TOP ; i++) {
		for (size_t j = 1 ; j < 6 ; j++ ) {
			hull[i][j] = 0;
		}
	}
	//printf("INI: %d\n",i);
	//printf("TOP: %d\n",hull[TOP][5]);

	/************************************************************************/
	// seqcode: Start profiling with Sun Compiler
	// collector_resume();
	/************************************************************************/
	const auto time_start = profile_clock_t::now();
	// -------------------------------------------------

	// Main loop (iterating over remaining points)
#if !defined(SLSTATS) && !defined(SLMINIMALSTATS)
#ifdef SLSIMULATE
	SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, 4, N+1, 1, SpecLib::getChunkSize(N+1-4, NChunks), sf_loop, hull);
#else
	SpecLib::specRun({NThreads, MinParalThreads}, 4, N+1, 1, SpecLib::getChunkSize(N+1-4, NChunks), sf_loop, hull);
#endif
#else
#ifdef SLSIMULATE
	SpecLib::StatsRunInfo statsRes = SpecLib::specRun({NThreads, MinParalThreads, SimulatedSuccessRatio}, 4, N+1, 1, SpecLib::getChunkSize(N+1-4, NChunks), sf_loop, hull);
#else
	SpecLib::StatsRunInfo statsRes = SpecLib::specRun({NThreads, MinParalThreads}, 4, N+1, 1, SpecLib::getChunkSize(N+1-4, NChunks), sf_loop, hull);
#endif
#endif
	// -------------------------------------------------
	// speccode: time measurement and display
	const auto time_stop = profile_clock_t::now();
	const double time_spec = std::chrono::duration<double>(time_stop - time_start).count();

//	// Show the results
//	size_t e = hull[TOP][1];
//	size_t p = hull[e][source];
//
//	do {
//		std::cout << input[x][p] << " " << input[y][p] << std::endl;
//		e = hull[e][succ];
//		p = hull[e][source];
//	}while(e != hull[TOP][1]);

	std::cout << "TotalNumIters = " << (N+1-4) << ", NumChunks = " << NChunks << ", NumItersPerChunk = " << SpecLib::getChunkSize(N+1-4, NChunks) << std::endl;
	std::cout << "[PAR] Total number of edges: " << hull[TOP][5] << std::endl;

	// -------------------------------------------------
	// Print time results
	std::cerr << "------------------------------------------------" << std::endl;
	std::cerr << "[PAR] Execution time: " << time_spec << std::endl;

#if defined(SLSTATS) || defined(SLMINIMALSTATS)
	printStatsRunInfo(statsRes);
#endif

#ifdef THREADINSTRUMENT
	ThreadInstrument::dumpLog("tilog.log");
#endif

#ifdef VALIDATE
	return (seq_test_and_check(hull)) ? 0 : -1;
#else
	return 0;
#endif
}
