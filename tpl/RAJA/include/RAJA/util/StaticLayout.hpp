/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA header file defining Layout, a N-dimensional index calculator
 *          with compile-time defined sizes and permutation
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_util_static_layout_HPP
#define RAJA_util_static_layout_HPP

#include "RAJA/config.hpp"

#include <iostream>
#include <limits>

#include "RAJA/index/IndexValue.hpp"

#include "RAJA/internal/foldl.hpp"

#include "RAJA/util/Operators.hpp"
#include "RAJA/util/Permutations.hpp"


// Note: this can be removed once https://github.com/LLNL/camp/pull/24 is
// incorporated into camp
namespace camp
{
template <idx_t N, typename T, T Idx0, T... IdxRest>
struct seq_at<N, camp::int_seq<T, Idx0, IdxRest...>> {
  static constexpr T value =
      seq_at<N - 1, camp::int_seq<T, IdxRest...>>::value;
};

template <typename T, T Idx0, T... IdxRest>
struct seq_at<0, camp::int_seq<T, Idx0, IdxRest...>> {
  static constexpr T value = Idx0;
};
}

namespace RAJA
{

namespace detail
{


template <typename IdxLin, typename Range, typename Sizes, typename Strides>
struct StaticLayoutBase_impl;


template <typename IdxLin,
          IdxLin... RangeInts,
          IdxLin... Sizes,
          IdxLin... Strides>
struct StaticLayoutBase_impl<IdxLin,
                             camp::int_seq<IdxLin, RangeInts...>,
                             camp::int_seq<IdxLin, Sizes...>,
                             camp::int_seq<IdxLin, Strides...>> {

  using IndexLinear = IdxLin;
  using sizes = camp::int_seq<IdxLin, Sizes...>;
  using strides = camp::int_seq<IdxLin, Strides...>;

  static constexpr camp::idx_t stride_one_dim =
      foldl_max<camp::idx_t>(
          (camp::seq_at<RangeInts, strides>::value == 1 ? RangeInts : -1)...);

  static constexpr size_t n_dims = sizeof...(Sizes);

  /*!
   * Default constructor.
   */
  RAJA_INLINE RAJA_HOST_DEVICE constexpr StaticLayoutBase_impl() {}

  RAJA_INLINE static void print()
  {
    camp::sink(printf("StaticLayout: arg%d: size=%d, stride=%d\n",
                               (int)RangeInts,
                               (int)Sizes,
                               (int)Strides)...);
  }


  /*!
   * Computes a linear space index from specified indices.
   * This is formed by the dot product of the indices and the layout strides.
   *
   * @param indices  Indices in the n-dimensional space of this layout
   * @return Linear space index.
   */
  template <typename... Indices>
  RAJA_INLINE RAJA_HOST_DEVICE constexpr IdxLin operator()(
      Indices... indices) const
  {
    // dot product of strides and indices
    return foldl_sum<IdxLin>((IdxLin(indices * Strides))...);
  }


  template <typename... Indices>
  static RAJA_INLINE RAJA_HOST_DEVICE constexpr IdxLin s_oper(Indices... indices)
  {
    // dot product of strides and indices
    return foldl_sum<IdxLin>((IdxLin(indices * Strides))...);
  }


  // Multiply together all of the sizes,
  // replacing 1 for any zero-sized dimensions
  static constexpr IdxLin s_size =
      RAJA::foldl_product<IdxLin>((Sizes == IdxLin(0) ? IdxLin(1) : Sizes)...);

  /*!
   * Computes a total size of the layout's space.
   * This is the produce of each dimensions size.
   *
   * @return Total size spanned by indices
   */
  RAJA_INLINE RAJA_HOST_DEVICE constexpr static IdxLin size()
  {
    return s_size;
  }


  template<camp::idx_t DIM>
  RAJA_INLINE
  RAJA_HOST_DEVICE
  constexpr
  IndexLinear get_dim_stride() const {
    return camp::seq_at<DIM, strides>::value;
  }


};

template <typename IdxLin, IdxLin N, IdxLin Idx, IdxLin... Sizes>
struct StrideCalculatorIdx {
  static_assert(N == sizeof...(Sizes), "");

