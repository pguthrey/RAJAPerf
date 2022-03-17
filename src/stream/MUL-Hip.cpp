//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "MUL.hpp"

#include "RAJA/RAJA.hpp"

#if defined(RAJA_ENABLE_HIP)

#include "common/HipDataUtils.hpp"

#include <iostream>

namespace rajaperf
{
namespace stream
{

#define MUL_DATA_SETUP_HIP \
  allocAndInitHipDeviceData(b, m_b, iend); \
  allocAndInitHipDeviceData(c, m_c, iend);

#define MUL_DATA_TEARDOWN_HIP \
  getHipDeviceData(m_b, b, iend); \
  deallocHipDeviceData(b); \
  deallocHipDeviceData(c)

template < size_t block_size >
__launch_bounds__(block_size)
__global__ void mul(Real_ptr b, Real_ptr c, Real_type alpha,
                    Index_type iend)
{
  Index_type i = blockIdx.x * block_size + threadIdx.x;
  if (i < iend) {
    MUL_BODY;
  }
}


template < size_t block_size >
void MUL::runHipVariantImpl(VariantID vid)
{
  const Index_type run_reps = getRunReps();
  const Index_type ibegin = 0;
  const Index_type iend = getActualProblemSize();

  MUL_DATA_SETUP;

  if ( vid == Base_HIP ) {

    MUL_DATA_SETUP_HIP;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      const size_t grid_size = RAJA_DIVIDE_CEILING_INT(iend, block_size);
      hipLaunchKernelGGL((mul<block_size>), dim3(grid_size), dim3(block_size), 0, 0,  b, c, alpha,
                                      iend );
      hipErrchk( hipGetLastError() );

    }
    stopTimer();

    MUL_DATA_TEARDOWN_HIP;

  } else if ( vid == Lambda_HIP ) {

    MUL_DATA_SETUP_HIP;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      auto mul_lambda = [=] __device__ (Index_type i) {
        MUL_BODY;
      };

      const size_t grid_size = RAJA_DIVIDE_CEILING_INT(iend, block_size);
      hipLaunchKernelGGL((lambda_hip_forall<block_size, decltype(mul_lambda)>),
        grid_size, block_size, 0, 0, ibegin, iend, mul_lambda);
      hipErrchk( hipGetLastError() );

    }
    stopTimer();

    MUL_DATA_TEARDOWN_HIP;

  } else if ( vid == RAJA_HIP ) {

    MUL_DATA_SETUP_HIP;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      RAJA::forall< RAJA::hip_exec<block_size, true /*async*/> >(
        RAJA::RangeSegment(ibegin, iend), [=] __device__ (Index_type i) {
        MUL_BODY;
      });

    }
    stopTimer();

    MUL_DATA_TEARDOWN_HIP;

  } else {
     getCout() << "\n  MUL : Unknown Hip variant id = " << vid << std::endl;
  }
}

void MUL::runHipVariant(VariantID vid, size_t tune_idx)
{
  size_t t = 0;
  seq_for(gpu_block_sizes_type{}, [&](auto block_size) {
    if (run_params.numValidGPUBlockSize() == 0u ||
        run_params.validGPUBlockSize(block_size)) {
      if (tune_idx == t) {
        runHipVariantImpl<block_size>(vid);
      }
      t += 1;
    }
  });
}

void MUL::setHipTuningDefinitions(VariantID vid)
{
  seq_for(gpu_block_sizes_type{}, [&](auto block_size) {
    if (run_params.numValidGPUBlockSize() == 0u ||
        run_params.validGPUBlockSize(block_size)) {
      addVariantTuningName(vid, "block_"+std::to_string(block_size));
    }
  });
}

} // end namespace stream
} // end namespace rajaperf

#endif  // RAJA_ENABLE_HIP
