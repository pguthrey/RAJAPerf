#!/usr/bin/env bash

###############################################################################
# Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
# and RAJA project contributors. See the RAJAPerf/LICENSE file for details.
#
# SPDX-License-Identifier: (BSD-3-Clause)
###############################################################################

if [ "$1" == "" ]; then
  echo
  echo "You must pass a compiler version number to script. For example,"
  echo "    toss3_gcc.sh 8.3.1"
  exit
fi

COMP_VER=$1
shift 1

BUILD_SUFFIX=lc_toss3-gcc-${COMP_VER}
RAJA_HOSTCONFIG=../tpl/RAJA/host-configs/lc-builds/toss3/gcc_X.cmake

echo
echo "Creating build directory build_${BUILD_SUFFIX} and generating configuration in it"
echo "Configuration extra arguments:"
echo "   $@"
echo

rm -rf build_${BUILD_SUFFIX} 2>/dev/null
mkdir build_${BUILD_SUFFIX} && cd build_${BUILD_SUFFIX}

module load cmake/3.14.5

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=/usr/tce/packages/gcc/gcc-${COMP_VER}/bin/g++ \
  -DBLT_CXX_STD=c++14 \
  -C ${RAJA_HOSTCONFIG} \
  -DENABLE_OPENMP=On \
  -DCMAKE_INSTALL_PREFIX=../install_${BUILD_SUFFIX} \
  "$@" \
  ..

echo
echo "***********************************************************************"
echo "cd into directory build_${BUILD_SUFFIX} and run make to build RAJA Perf Suite"
echo "***********************************************************************"
