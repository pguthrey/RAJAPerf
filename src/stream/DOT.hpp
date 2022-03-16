//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// DOT kernel reference implementation:
///
/// for (Index_type i = ibegin; i < iend; ++i ) {
///   dot += a[i] * b[i];
/// }
///

#ifndef RAJAPerf_Stream_DOT_HPP
#define RAJAPerf_Stream_DOT_HPP

#define DOT_DATA_SETUP \
  Real_ptr a = m_a; \
  Real_ptr b = m_b;

#define DOT_BODY  \
  dot += a[i] * b[i] ;


#include "common/KernelBase.hpp"

namespace rajaperf
{
class RunParams;

namespace stream
{

class DOT : public KernelBase
{
public:

  DOT(const RunParams& params);

  ~DOT();

  void setUp(VariantID vid, size_t tid);
  void updateChecksum(VariantID vid, size_t tid);
  void tearDown(VariantID vid, size_t tid);

  void runSeqVariant(VariantID vid, size_t tid);
  void runOpenMPVariant(VariantID vid, size_t tid);
  void runCudaVariant(VariantID vid, size_t tid);
  void runHipVariant(VariantID vid, size_t tid);
  void runOpenMPTargetVariant(VariantID vid, size_t tid);

  void setCudaTuningDefinitions(VariantID vid);
  void setHipTuningDefinitions(VariantID vid);
  template < size_t block_size >
  void runCudaVariantImpl(VariantID vid);
  template < size_t block_size >
  void runHipVariantImpl(VariantID vid);

private:
  static const size_t default_gpu_block_size = 256;
  using gpu_block_sizes_type = gpu_block_size::list_type<default_gpu_block_size>;

  Real_ptr m_a;
  Real_ptr m_b;
  Real_type m_dot;
  Real_type m_dot_init;
};

} // end namespace stream
} // end namespace rajaperf

#endif // closing endif for header file include guard
