//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2017-22, Lawrence Livermore National Security, LLC
// and RAJA Performance Suite project contributors.
// See the RAJAPerf/LICENSE file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

// Uncomment to add compiler directives for loop unrolling
//#define USE_RAJAPERF_UNROLL

#include "DIFFUSION3DPA.hpp"

#include "RAJA/RAJA.hpp"

#include <iostream>

namespace rajaperf {
namespace apps {

void DIFFUSION3DPA::runOpenMPVariant(VariantID vid, size_t /*tune_idx*/) {

#if defined(RAJA_ENABLE_OPENMP) && defined(RUN_OPENMP)

  const Index_type run_reps = getRunReps();
  DIFFUSION3DPA_DATA_SETUP;

  switch (vid) {

  case Base_OpenMP: {

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

#pragma omp parallel for
      for (int e = 0; e < NE; ++e) {

        DIFFUSION3DPA_0_CPU;

        CPU_FOREACH(dz, z, DPA_D1D) {
          CPU_FOREACH(dy, y, DPA_D1D) {
            CPU_FOREACH(dx, x, DPA_D1D) {
              DIFFUSION3DPA_1;
            }
          }
        }

        CPU_FOREACH(dy, y, DPA_D1D) {
          CPU_FOREACH(qx, x, DPA_Q1D) {
            DIFFUSION3DPA_2;
          }
        }

        CPU_FOREACH(dz, z, DPA_D1D) {
          CPU_FOREACH(dy, y, DPA_D1D) {
            CPU_FOREACH(qx, x, DPA_Q1D) {
              DIFFUSION3DPA_3;
            }
          }
        }

        CPU_FOREACH(dz, z, DPA_D1D) {
          CPU_FOREACH(qy, y, DPA_Q1D) {
            CPU_FOREACH(qx, x, DPA_Q1D) {
              DIFFUSION3DPA_4;
            }
          }
        }

        CPU_FOREACH(qz, z, DPA_Q1D) {
          CPU_FOREACH(qy, y, DPA_Q1D) {
            CPU_FOREACH(qx, x, DPA_Q1D) {
              DIFFUSION3DPA_5;
            }
          }
        }

        CPU_FOREACH(d, y, DPA_D1D) {
          CPU_FOREACH(q, x, DPA_Q1D) {
            DIFFUSION3DPA_6;
          }
        }

        CPU_FOREACH(qz, z, DPA_Q1D) {
          CPU_FOREACH(qy, y, DPA_Q1D) {
            CPU_FOREACH(dx, x, DPA_D1D) {
              DIFFUSION3DPA_7;
            }
          }
        }

        CPU_FOREACH(qz, z, DPA_Q1D) {
          CPU_FOREACH(dy, y, DPA_D1D) {
            CPU_FOREACH(dx, x, DPA_D1D) {
              DIFFUSION3DPA_8;
            }
          }
        }

        CPU_FOREACH(dz, z, DPA_D1D) {
          CPU_FOREACH(dy, y, DPA_D1D) {
            CPU_FOREACH(dx, x, DPA_D1D) {
              DIFFUSION3DPA_9;
            }
          }
        }

      } // element loop
    }
    stopTimer();

    break;
  }

  case RAJA_OpenMP: {

    // Currently Teams requires two policies if compiled with a device
    using launch_policy = RAJA::expt::LaunchPolicy<RAJA::expt::omp_launch_t
#if defined(RAJA_DEVICE_ACTIVE)
                                                   ,
                                                   d3d_device_launch
#endif
                                                   >;

    using outer_x = RAJA::expt::LoopPolicy<RAJA::omp_for_exec
#if defined(RAJA_DEVICE_ACTIVE)
                                           ,
                                           d3d_gpu_block_x_policy
#endif
                                           >;

    using inner_x = RAJA::expt::LoopPolicy<RAJA::loop_exec
#if defined(RAJA_DEVICE_ACTIVE)
                                           ,
                                           d3d_gpu_thread_x_policy
#endif
                                           >;

    using inner_y = RAJA::expt::LoopPolicy<RAJA::loop_exec
#if defined(RAJA_DEVICE_ACTIVE)
                                           ,
                                           d3d_gpu_thread_y_policy
#endif
                                           >;

    using inner_z = RAJA::expt::LoopPolicy<RAJA::loop_exec
#if defined(RAJA_DEVICE_ACTIVE)
                                           ,
                                           d3d_gpu_thread_z_policy
#endif
                                           >;

    startTimer();
    for (RepIndex_type irep = 0; irep < run_reps; ++irep) {

      // Grid is empty as the host does not need a compute grid to be specified
      RAJA::expt::launch<launch_policy>(
          RAJA::expt::HOST, RAJA::expt::Grid(),
          [=] RAJA_HOST_DEVICE(RAJA::expt::LaunchContext ctx) {

          RAJA::expt::loop<outer_x>(ctx, RAJA::RangeSegment(0, NE),
            [&](int e) {

              DIFFUSION3DPA_0_CPU;

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                    [&](int dy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                        [&](int dx) {

                          DIFFUSION3DPA_1;

                        } // lambda (dx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (dy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

              ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, 1),
                [&](int RAJA_UNUSED_ARG(dz)) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                    [&](int dy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                        [&](int qx) {

                          DIFFUSION3DPA_2;

                        } // lambda (qx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (dy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

              ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                    [&](int dy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                        [&](int qx) {

                          DIFFUSION3DPA_3;

                        } // lambda (qx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (dy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

              ctx.teamSync();

              RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                [&](int dz) {
                  RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                    [&](int qy) {
                      RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                        [&](int qx) {

                          DIFFUSION3DPA_4;

                        } // lambda (qx)
                      ); // RAJA::expt::loop<inner_x>
                    } // lambda (qy)
                  );  //RAJA::expt::loop<inner_y>
                } // lambda (dz)
              );  //RAJA::expt::loop<inner_z>

             ctx.teamSync();

             RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
               [&](int qz) {
                 RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                   [&](int qy) {
                     RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                       [&](int qx) {

                         DIFFUSION3DPA_5;

                       } // lambda (qx)
                     ); // RAJA::expt::loop<inner_x>
                   } // lambda (qy)
                 );  //RAJA::expt::loop<inner_y>
               } // lambda (qz)
             );  //RAJA::expt::loop<inner_z>

             ctx.teamSync();

             RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, 1),
               [&](int RAJA_UNUSED_ARG(dz)) {
                 RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                   [&](int d) {
                     RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                       [&](int q) {

                         DIFFUSION3DPA_6;

                       } // lambda (q)
                     ); // RAJA::expt::loop<inner_x>
                   } // lambda (d)
                 );  //RAJA::expt::loop<inner_y>
               } // lambda (dz)
             );  //RAJA::expt::loop<inner_z>

