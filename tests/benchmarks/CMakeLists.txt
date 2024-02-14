# SpecLib: Library for speculative execution of loops
# Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
#
# Distributed under the MIT License. (See accompanying file LICENSE)

# \file     CMakeLists.txt
# \brief    CMakeLists.txt for the benchmarks of the project
# \author   Millan A. Martinez  <millan.alvarez@udc.es>
# \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
# \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
# \author   Francisco F. Rivera <ff.rivera@usc.es>

cmake_minimum_required( VERSION 2.8...3.28 )

set(benchmarks B-synthetic-fast_v1 D-2D-Hull-Disc-10M E-2D-Hull-Square-10M F-2D-Hull-Kuzmin-10M R-test_reduction R-test_reduction_atomic R-test_reduction_slow R-test_reduction_atomic_slow)

if(NOT SLSHOWSTATS)
  set(SLSHOWSTATS "Disabled" CACHE STRING "Print statistics after execution, options are: Disabled Minimal Detailed" )
endif(NOT SLSHOWSTATS)
set_property(CACHE SLSHOWSTATS PROPERTY STRINGS "Disabled" "Minimal" "Detailed")
if(NOT SLNOCANCEL)
  set(SLNOCANCEL OFF CACHE BOOL "Enable to suppress the cancellation check within loops" )
endif(NOT SLNOCANCEL)
if(NOT SLNOALLOCA)
  set(SLNOALLOCA OFF CACHE BOOL "Enable to suppress the use of 'alloca' in SpecVector and use 'malloc' instead" )
endif(NOT SLNOALLOCA)
if(NOT SLALLOCAINMALLOCH)
  set(SLALLOCAINMALLOCH OFF CACHE BOOL "Enable to indicate that 'alloca' function is defined in <malloc.h> instead of <alloca.h>" )
endif(NOT SLALLOCAINMALLOCH)
if(NOT ENABLE_DELAY)
  set(ENABLE_DELAY OFF CACHE BOOL "Enable artificial delay within loops" )
endif(NOT ENABLE_DELAY)
if(NOT VALIDATE)
  set(VALIDATE OFF CACHE BOOL "Enable validation in benchmarks" )
endif(NOT VALIDATE)
if(NOT SLSIMULATE)
  set(SLSIMULATE OFF CACHE BOOL "Enable simulation mode (only benchmarks)" )
endif(NOT SLSIMULATE)
if(NOT THREADLIB)
  if(NOT PTHREAD_LIB)
    set(THREADLIB "" CACHE STRING "Linking C++ library for <thread> (default: None)" )
  else(NOT PTHREAD_LIB)
    set(THREADLIB "pthread" CACHE STRING "Linking C++ library for <thread> (default: pthread)" )
  endif(NOT PTHREAD_LIB)
endif(NOT THREADLIB)

add_executable( B-synthetic-fast_v1 B-synthetic-fast_v1.cpp )
add_executable( D-2D-Hull-Disc-10M DEF-2D-Hull-10M.cpp )
add_executable( E-2D-Hull-Square-10M DEF-2D-Hull-10M.cpp )
add_executable( F-2D-Hull-Kuzmin-10M DEF-2D-Hull-10M.cpp )
add_executable( R-test_reduction R-test_reduction.cpp )
add_executable( R-test_reduction_atomic R-test_reduction_atomic.cpp )
add_executable( R-test_reduction_slow R-test_reduction_slow.cpp )
add_executable( R-test_reduction_atomic_slow R-test_reduction_atomic_slow.cpp )

target_compile_definitions( D-2D-Hull-Disc-10M PRIVATE DISC )
target_compile_definitions( E-2D-Hull-Square-10M PRIVATE SQUARE )
target_compile_definitions( F-2D-Hull-Kuzmin-10M PRIVATE KUZMIN )

foreach(benchmark ${benchmarks})
  if(SLSHOWSTATS)
    if(SLSHOWSTATS STREQUAL "Detailed")
      target_compile_definitions( ${benchmark} PRIVATE SLSTATS )
    elseif(SLSHOWSTATS STREQUAL "Minimal")
      target_compile_definitions( ${benchmark} PRIVATE SLMINIMALSTATS )
    endif(SLSHOWSTATS STREQUAL "Detailed")
  endif(SLSHOWSTATS)
  if(SLNOCANCEL)
    target_compile_definitions( ${benchmark} PRIVATE SLNOCANCEL )
  endif(SLNOCANCEL)
  
  if(SLNOALLOCA)
    target_compile_definitions( ${benchmark} PRIVATE NOALLOCA )
  endif(SLNOALLOCA)
  if(SLALLOCAINMALLOCH)
    target_compile_definitions( ${benchmark} PRIVATE ALLOCAINMALLOCH )
  endif(SLALLOCAINMALLOCH)
  
  if(ENABLE_DELAY)
    target_compile_definitions( ${benchmark} PRIVATE ENABLE_DELAY )
  endif(ENABLE_DELAY)
  if(VALIDATE)
    target_compile_definitions( ${benchmark} PRIVATE VALIDATE )
  endif(VALIDATE)
  if(SLSIMULATE)
    target_compile_definitions( ${benchmark} PRIVATE SLSIMULATE )
  endif(SLSIMULATE)
  target_include_directories( ${benchmark} PRIVATE ${PROJECT_SOURCE_DIR}/include )
  if(THREADLIB)
    string( TOLOWER "${THREADLIB}" THREADLIB_CMP )
    if(NOT(THREADLIB_CMP STREQUAL "none"))
      target_link_libraries( ${benchmark} ${THREADLIB} )
    endif(NOT(THREADLIB_CMP STREQUAL "none"))
  endif(THREADLIB)
endforeach(benchmark)
if(ATOMIC_LIB)
  target_link_libraries( R-test_reduction_atomic atomic )
  target_link_libraries( R-test_reduction_atomic_slow atomic )
endif(ATOMIC_LIB)

add_subdirectory( sequential )

install( TARGETS ${benchmarks}
         RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin )
