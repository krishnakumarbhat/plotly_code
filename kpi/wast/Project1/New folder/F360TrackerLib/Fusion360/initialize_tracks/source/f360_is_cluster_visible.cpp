/*===================================================================================*\
* FILE: f360_is_cluster_visible.cpp
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Is_Cluster_Visible function
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/

#include "f360_nees_cfmi_preconditions.h"
#include "f360_is_cluster_visible.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Is_Cluster_Visible()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const Occlusion_T& occlusion,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detections,
   * const F360_Cluster_T& current_cluster,
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT: 
   * --------------------------------------------------------------------------
   * Determine if cluster is visible based on sensor perspetive from which
   * detections are coming.
   *
   * Check from which sensor detections in a cluster are coming. Only current
   * detections are taken into account, historical are ignored. Check occlusion
   * status of cluster position based on view from this sensor. If cluster contains
   * detections from multiple sensors, occlusion will be checked for all of them.
   * Cluster is considered visible if for at least one sensor occlusion 
   * status is visible or undefined.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Cluster_Visible(
      const Occlusion_T& occlusion,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detections,
      const F360_Cluster_T& current_cluster)
   {
      bool f_visible = false;

      bool f_occlusion_from_sensor_checked[MAX_NUMBER_OF_SENSORS] = {};

      for (int32_t i = 0; i < current_cluster.ndets; i++)
      {
         const int16_t det_idx = current_cluster.detids[i] - 1;
         const int32_t sensor_idx = raw_detections.detections[det_idx].raw.sensor_id - 1;
         if (!f_occlusion_from_sensor_checked[sensor_idx])
         {
            const Occlusion_Status_T occlusion_status = occlusion.Determine_Occlusion_Status_Single_Sensor(current_cluster.vcs_position.x, current_cluster.vcs_position.y, sensor_idx);
            f_visible = (OCCLUSION_STATUS_OCCLUDED != occlusion_status) && (OCCLUSION_STATUS_ON_EDGE != occlusion_status);
            f_occlusion_from_sensor_checked[sensor_idx] = true;
            if (f_visible)
            {
               break;
            }
         }
      }

      return f_visible;
   }
}