             ctx.teamSync();

             RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
               [&](int qz) {
                 RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
                   [&](int qy) {
                     RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                       [&](int dx) {

                         DIFFUSION3DPA_7;

                       } // lambda (dx)
                     ); // RAJA::expt::loop<inner_x>
                   } // lambda (qy)
                 );  //RAJA::expt::loop<inner_y>
               } // lambda (qz)
             );  //RAJA::expt::loop<inner_z>

             ctx.teamSync();

             RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_Q1D),
               [&](int qz) {
                 RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                   [&](int dy) {
                     RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                       [&](int dx) {

                         DIFFUSION3DPA_8;

                       } // lambda (dx)
                     ); // RAJA::expt::loop<inner_x>
                   } // lambda (dy)
                 );  //RAJA::expt::loop<inner_y>
               } // lambda (qz)
             );  //RAJA::expt::loop<inner_z>

             ctx.teamSync();

             RAJA::expt::loop<inner_z>(ctx, RAJA::RangeSegment(0, DPA_D1D),
               [&](int dz) {
                 RAJA::expt::loop<inner_y>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                   [&](int dy) {
                     RAJA::expt::loop<inner_x>(ctx, RAJA::RangeSegment(0, DPA_D1D),
                       [&](int dx) {

                         DIFFUSION3DPA_9;

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
    getCout() << "\n DIFFUSION3DPA : Unknown OpenMP variant id = " << vid
              << std::endl;
  }

#else
  RAJA_UNUSED_VAR(vid);
#endif
}

} // end namespace apps
} // end namespace rajaperf
