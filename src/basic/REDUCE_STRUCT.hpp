//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-21, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// REDUCE_STRUCT kernel reference implementation:
///
/// Real_type xsum = 0.0;
/// Real_type xmin = 0.0;
/// Real_type xmax = 0.0;
///
/// for (Index_type i = ibegin; i < iend; ++i ) {
///   xsum += x[i] ;
///   xmin = RAJA_MIN(xmin, x[i]) ;
///   xmax = RAJA_MAX(xmax, x[i]) ;
/// }
///
/// particles.xcenter += xsum;
/// particles.xcenter /= particles.N
/// particles.xmin = RAJA_MIN(m_xmin, xmin);
/// particles.xmax = RAJA_MAX(m_xmax, xmax);
///
/// RAJA_MIN/MAX are macros that do what you would expect.
///

#ifndef RAJAPerf_Basic_REDUCE_STRUCT_HPP
#define RAJAPerf_Basic_REDUCE_STRUCT_HPP


#define REDUCE_STRUCT_DATA_SETUP \
  particles_t particles; \
  particles.N = 100; \
  Real_type X_MIN = 0.0, X_MAX = 100.0; \
  Real_type Y_MIN = 0.0, Y_MAX = 50.0; \
  Real_type Lx = (X_MAX) - (X_MIN); \
  Real_type Ly = (Y_MAX) - (Y_MIN); \
  Real_type dx = Lx/(Real_type)(particles.N); \
  Real_type dy = Ly/(Real_type)(particles.N); \
  Real_type DX = dx*(particles.N-1); \
  Real_type DY = dy*(particles.N-1); 

#define REDUCE_STRUCT_BODY  \
  particles.xcenter += particles.x[i] ; \
  particles.xcenter /= particles.N \
  particles.xmin = RAJA_MIN(particles.xmin, particles.x[i]) ; \
  particles.xmax = RAJA_MAX(particles.xmax, particles.x[i]) ;

#define REDUCE_STRUCT_BODY_RAJA  \
  xsum += particles.x[i] ; \
  xmin.min(particles.x[i]) ; \
  xmax.max(particles.x[i]) ;


#include "common/KernelBase.hpp"

namespace rajaperf
{
class RunParams;

namespace basic
{

class REDUCE_STRUCT : public KernelBase
{
public:

  REDUCE_STRUCT(const RunParams& params);

  ~REDUCE_STRUCT();

  void setUp(VariantID vid);
  void updateChecksum(VariantID vid);
  void tearDown(VariantID vid);

  void runSeqVariant(VariantID vid);
  void runOpenMPVariant(VariantID vid);
  void runCudaVariant(VariantID vid);
  void runHipVariant(VariantID vid);
  void runOpenMPTargetVariant(VariantID vid);

private:
  Real_ptr m_x; Real_ptr m_y;

  struct particles_t{
    Int_type N;
    Real_ptr x, y;

    Real_ptr GetCenter(){return &center[0];};
    Real_type GetXMax(){return xmax;};
    Real_type GetXMin(){return xmin;};
    Real_type GetYMax(){return ymax;};
    Real_type GetYMin(){return ymin;};
    void SetCenter(Real_type xval, Real_type yval){this->center[0]=xval, this->center[1]=yval;};
    void SetXMin(Real_type val){this->xmin=val;};
    void SetXMax(Real_type val){this->xmax=val;};
    void SetYMin(Real_type val){this->ymin=val;};
    void SetYMax(Real_type val){this->ymax=val;};
    //results
    private:
    Real_type center[2];
    Real_type xmin, xmax;
    Real_type ymin, ymax;
    }; 
};

} // end namespace basic
} // end namespace rajaperf

#endif // closing endif for header file include guard