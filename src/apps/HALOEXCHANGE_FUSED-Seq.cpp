//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "HALOEXCHANGE_FUSED.hpp"

#include "RAJA/RAJA.hpp"

#include <iostream>

namespace rajaperf
{
namespace apps
{


void HALOEXCHANGE_FUSED::runSeqVariantDefault(VariantID vid)
{
  const Index_type run_reps = getRunReps();

  HALOEXCHANGE_FUSED_DATA_SETUP;

  switch ( vid ) {

    case Base_Seq : {

      HALOEXCHANGE_FUSED_MANUAL_FUSER_SETUP;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        Index_type pack_index = 0;

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = pack_index_lists[l];
          Index_type  len  = pack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            pack_ptr_holders[pack_index] = ptr_holder{buffer, list, var};
            pack_lens[pack_index]        = len;
            pack_index += 1;
            buffer += len;
          }
        }
        for (Index_type j = 0; j < pack_index; j++) {
          Real_ptr   buffer = pack_ptr_holders[j].buffer;
          Int_ptr    list   = pack_ptr_holders[j].list;
          Real_ptr   var    = pack_ptr_holders[j].var;
          Index_type len    = pack_lens[j];
          for (Index_type i = 0; i < len; i++) {
            HALOEXCHANGE_FUSED_PACK_BODY;
          }
        }

        Index_type unpack_index = 0;

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = unpack_index_lists[l];
          Index_type  len  = unpack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            unpack_ptr_holders[unpack_index] = ptr_holder{buffer, list, var};
            unpack_lens[unpack_index]        = len;
            unpack_index += 1;
            buffer += len;
          }
        }
        for (Index_type j = 0; j < unpack_index; j++) {
          Real_ptr   buffer = unpack_ptr_holders[j].buffer;
          Int_ptr    list   = unpack_ptr_holders[j].list;
          Real_ptr   var    = unpack_ptr_holders[j].var;
          Index_type len    = unpack_lens[j];
          for (Index_type i = 0; i < len; i++) {
            HALOEXCHANGE_FUSED_UNPACK_BODY;
          }
        }

      }
      stopTimer();

      HALOEXCHANGE_FUSED_MANUAL_FUSER_TEARDOWN;

      break;
    }

#if defined(RUN_RAJA_SEQ)
    case Lambda_Seq : {

      HALOEXCHANGE_FUSED_MANUAL_LAMBDA_FUSER_SETUP;

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        Index_type pack_index = 0;

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = pack_index_lists[l];
          Index_type  len  = pack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            new(&pack_lambdas[pack_index]) pack_lambda_type(make_pack_lambda(buffer, list, var));
            pack_lens[pack_index] = len;
            pack_index += 1;
            buffer += len;
          }
        }
        for (Index_type j = 0; j < pack_index; j++) {
          auto       pack_lambda = pack_lambdas[j];
          Index_type len         = pack_lens[j];
          for (Index_type i = 0; i < len; i++) {
            pack_lambda(i);
          }
        }

        Index_type unpack_index = 0;

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = unpack_index_lists[l];
          Index_type  len  = unpack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            new(&unpack_lambdas[unpack_index]) unpack_lambda_type(make_unpack_lambda(buffer, list, var));
            unpack_lens[unpack_index] = len;
            unpack_index += 1;
            buffer += len;
          }
        }
        for (Index_type j = 0; j < unpack_index; j++) {
          auto       unpack_lambda = unpack_lambdas[j];
          Index_type len           = unpack_lens[j];
          for (Index_type i = 0; i < len; i++) {
            unpack_lambda(i);
          }
        }

      }
      stopTimer();

      HALOEXCHANGE_FUSED_MANUAL_LAMBDA_FUSER_TEARDOWN;

      break;
    }

    case RAJA_Seq : {

      using AllocatorHolder = RAJAPoolAllocatorHolder<
        RAJA::basic_mempool::MemPool<RAJA::basic_mempool::generic_allocator>>;
      using Allocator = AllocatorHolder::Allocator<char>;

      AllocatorHolder allocatorHolder;

      using range_segment = RAJA::TypedRangeSegment<Index_type>;

      using workgroup_policy = RAJA::WorkGroupPolicy <
                                   RAJA::loop_work,
                                   RAJA::ordered,
                                   RAJA::constant_stride_array_of_objects,
                                   RAJA::direct_dispatch<camp::list<range_segment, Packer>,
                                                         camp::list<range_segment, UnPacker>> >;

      using workpool = RAJA::WorkPool< workgroup_policy,
                                       Index_type,
                                       RAJA::xargs<>,
                                       Allocator >;

      using workgroup = RAJA::WorkGroup< workgroup_policy,
                                         Index_type,
                                         RAJA::xargs<>,
                                         Allocator >;

      using worksite = RAJA::WorkSite< workgroup_policy,
                                       Index_type,
                                       RAJA::xargs<>,
                                       Allocator >;

      workpool pool_pack  (allocatorHolder.template getAllocator<char>());
      workpool pool_unpack(allocatorHolder.template getAllocator<char>());
      pool_pack.reserve(num_neighbors * num_vars, 1024ull*1024ull);
      pool_unpack.reserve(num_neighbors * num_vars, 1024ull*1024ull);

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = pack_index_lists[l];
          Index_type  len  = pack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            pool_pack.enqueue(range_segment(0, len), Packer{buffer, var, list});
            buffer += len;
          }
        }
        workgroup group_pack = pool_pack.instantiate();
        worksite site_pack = group_pack.run();

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = unpack_index_lists[l];
          Index_type  len  = unpack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            pool_unpack.enqueue(range_segment(0, len), UnPacker{buffer, var, list});
            buffer += len;
          }
        }
        workgroup group_unpack = pool_unpack.instantiate();
        worksite site_unpack = group_unpack.run();

      }
      stopTimer();

      break;
    }
