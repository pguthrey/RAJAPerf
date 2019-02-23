//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-19, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-738930
//
// All rights reserved.
//
// This file is part of the RAJA Performance Suite.
//
// For details about use and distribution, please read RAJAPerf/LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
///
/// POLYBENCH_FLOYD_WARSHALL kernel reference implementation:
///
/// Note: kernel is altered to enable parallelism (original did not have
///       separate input and output arrays).
///
/// for (Index_type k = 0; k < N; k++) {
///   for (Index_type i = 0; i < N; i++) {
///     for (Index_type j = 0; j < N; j++) {
///       pout[i][j] = pin[i][j] < pin[i][k] + pin[k][j] ?
///                    pin[i][j] : pin[i][k] + pin[k][j]; 
///     } 
///   } 
/// }


#ifndef RAJAPerf_POLYBENCH_FLOYD_WARSHALL_HPP
#define RAJAPerf_POLYBENCH_FLOYD_WARSHALL_HPP

#define POLYBENCH_FLOYD_WARSHALL_BODY \
  pout[j + i*N] = pin[j + i*N] < pin[k + i*N] + pin[j + k*N] ? \
                  pin[j + i*N] : pin[k + i*N] + pin[j + k*N];


#define POLYBENCH_FLOYD_WARSHALL_BODY_RAJA \
  poutview(i, j) = pinview(i, j) < pinview(i, k) + pinview(k, j) ? \
                   pinview(i, j) : pinview(i, k) + pinview(k, j);


#define POLYBENCH_FLOYD_WARSHALL_VIEWS_RAJA \
  using VIEW_TYPE = RAJA::View<Real_type, \
                               RAJA::Layout<2, Index_type, 1>>; \
\
  VIEW_TYPE pinview(pin, RAJA::Layout<2>(N, N)); \
  VIEW_TYPE poutview(pout, RAJA::Layout<2>(N, N));


#include "common/KernelBase.hpp"

namespace rajaperf 
{

class RunParams;

namespace polybench
{

class POLYBENCH_FLOYD_WARSHALL : public KernelBase
{
public:

  POLYBENCH_FLOYD_WARSHALL(const RunParams& params);

  ~POLYBENCH_FLOYD_WARSHALL();


  void setUp(VariantID vid);
  void runKernel(VariantID vid); 
  void updateChecksum(VariantID vid);
  void tearDown(VariantID vid);
  void runCudaVariant(VariantID vid);
  void runOpenMPTargetVariant(VariantID vid);

private:
  Index_type m_N;

  Real_ptr m_pin;
  Real_ptr m_pout;
};

} // end namespace polybench
} // end namespace rajaperf

#endif // closing endif for header file include guard
