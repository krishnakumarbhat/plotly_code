/*===================================================================================\
 * FILE: f360_nees_cfmi_post_processing.cpp
 *====================================================================================
 * Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   Post-processing function for nees cost function initialization
 *
 * Applicable Standards (in order of precedence: highest first):
 *   ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *   ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_nees_cfmi_post_processing.h"
#include "f360_nees_cfmi_post_processing_internals.h"

#include "f360_math.h"
#include "f360_math_func.h"
#include "f360_nees_cfmi_helpers.h"
#include "f360_nees_cfmi_minimization.h"
#include "f360_nees_cfmi_stationary_checks.h"
#include "f360_uncertainty_propagation.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: NEES_CFMI_Init_Info_Post_Processing
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibrations
   * const F360_NEES_CFMI_Information_T& nees_cfmi_information
   * const F360_Host_Props_T& host_props
   * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
   * const bool valid_for_liberal_tracking
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Post-processing of velocity information
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void NEES_CFMI_Init_Info_Post_Processing(
      const F360_Calibrations_T& calibrations,
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info,
      const bool valid_for_liberal_tracking)
   {

      //TODO: DEX-3259 liberal to be removed
      // Overwrite velocity estimate with radial velocity in liberal initialization mode
       if ((!nees_cfmi_init_info.f_stationary) && valid_for_liberal_tracking)
       {
           nees_cfmi_init_info.VCS_vel_hyp.f_valid = nees_cfmi_information.radial_velocity.f_valid;
           nees_cfmi_init_info.f_valid = nees_cfmi_information.radial_velocity.f_valid;
           nees_cfmi_init_info.f_plausible = true;

           NEES_CFMI_Estimate_Detection_Bounding_Box(nees_cfmi_information, nees_cfmi_init_info);

           nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_X][F360_2D_IDX_X] =
               nees_cfmi_information.pos_center_cov[F360_2D_IDX_X][F360_2D_IDX_X];
           nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_X][F360_2D_IDX_Y] =
               nees_cfmi_information.pos_center_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
           nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_Y][F360_2D_IDX_X] =
               nees_cfmi_information.pos_center_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
           nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] =
               nees_cfmi_information.pos_center_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];
       }
       else
       {
           if (nees_cfmi_init_info.VCS_vel_hyp.f_valid)
           {
               // Covariance trace + Information
               nees_cfmi_init_info.VCS_vel_hyp.vel_cov_trace = Trace_Of_2d_Covariance(nees_cfmi_init_info.VCS_vel_hyp.vel_cov);

               Velocity_Plausibility_Checks(calibrations, nees_cfmi_init_info.VCS_vel_hyp);
               Initialization_Sanity_Checks(calibrations, nees_cfmi_init_info);


               nees_cfmi_init_info.f_valid = !nees_cfmi_init_info.f_fault_detected;

               //DFD-1878 re-visit to decide removing or not
               nees_cfmi_init_info.VCS_vel_hyp.f_valid = !nees_cfmi_init_info.f_fault_detected;

               if (nees_cfmi_init_info.f_valid)
               {
                   nees_cfmi_init_info.f_plausible = nees_cfmi_init_info.VCS_vel_hyp.f_plausible;

                   Update_NEES_CFMI_Init_Scheme(nees_cfmi_init_info);

                   NEES_CFMI_Estimate_Detection_Bounding_Box(nees_cfmi_information, nees_cfmi_init_info);

                   nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_X][F360_2D_IDX_X] =
                       nees_cfmi_information.pos_center_cov[F360_2D_IDX_X][F360_2D_IDX_X];
                   nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_X][F360_2D_IDX_Y] =
                       nees_cfmi_information.pos_center_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
                   nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_Y][F360_2D_IDX_X] =
                       nees_cfmi_information.pos_center_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
                   nees_cfmi_init_info.VCS_pos_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] =
                       nees_cfmi_information.pos_center_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];


               }

           }
       }

   }

   /*===========================================================================*\
    * FUNCTION: NEES_CFMI_Estimate_Detection_Bounding_Box
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const F360_NEES_CFMI_Information_T& nees_cfmi_information
    * F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info
    *
    * DEVIATIONS FROM STANDARDS:
    * None.
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * Computes a bounding box containing all the detections in nees_cfmi_information
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   void NEES_CFMI_Estimate_Detection_Bounding_Box(
      const F360_NEES_CFMI_Information_T& nees_cfmi_information,
      F360_NEES_CFMI_Init_Info_T& nees_cfmi_init_info)
   {
      // Compute the min and max para and orth position of all detections in TCS coordinate system (NOTE: TCS has center in VCS origin but is rotated according to the heading of the estimated velocity vector)
      const float32_t init_hdg = F360_Atan2f(nees_cfmi_init_info.VCS_vel_hyp.vel.lateral, nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal);
      const float32_t cos_init_hdg = F360_Cosf(init_hdg);
      const float32_t sin_init_hdg = F360_Sinf(init_hdg);
      float32_t max_para = -INFTY;
      float32_t min_para = INFTY;
      float32_t max_orth = -INFTY;
      float32_t min_orth = INFTY;

      for (uint16_t i = 0U; i < nees_cfmi_information.dets_num; i++)
      {
         float32_t para_pos;
         float32_t orth_pos;

         if (nees_cfmi_information.detections[i].idx.f_historical)
         {
            const float32_t long_pos = nees_cfmi_information.detections[i].pos.x + (nees_cfmi_init_info.VCS_vel_hyp.vel.longitudinal * nees_cfmi_information.detections[i].timestamp_diff);
            const float32_t lat_pos = nees_cfmi_information.detections[i].pos.y + (nees_cfmi_init_info.VCS_vel_hyp.vel.lateral * nees_cfmi_information.detections[i].timestamp_diff);
            para_pos = cos_init_hdg * long_pos + sin_init_hdg * lat_pos;
            orth_pos = -sin_init_hdg * long_pos + cos_init_hdg * lat_pos;
         }
         else
         {
            para_pos = cos_init_hdg * nees_cfmi_information.detections[i].pos.x + sin_init_hdg * nees_cfmi_information.detections[i].pos.y;
            orth_pos = -sin_init_hdg * nees_cfmi_information.detections[i].pos.x + cos_init_hdg * nees_cfmi_information.detections[i].pos.y;
         }

         if (para_pos > max_para)
         {
            max_para = para_pos;
         }
         if (para_pos < min_para)
         {
            min_para = para_pos;
         }
         if (orth_pos > max_orth)
         {
            max_orth = orth_pos;
         }
         if (orth_pos < min_orth)
         {
            min_orth = orth_pos;
         }
      }

      // Compute a bounding box containing all the detections in TCS
      const float32_t length = max_para - min_para;
      const float32_t width = max_orth - min_orth;
      const float32_t center_para = 0.5F * (max_para + min_para);
      const float32_t center_orth = 0.5F * (max_orth + min_orth);

      // Transform to VCS
      const float32_t center_x = cos_init_hdg * center_para - sin_init_hdg * center_orth;
      const float32_t center_y = sin_init_hdg * center_para + cos_init_hdg * center_orth;

      // Assign properties
      nees_cfmi_init_info.det_bbox.Set_Orientation(init_hdg);
      nees_cfmi_init_info.det_bbox.Set_Center(center_x, center_y);
      nees_cfmi_init_info.det_bbox.Set_Length(length);
      nees_cfmi_init_info.det_bbox.Set_Width(width);
   }




}