#endif // RUN_RAJA_SEQ

    default : {
      getCout() << "\n HALOEXCHANGE_FUSED : Unknown variant id = " << vid << std::endl;
    }

  }

}

void HALOEXCHANGE_FUSED::runSeqVariantFuncPtr(VariantID vid)
{
  const Index_type run_reps = getRunReps();

  HALOEXCHANGE_FUSED_DATA_SETUP;

  switch ( vid ) {

#if defined(RUN_RAJA_SEQ)
    case RAJA_Seq : {

      using AllocatorHolder = RAJAPoolAllocatorHolder<
        RAJA::basic_mempool::MemPool<RAJA::basic_mempool::generic_allocator>>;
      using Allocator = AllocatorHolder::Allocator<char>;

      AllocatorHolder allocatorHolder;

      using workgroup_policy = RAJA::WorkGroupPolicy <
                                   RAJA::loop_work,
                                   RAJA::ordered,
                                   RAJA::constant_stride_array_of_objects,
                                   RAJA::indirect_function_call_dispatch >;

      using workpool = RAJA::WorkPool< workgroup_policy,
                                       Index_type,
                                       RAJA::xargs<>,
                                       Allocator >;

      using workgroup = RAJA::WorkGroup< workgroup_policy,
                                         Index_type,
                                         RAJA::xargs<>,
                                         Allocator >;

      using worksite = RAJA::WorkSite< workgroup_policy,
                                       Index_type,
                                       RAJA::xargs<>,
                                       Allocator >;

      workpool pool_pack  (allocatorHolder.template getAllocator<char>());
      workpool pool_unpack(allocatorHolder.template getAllocator<char>());
      pool_pack.reserve(num_neighbors * num_vars, 1024ull*1024ull);
      pool_unpack.reserve(num_neighbors * num_vars, 1024ull*1024ull);

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = pack_index_lists[l];
          Index_type  len  = pack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            auto haloexchange_fused_pack_base_lam = [=](Index_type i) {
                  HALOEXCHANGE_FUSED_PACK_BODY;
                };
            pool_pack.enqueue(
                RAJA::TypedRangeSegment<Index_type>(0, len),
                haloexchange_fused_pack_base_lam );
            buffer += len;
          }
        }
        workgroup group_pack = pool_pack.instantiate();
        worksite site_pack = group_pack.run();

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = unpack_index_lists[l];
          Index_type  len  = unpack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            auto haloexchange_fused_unpack_base_lam = [=](Index_type i) {
                  HALOEXCHANGE_FUSED_UNPACK_BODY;
                };
            pool_unpack.enqueue(
                RAJA::TypedRangeSegment<Index_type>(0, len),
                haloexchange_fused_unpack_base_lam );
            buffer += len;
          }
        }
        workgroup group_unpack = pool_unpack.instantiate();
        worksite site_unpack = group_unpack.run();

      }
      stopTimer();

      break;
    }
