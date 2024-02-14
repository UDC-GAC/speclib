# SpecLib: Library for speculative execution of loops
# Copyright (C) 2023 Millan A. Martinez, Basilio B. Fraguela, Jose C. Cabaleiro, Francisco F. Rivera. Universidade da Coruna
#
# Distributed under the MIT License. (See accompanying file LICENSE)

# \file     CMakeLists.txt
# \brief    CMakeLists.txt for the sequential benchmarks of the project
# \author   Millan A. Martinez  <millan.alvarez@udc.es>
# \author   Basilio B. Fraguela <basilio.fraguela@udc.es>
# \author   Jose C. Cabaleiro   <jc.cabaleiro@usc.es>
# \author   Francisco F. Rivera <ff.rivera@usc.es>

cmake_minimum_required( VERSION 2.8...3.28 )

set(sequentials B-synthetic-fast_v1_seq D-2D-Hull-Disc-10M_seq E-2D-Hull-Square-10M_seq F-2D-Hull-Kuzmin-10M_seq R-test_reduction_seq)

add_executable( B-synthetic-fast_v1_seq B-synthetic-fast_v1_seq.cpp )
add_executable( D-2D-Hull-Disc-10M_seq DEF-2D-Hull-10M_seq.cpp )
add_executable( E-2D-Hull-Square-10M_seq DEF-2D-Hull-10M_seq.cpp )
add_executable( F-2D-Hull-Kuzmin-10M_seq DEF-2D-Hull-10M_seq.cpp )
add_executable( R-test_reduction_seq R-test_reduction_seq.cpp )

target_compile_definitions( D-2D-Hull-Disc-10M_seq PRIVATE DISC )
target_compile_definitions( E-2D-Hull-Square-10M_seq PRIVATE SQUARE )
target_compile_definitions( F-2D-Hull-Kuzmin-10M_seq PRIVATE KUZMIN )

install( TARGETS ${sequentials}
         RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/bin )
