//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-21, Lawrence Livermore National Security, LLC
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
  setDefaultProblemSize(100);
//setDefaultReps(5000);
// Set reps to low value until we resolve RAJA omp-target
// reduction performance issues
  setDefaultReps(1);

  setActualProblemSize( 100 );

  setItsPerRep( 1 );
  setKernelsPerRep(1);
  setBytesPerRep( (6*sizeof(Real_type) + 6*sizeof(Real_type)) +
                  (0*sizeof(Real_type) + 1*sizeof(Real_type)) * getActualProblemSize() );
  setFLOPsPerRep(1 * getActualProblemSize() + 1);

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

void REDUCE_STRUCT::setUp(VariantID vid)
{
  allocAndInitData(m_x, getActualProblemSize(), vid);
  allocAndInitData(m_y, getActualProblemSize(), vid);
}

void REDUCE_STRUCT::updateChecksum(VariantID vid)
{
  return;
}

void REDUCE_STRUCT::tearDown(VariantID vid)
{
  (void) vid;
  deallocData(m_x);
  deallocData(m_y);
}

} // end namespace basic
} // end namespace rajaperf