#endif // RUN_RAJA_SEQ

    default : {
      getCout() << "\n HALOEXCHANGE_FUSED : Unknown variant id = " << vid << std::endl;
    }

  }

}

void HALOEXCHANGE_FUSED::runSeqVariantVirtFunc(VariantID vid)
{
  const Index_type run_reps = getRunReps();

  HALOEXCHANGE_FUSED_DATA_SETUP;

  switch ( vid ) {

#if defined(RUN_RAJA_SEQ)
    case RAJA_Seq : {

      using AllocatorHolder = RAJAPoolAllocatorHolder<
        RAJA::basic_mempool::MemPool<RAJA::basic_mempool::generic_allocator>>;
      using Allocator = AllocatorHolder::Allocator<char>;

      AllocatorHolder allocatorHolder;

      using workgroup_policy = RAJA::WorkGroupPolicy <
                                   RAJA::loop_work,
                                   RAJA::ordered,
                                   RAJA::constant_stride_array_of_objects,
                                   RAJA::indirect_virtual_function_dispatch >;

      using workpool = RAJA::WorkPool< workgroup_policy,
                                       Index_type,
                                       RAJA::xargs<>,
                                       Allocator >;

      using workgroup = RAJA::WorkGroup< workgroup_policy,
                                         Index_type,
                                         RAJA::xargs<>,
                                         Allocator >;

      using worksite = RAJA::WorkSite< workgroup_policy,
                                       Index_type,
                                       RAJA::xargs<>,
                                       Allocator >;

      workpool pool_pack  (allocatorHolder.template getAllocator<char>());
      workpool pool_unpack(allocatorHolder.template getAllocator<char>());
      pool_pack.reserve(num_neighbors * num_vars, 1024ull*1024ull);
      pool_unpack.reserve(num_neighbors * num_vars, 1024ull*1024ull);

      startTimer();
      for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = pack_index_lists[l];
          Index_type  len  = pack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            auto haloexchange_fused_pack_base_lam = [=](Index_type i) {
                  HALOEXCHANGE_FUSED_PACK_BODY;
                };
            pool_pack.enqueue(
                RAJA::TypedRangeSegment<Index_type>(0, len),
                haloexchange_fused_pack_base_lam );
            buffer += len;
          }
        }
        workgroup group_pack = pool_pack.instantiate();
        worksite site_pack = group_pack.run();

        for (Index_type l = 0; l < num_neighbors; ++l) {
          Real_ptr buffer = buffers[l];
          Int_ptr list = unpack_index_lists[l];
          Index_type  len  = unpack_index_list_lengths[l];
          for (Index_type v = 0; v < num_vars; ++v) {
            Real_ptr var = vars[v];
            auto haloexchange_fused_unpack_base_lam = [=](Index_type i) {
                  HALOEXCHANGE_FUSED_UNPACK_BODY;
                };
            pool_unpack.enqueue(
                RAJA::TypedRangeSegment<Index_type>(0, len),
                haloexchange_fused_unpack_base_lam );
            buffer += len;
          }
        }
        workgroup group_unpack = pool_unpack.instantiate();
        worksite site_unpack = group_unpack.run();

      }
      stopTimer();

      break;
    }
#endif // RUN_RAJA_SEQ

    default : {
      getCout() << "\n HALOEXCHANGE_FUSED : Unknown variant id = " << vid << std::endl;
    }

  }

}

void HALOEXCHANGE_FUSED::runSeqVariant(VariantID vid, size_t tune_idx)
{
  size_t t = 0;

  if (tune_idx == t) {

    runSeqVariantDefault(vid);

  }

  t += 1;

  if (vid == RAJA_Seq) {

    if (tune_idx == t) {

      runSeqVariantFuncPtr(vid);

    }

    t += 1;

    if (tune_idx == t) {

      runSeqVariantVirtFunc(vid);

    }

    t += 1;

  }
}

void HALOEXCHANGE_FUSED::setSeqTuningDefinitions(VariantID vid)
{
  addVariantTuningName(vid, "default");

  if (vid == RAJA_Seq) {
    addVariantTuningName(vid, "funcptr");
    addVariantTuningName(vid, "virtfunc");
  }
}

} // end namespace apps
} // end namespace rajaperf
