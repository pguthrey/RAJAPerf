//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "CONVECTION3DPA.hpp"

#include "RAJA/RAJA.hpp"

#include <iostream>

namespace rajaperf {
namespace apps {

void CONVECTION3DPA::runOpenMPVariant(VariantID vid, size_t RAJAPERF_UNUSED_ARG(tune_idx)) {

#if defined(RAJA_ENABLE_OPENMP) && defined(RUN_OPENMP)

  const Index_type run_reps = getRunReps();
  CONVECTION3DPA_DATA_SETUP;

  switch (vid) {

  case Base_OpenMP: {

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

#pragma omp parallel for
      for (int e = 0; e < NE; ++e) {

        CONVECTION3DPA_0_CPU;

        CPU_FOREACH(dz,z,CPA_D1D)
        {
          CPU_FOREACH(dy,y,CPA_D1D)
          {
            CPU_FOREACH(dx,x,CPA_D1D)
            {
              CONVECTION3DPA_1;
            }
          }
        }

        CPU_FOREACH(dz,z,CPA_D1D)
        {
          CPU_FOREACH(dy,y,CPA_D1D)
          {
            CPU_FOREACH(qx,x,CPA_Q1D)
            {
              CONVECTION3DPA_2;
            }
          }
        }

        CPU_FOREACH(dz,z,CPA_D1D)
        {
          CPU_FOREACH(qx,x,CPA_Q1D)
          {
            CPU_FOREACH(qy,y,CPA_Q1D)
            {
              CONVECTION3DPA_3;
            }
          }
        }

        CPU_FOREACH(qx,x,CPA_Q1D)
        {
          CPU_FOREACH(qy,y,CPA_Q1D)
          {
            CPU_FOREACH(qz,z,CPA_Q1D)
            {
              CONVECTION3DPA_4;
            }
          }
        }

        CPU_FOREACH(qz,z,CPA_Q1D)
        {
          CPU_FOREACH(qy,y,CPA_Q1D)
          {
            CPU_FOREACH(qx,x,CPA_Q1D)
            {
              CONVECTION3DPA_5;
            }
          }
        }

        CPU_FOREACH(qx,x,CPA_Q1D)
        {
          CPU_FOREACH(qy,y,CPA_Q1D)
          {
            CPU_FOREACH(dz,z,CPA_D1D)
            {
              CONVECTION3DPA_6;
            }
          }
        }

        CPU_FOREACH(dz,z,CPA_D1D)
        {
           CPU_FOREACH(qx,x,CPA_Q1D)
           {
              CPU_FOREACH(dy,y,CPA_D1D)
              {
                CONVECTION3DPA_7;
             }
          }
        }

        CPU_FOREACH(dz,z,CPA_D1D)
        {
          CPU_FOREACH(dy,y,CPA_D1D)
          {
            CPU_FOREACH(dx,x,CPA_D1D)
            {
              CONVECTION3DPA_8;
            }
          }
        }

      } // element loop
    }
    stopTimer();

    break;
  }

  case RAJA_OpenMP: {

    using launch_policy = RAJA::expt::LaunchPolicy<RAJA::expt::omp_launch_t>;

    using outer_x = RAJA::expt::LoopPolicy<RAJA::omp_for_exec>;

    using inner_x = RAJA::expt::LoopPolicy<RAJA::loop_exec>;

    using inner_y = RAJA::expt::LoopPolicy<RAJA::loop_exec>;

    using inner_z = RAJA::expt::LoopPolicy<RAJA::loop_exec>;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      // Grid is empty as the host does not need a compute grid to be specified
      RAJA::expt::launch<launch_policy>(
          RAJA::expt::Grid(),
          [=] RAJA_HOST_DEVICE(RAJA::expt::LaunchContext ctx) {

          RAJA::expt::loop<outer_x>(ctx, RAJA::RangeSegment(0, NE),
            [&](int e) {

             CONVECTION3DPA_0_CPU;

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                    [&](int dy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                        [&](int dx) {

                          CONVECTION3DPA_1;

                        } // lambda (dx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (dy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

              ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                    [&](int dy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                        [&](int qx) {

                          CONVECTION3DPA_2;

                        } // lambda (dx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (dy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

            ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                    [&](int qx) {
                      RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                        [&](int qy) {

                          CONVECTION3DPA_3;

                        } // lambda (dy)
                      ); // RAJA::expt::loop<inner_y>
                    } // lambda (dx)
                  );  //RAJA::expt::loop<inner_x>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

            ctx.teamSync();

              RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                [&](int qx) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                    [&](int qy) {
                      RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                        [&](int qz) {

                          CONVECTION3DPA_4;

                        } // lambda (qz)
                      ); // RAJA::expt::loop<inner_z>
                    } // lambda (qy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (qx)
              );  //RAJA::expt::loop<inner_x>

            ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                [&](int qz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                    [&](int qy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                        [&](int qx) {

                          CONVECTION3DPA_5;

                        } // lambda (qx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (qy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (qz)
              );  //RAJA::expt::loop<inner_z>

            ctx.teamSync();

              RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                [&](int qx) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                    [&](int qy) {
                      RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                        [&](int dz) {

                          CONVECTION3DPA_6;

                        } // lambda (dz)
                      ); // RAJA::expt::loop<inner_z>
                    } // lambda (qy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (qx)
              );  //RAJA::expt::loop<inner_x>

            ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_Q1D),
                    [&](int qx) {
                      RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                        [&](int dy) {

                          CONVECTION3DPA_7;

                        } // lambda (dy)
                      ); // RAJA::expt::loop<inner_y>
                    } // lambda (qx)
                  );  //RAJA::expt::loop<inner_x>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

            ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                    [&](int dy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, CPA_D1D),
                        [&](int dx) {

                          CONVECTION3DPA_8;

                        } // lambda (dx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (dy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

            } // lambda (e)
          ); // RAJA::expt::loop<outer_x>

        }  // outer lambda (ctx)
      );  // RAJA::expt::launch
    }  // loop over kernel reps
    stopTimer();

    return;
  }

  default:
    getCout() << "\n CONVECTION3DPA : Unknown OpenMP variant id = " << vid
              << std::endl;
  }

#else
  RAJA_UNUSED_VAR(vid);
#endif
}

} // end namespace apps
} // end namespace rajaperf