  using sizes_seq = camp::int_seq<IdxLin, Sizes...>;
  static constexpr IdxLin size = camp::seq_at<Idx, sizes_seq>::value;
  static constexpr IdxLin size_last =
      StrideCalculatorIdx<IdxLin, N, Idx + 1, Sizes...>::size;
  static constexpr IdxLin value =
      (size_last > 0 ? size_last : 1) *
      StrideCalculatorIdx<IdxLin, N, Idx + 1, Sizes...>::value;
  static constexpr IdxLin stride = size > 0 ? value : 0;
};

template <typename IdxLin, IdxLin N, IdxLin... Sizes>
struct StrideCalculatorIdx<IdxLin, N, N, Sizes...> {
  static_assert(N == sizeof...(Sizes), "");

  static constexpr IdxLin size = 1;
  static constexpr IdxLin value = 1;
  static constexpr IdxLin stride = size > 0 ? value : 0;
};

template <typename IdxLin, typename Range, typename Perm, typename Sizes>
struct StrideCalculator;

template <typename IdxLin, IdxLin ... Range, camp::idx_t... Perm, IdxLin... Sizes>
struct StrideCalculator<IdxLin,
                        camp::int_seq<IdxLin, Range...>,
                        camp::idx_seq<Perm...>,
                        camp::int_seq<IdxLin, Sizes...>> {
  static_assert(sizeof...(Sizes) == sizeof...(Perm), "");

  using sizes = camp::int_seq<IdxLin, Sizes...>;
  static constexpr IdxLin N = sizeof...(Sizes);
  using range = camp::int_seq<IdxLin, Range...>;
  using perm = camp::idx_seq<Perm...>;
  using inv_perm = invert_permutation<perm>;
  using strides_unperm =
      camp::int_seq<IdxLin, StrideCalculatorIdx<IdxLin, N, Range, camp::seq_at<Perm, sizes>::value...>::stride...>;
  
  using strides = camp::int_seq<IdxLin, camp::seq_at<camp::seq_at<Range, inv_perm>::value, strides_unperm>::value...>;
};


template <typename Layout, typename DimTypeList>
struct TypedStaticLayoutImpl;

template <typename Layout, typename... DimTypes>
struct TypedStaticLayoutImpl<Layout, camp::list<DimTypes...>> {

  using IndexLinear = typename Layout::IndexLinear;

  static
  constexpr
  camp::idx_t stride_one_dim = Layout::stride_one_dim;

  static constexpr IndexLinear n_dims = sizeof...(DimTypes);
  /*!
   * Computes a linear space index from specified indices.
   * This is formed by the dot product of the indices and the layout strides.
   *
   * @param indices  Indices in the n-dimensional space of this layout
   * @return Linear space index.
   */
  static RAJA_INLINE RAJA_HOST_DEVICE constexpr IndexLinear s_oper(
      DimTypes... indices)
  {
    return Layout::s_oper(stripIndexType(indices)...);
  }


  static constexpr IndexLinear s_size = Layout::s_size;

  RAJA_INLINE RAJA_HOST_DEVICE constexpr static IndexLinear size()
  {
    return s_size;
  }

  template<camp::idx_t DIM>
  RAJA_INLINE
  RAJA_HOST_DEVICE
  constexpr
  IndexLinear get_dim_stride() const {
    return Layout{}.get_dim_stride();
  }

  RAJA_INLINE
  static void print() { Layout::print(); }
};




}  // namespace detail


template <typename Perm, typename IdxLin, camp::idx_t... Sizes>
using StaticLayoutT = detail::StaticLayoutBase_impl<
    IdxLin,
    camp::make_int_seq_t<IdxLin, sizeof...(Sizes)>,
    camp::int_seq<IdxLin, Sizes...>,
    typename detail::StrideCalculator<IdxLin,
                                      camp::make_int_seq_t<IdxLin, sizeof...(Sizes)>,
                                      Perm,
                                      camp::int_seq<IdxLin, Sizes...>>::strides>;


template <typename Perm, camp::idx_t... Sizes>
using StaticLayout = StaticLayoutT<Perm, camp::idx_t, Sizes...>;



template <typename Perm, typename IdxLin, typename TypeList, camp::idx_t... Sizes>
using TypedStaticLayout =
    detail::TypedStaticLayoutImpl<StaticLayoutT<Perm, IdxLin, Sizes...>, TypeList>;


}  // namespace RAJA

#endif