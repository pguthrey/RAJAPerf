//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "TRAP_INT.hpp"

#include "RAJA/RAJA.hpp"

#if defined(RAJA_ENABLE_CUDA)

#include "common/CudaDataUtils.hpp"

#include <iostream>

namespace rajaperf
{
namespace basic
{

//
// Function used in TRAP_INT loop.
//
RAJA_INLINE
RAJA_DEVICE
Real_type trap_int_func(Real_type x,
                        Real_type y,
                        Real_type xp,
                        Real_type yp)
{
   Real_type denom = (x - xp)*(x - xp) + (y - yp)*(y - yp);
   denom = 1.0/sqrt(denom);
   return denom;
}


#define TRAP_INT_DATA_SETUP_CUDA  // nothing to do here...

#define TRAP_INT_DATA_TEARDOWN_CUDA // nothing to do here...

#define TRAP_INT_BODY_CUDA(atomicAdd) \
  RAJAPERF_REDUCE_1_CUDA(Real_type, TRAP_INT_VAL, sumx, 0.0, RAJAPERF_ADD_OP, atomicAdd)

template < size_t block_size >
__launch_bounds__(block_size)
__global__ void trapint(Real_type x0, Real_type xp,
                        Real_type y, Real_type yp,
                        Real_type h,
                        Real_ptr sumx,
                        Index_type iend)
{
  TRAP_INT_BODY_CUDA(::atomicAdd)
}



template < size_t block_size >
void TRAP_INT::runCudaVariantReduceAtomic(VariantID vid)
{
  const Index_type run_reps = getRunReps();
  const Index_type ibegin = 0;
  const Index_type iend = getActualProblemSize();

  TRAP_INT_DATA_SETUP;

  if ( vid == Base_CUDA ) {

    TRAP_INT_DATA_SETUP_CUDA;

    Real_ptr sumx;
    allocAndInitCudaDeviceData(sumx, &sumx_init, 1);

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      initCudaDeviceData(sumx, &sumx_init, 1);

      const size_t grid_size = RAJA_DIVIDE_CEILING_INT(iend, block_size);
      trapint<block_size><<<grid_size, block_size,
                sizeof(Real_type)*block_size>>>(x0, xp,
                                                y, yp,
                                                h,
                                                sumx,
                                                iend);
      cudaErrchk( cudaGetLastError() );

      Real_type lsumx;
      Real_ptr plsumx = &lsumx;
      getCudaDeviceData(plsumx, sumx, 1);
      m_sumx += lsumx * h;

    }
    stopTimer();

    deallocCudaDeviceData(sumx);

    TRAP_INT_DATA_TEARDOWN_CUDA;

  } else if ( vid == Lambda_CUDA ) {

    TRAP_INT_DATA_SETUP_CUDA;

    Real_ptr sumx;
    allocAndInitCudaDeviceData(sumx, &sumx_init, 1);

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      initCudaDeviceData(sumx, &sumx_init, 1);

      auto trapint_lam = [=] __device__ () {
        TRAP_INT_BODY_CUDA(::atomicAdd)
      };

      const size_t grid_size = RAJA_DIVIDE_CEILING_INT(iend, block_size);
      lambda_cuda<block_size><<<grid_size, block_size,
                sizeof(Real_type)*block_size>>>(trapint_lam);
      cudaErrchk( cudaGetLastError() );

      Real_type lsumx;
      Real_ptr plsumx = &lsumx;
      getCudaDeviceData(plsumx, sumx, 1);
      m_sumx += lsumx * h;

    }
    stopTimer();

    deallocCudaDeviceData(sumx);

    TRAP_INT_DATA_TEARDOWN_CUDA;

  } else if ( vid == RAJA_CUDA ) {

    TRAP_INT_DATA_SETUP_CUDA;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      RAJA::ReduceSum<RAJA::cuda_reduce_atomic, Real_type> sumx(sumx_init);

      RAJA::forall< RAJA::cuda_exec<block_size, true /*async*/> >(
        RAJA::RangeSegment(ibegin, iend), [=] __device__ (Index_type i) {
        TRAP_INT_BODY;
      });

      m_sumx += static_cast<Real_type>(sumx.get()) * h;

    }
    stopTimer();

    TRAP_INT_DATA_TEARDOWN_CUDA;

  } else {
     getCout() << "\n  TRAP_INT : Unknown Cuda variant id = " << vid << std::endl;
  }
}

template < size_t block_size >
void TRAP_INT::runCudaVariantReduce(VariantID vid)
{
  const Index_type run_reps = getRunReps();
  const Index_type ibegin = 0;
  const Index_type iend = getActualProblemSize();

  TRAP_INT_DATA_SETUP;

  if ( vid == RAJA_CUDA ) {

    TRAP_INT_DATA_SETUP_CUDA;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      RAJA::ReduceSum<RAJA::cuda_reduce, Real_type> sumx(sumx_init);

      RAJA::forall< RAJA::cuda_exec<block_size, true /*async*/> >(
        RAJA::RangeSegment(ibegin, iend), [=] __device__ (Index_type i) {
        TRAP_INT_BODY;
      });

      m_sumx += static_cast<Real_type>(sumx.get()) * h;

    }
    stopTimer();

    TRAP_INT_DATA_TEARDOWN_CUDA;

  } else {
     getCout() << "\n  TRAP_INT : Unknown Cuda variant id = " << vid << std::endl;
  }
}

void TRAP_INT::runCudaVariant(VariantID vid, size_t tune_idx)
{
  size_t t = 0;
  seq_for(gpu_block_sizes_type{}, [&](auto block_size) {
    if (run_params.numValidGPUBlockSize() == 0u ||
        run_params.validGPUBlockSize(block_size)) {
      if (tune_idx == t) {
        runCudaVariantReduceAtomic<block_size>(vid);
      }
      t += 1;
    }
  });
  if ( vid == RAJA_CUDA ) {
    seq_for(gpu_block_sizes_type{}, [&](auto block_size) {
      if (run_params.numValidGPUBlockSize() == 0u ||
          run_params.validGPUBlockSize(block_size)) {
        if (tune_idx == t) {
          runCudaVariantReduce<block_size>(vid);
        }
        t += 1;
      }
    });
  }
}

void TRAP_INT::setCudaTuningDefinitions(VariantID vid)
{
  seq_for(gpu_block_sizes_type{}, [&](auto block_size) {
    if (run_params.numValidGPUBlockSize() == 0u ||
        run_params.validGPUBlockSize(block_size)) {
      addVariantTuningName(vid, "reduceAtomic_"+std::to_string(block_size));
    }
  });
  if ( vid == RAJA_CUDA ) {
    seq_for(gpu_block_sizes_type{}, [&](auto block_size) {
      if (run_params.numValidGPUBlockSize() == 0u ||
          run_params.validGPUBlockSize(block_size)) {
        addVariantTuningName(vid, "reduce_"+std::to_string(block_size));
      }
    });
  }
}

} // end namespace basic
} // end namespace rajaperf

#endif  // RAJA_ENABLE_CUDA
