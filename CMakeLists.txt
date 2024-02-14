# SpecLib: Library for speculative execution of loops
# Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
#
# Distributed under the MIT License. (See accompanying file LICENSE)

# \file     CMakeLists.txt
# \brief    Main CMakeLists.txt file
# \author   Millan A. Martinez  <millan.alvarez@udc.es>
# \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
# \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
# \author   Francisco F. Rivera <ff.rivera@usc.es>

cmake_minimum_required( VERSION 2.8...3.28 )

project( speclib )

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(NOT CMAKE_CXX_STANDARD)
  set( CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard to be used" )
endif(NOT CMAKE_CXX_STANDARD)

set(CMAKE_FIND_LIBRARY_SUFFIXES .so ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES .dll ${CMAKE_FIND_LIBRARY_SUFFIXES})

find_library(ATOMIC_LIB
              NAMES libatomic.so libatomic.so.1 libatomic.dll libatomic-1.dll libatomic.1.dll )
find_library(PTHREAD_LIB
              NAMES pthread libpthread libpthread.so libpthread.so.0 libpthread.so.1 libwinpthread
                    libpthread.dll libpthread-0.dll libpthread.0.dll libpthread-1.dll libpthread.1.dll
                    libwinpthread.dll libwinpthread-0.dll libwinpthread.0.dll libwinpthread-1.dll libwinpthread.1.dll)
if(NOT ATOMIC_LIB)
  find_file(ATOMIC_LIB
          NAMES libatomic.so libatomic.so.1 libatomic.dll libatomic-1.dll libatomic.1.dll )
endif(NOT ATOMIC_LIB)
if(NOT PTHREAD_LIB)
  find_file(PTHREAD_LIB
          NAMES libpthread.so libpthread.so.0 libpthread.so.1
                libpthread.dll libpthread-0.dll libpthread.0.dll libpthread-1.dll libpthread.1.dll
                libwinpthread.dll libwinpthread-0.dll libwinpthread.0.dll libwinpthread-1.dll libwinpthread.1.dll)
endif(NOT PTHREAD_LIB)

add_subdirectory( tests )

install( DIRECTORY ${PROJECT_SOURCE_DIR}/include/speclib
         DESTINATION include
         FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" )
