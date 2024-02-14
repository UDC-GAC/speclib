## SpecLib </p>

This thread-level speculation (TLS) library provides a framework to parallelize loops in shared memory systems with automatic speculative parallelization techniques based on C++ threads.

### Requirements and how to use

This is a header-only library and only uses elements from the standard library, thus it has no extra dependencies. You only need to import the files from the `include` directory in your project. All files from `include` folder are needed, but you only must include in your code the main file `speclib\speclib.h` (e.g. `#include "speclib/speclib.h"`).

The minimum version of C++ required by the library is C++14 (e.g. `-std=c++14`).

The provided tests and benchmarks can be compiled and executed using `cmake` and/or `ccmake`.

### Examples and documentation

- The `doc` directory contains the supporting documentation for the use and understanding of the library.

- The `include` subfolder contains all the files needed to use the library.

- The `tests/basic_tests` directory contains some basic test examples that allow you to quickly test the correct operation of the library.

- The `tests/benchmarks` directory contains some benchmark examples that can be used to evaluate the library performance. It also contains their sequential versions.

- The `SpecLib` library and its model are described and compared with other alternatives in the publication [A new thread-level speculative automatic parallelization model and library based on duplicate code execution](https://www.researchsquare.com/article/rs-3405920/v1) ([DOI 10.21203/rs.3.rs-3405920/v1](https://doi.org/10.21203/rs.3.rs-3405920/v1)).
