//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "REDUCE_STRUCT.hpp"

#include "RAJA/RAJA.hpp"

#include "common/DataUtils.hpp"

#include <limits>

namespace rajaperf
{
namespace basic
{


REDUCE_STRUCT::REDUCE_STRUCT(const RunParams& params)
  : KernelBase(rajaperf::Basic_REDUCE_STRUCT, params)
{
  setDefaultProblemSize(1000000);
//setDefaultReps(5000);
// Set reps to low value until we resolve RAJA omp-target
// reduction performance issues
  setDefaultReps(50);

  setActualProblemSize( getTargetProblemSize() );

  setItsPerRep( getActualProblemSize() );
  setKernelsPerRep(1);
  setBytesPerRep( 6*sizeof(Real_type) + 2*sizeof(Real_type)*getActualProblemSize());
  setFLOPsPerRep(2 * getActualProblemSize() + 2);
    

  setUsesFeature(Forall);
  setUsesFeature(Reduction);

  setVariantDefined( Base_Seq );
  setVariantDefined( Lambda_Seq );
  setVariantDefined( RAJA_Seq );

  setVariantDefined( Base_OpenMP );
  setVariantDefined( Lambda_OpenMP );
  setVariantDefined( RAJA_OpenMP );

  setVariantDefined( Base_OpenMPTarget );
  setVariantDefined( RAJA_OpenMPTarget );

  setVariantDefined( Base_CUDA );
  setVariantDefined( RAJA_CUDA );

  setVariantDefined( Base_HIP );
  setVariantDefined( RAJA_HIP );
}

REDUCE_STRUCT::~REDUCE_STRUCT()
{
}

void REDUCE_STRUCT::setUp(VariantID vid, size_t RAJAPERF_UNUSED_ARG(tune_idx))
{
  allocAndInitData(m_x, getActualProblemSize(), vid);
  allocAndInitData(m_y, getActualProblemSize(), vid);
  Real_type dx = Lx/(Real_type)(getActualProblemSize());
  Real_type dy = Ly/(Real_type)(getActualProblemSize());
  for (int i=0;i<getActualProblemSize();i++){ \
      m_x[i] = i*dx;  
      m_y[i] = i*dy; 
  } 
}

void REDUCE_STRUCT::updateChecksum(VariantID vid, size_t tune_idx)
{
  checksum[vid][tune_idx] += m_particles.GetCenter()[0];
  checksum[vid][tune_idx] += m_particles.GetXMin();
  checksum[vid][tune_idx] += m_particles.GetXMax();
  checksum[vid][tune_idx] += m_particles.GetCenter()[1];
  checksum[vid][tune_idx] += m_particles.GetYMin();
  checksum[vid][tune_idx] += m_particles.GetYMax();

  return;
}

void REDUCE_STRUCT::tearDown(VariantID vid, size_t RAJAPERF_UNUSED_ARG(tune_idx))
{
  (void) vid;
  deallocData(m_x);
  deallocData(m_y);
}

} // end namespace basic
} // end namespace rajaperf
