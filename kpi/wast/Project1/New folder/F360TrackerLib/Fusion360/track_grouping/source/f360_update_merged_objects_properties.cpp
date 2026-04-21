/*===================================================================================*\
* FILE: f360_update_merged_objects_properties.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Update_Merged_Objects_Properties() function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/

#include "f360_update_merged_objects_properties.h"
#include "f360_update_object_reference_point.h"
#include "f360_math_func.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_static_env_polys_support_functions.h"
#include "f360_norm_heading_angle.h"
#include "f360_get_reference_point_para_side.h"

namespace f360_variant_A
{   
  static void Inherit_Flags_After_Merge(
      F360_Object_Track_T & object_track_to_keep,
      const F360_Object_Track_T & object_track_to_kill);
   
  static void Determine_Maximum_Width_And_Adapt(
      const F360_Calibrations_T & calib,
      float32_t & width,
      float32_t & wid1,
      float32_t & wid2);

   /*===========================================================================*\
   * FUNCTION: Update_Merged_Objects_Properties()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object_track_to_kill
   * const F360_Calibrations_T & calib
   * const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const F360_Host_T & host
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Globals_T& globals
   * F360_Dimensions_T & dimensions
   * F360_Object_Track_T & object_track_to_keep
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
   * Function updates objects properties after merge.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Update_Merged_Objects_Properties(
      const F360_Object_Track_T & object_track_to_kill,
      const F360_Calibrations_T & calibrations,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const F360_Host_T & host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Globals_T& globals,
      F360_Dimensions_T& dimensions,
      F360_Object_Track_T& object_track_to_keep)
   {
      Inherit_Flags_After_Merge(object_track_to_keep, object_track_to_kill);

      Determine_Maximum_Width_And_Adapt(calibrations, dimensions.width, dimensions.wid1, dimensions.wid2);

      Adjust_Obj_States_After_Merge(dimensions, calibrations, host, sensors, globals, object_track_to_keep);

      Flag_Single_Object_On_And_Behind_SEP(sep, calibrations, object_track_to_keep);
   }

   /*===========================================================================*\
   * FUNCTION: Adjust_Obj_States_After_Merge()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Dimensions_T & merged_object_initial_size
   * const F360_Calibrations_T& calibrations
   * const F360_Host_T & host
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Globals_T& globals
   * F360_Object_Track_T & object_track_to_keep
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
   * Function update objects state after merge
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   void Adjust_Obj_States_After_Merge(
      const F360_Dimensions_T & merged_object_initial_size,
      const F360_Calibrations_T& calibrations,
      const F360_Host_T & host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Globals_T& globals,
      F360_Object_Track_T& object_track_to_keep)
   {
      // Compute center of new merged object bounding box
      float32_t new_center_vcs_x;
      float32_t new_center_vcs_y;

      const float32_t new_center_para = 0.5F * (merged_object_initial_size.len2 - merged_object_initial_size.len1);
      
      float32_t new_center_orth = 0.5F * (merged_object_initial_size.wid2 - merged_object_initial_size.wid1);
      // If the rear or front of the keep object is visible, use keep object's width (i.e. orth center in keep object's TCS = 0)
      float32_t new_width = merged_object_initial_size.width;
      const F360_Object_Sides_T rear_or_front_side = Get_Reference_Point_Para_Side(object_track_to_keep.reference_point);
      const bool f_rear_or_front_visible = (F360_OBJECT_SIDES_INVALID != rear_or_front_side);
      if (f_rear_or_front_visible)
      {
         new_center_orth = 0.0F;
         new_width = object_track_to_keep.bbox.Get_Width();
      }

      Convert_TCS_Posn_To_VCS_Posn(
         new_center_para,
         new_center_orth,
         object_track_to_keep.bbox.Get_Center().x,
         object_track_to_keep.bbox.Get_Center().y,
         object_track_to_keep.bbox.Get_Orientation(),
         new_center_vcs_x,
         new_center_vcs_y);

      // Change position and reference point of object to correspond to the new center
      object_track_to_keep.reference_point = F360_REFERENCE_POINT_CENTER;
      object_track_to_keep.min_projection_reference_point = F360_REFERENCE_POINT_CENTER;
      object_track_to_keep.vcs_position.x = new_center_vcs_x;
      object_track_to_keep.vcs_position.y = new_center_vcs_y;

      // Update predicted position as well to correspond to the new center
      object_track_to_keep.predicted_vcs_position.x = object_track_to_keep.vcs_position.x;
      object_track_to_keep.predicted_vcs_position.y = object_track_to_keep.vcs_position.y;

      // Set new bbox dimensions
      object_track_to_keep.Update_Bbox_Size(merged_object_initial_size.length, new_width);

      // Update reference point to choose one which is better than CENTER
      Update_Object_Reference_Point(host.dist_rear_axle_to_vcs_m, true, true, calibrations, sensors, globals, object_track_to_keep);
   }

   /*===========================================================================*\
   * FUNCTION: Inherit_Flags_After_Merge()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Object_Track_T & object_track_to_keep,
   * const F360_Object_Track_T & object_track_to_kill
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
   * Inherit flags based on a combination of properties for the track to be kept
   * and the track to be killed.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static void Inherit_Flags_After_Merge(
      F360_Object_Track_T & object_track_to_keep,
      const F360_Object_Track_T & object_track_to_kill)
   {
      // Inherit flags correctly
      object_track_to_keep.f_vehicular_trk = (object_track_to_keep.f_vehicular_trk || object_track_to_kill.f_vehicular_trk);
      object_track_to_keep.f_moveable = (object_track_to_keep.f_moveable || object_track_to_kill.f_moveable);
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Maximum_Width_And_Adapt()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib
   * float32_t & width
   * float32_t & wid1
   * float32_t & wid2
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
   * Determine the maximum allowed width for the track based on its motion
   * properties (f_vehicular_trk and f_moveable). Depending on the result,
   * adapt wid1 and wid2 correspondingly.
   *
   * PRECONDITIONS:
   * All the Pointers should Point to valid structures.
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static void Determine_Maximum_Width_And_Adapt(
      const F360_Calibrations_T & calib,
      float32_t & width,
      float32_t & wid1,
      float32_t & wid2)
   {
      // Determine max width of merged object based on motion flags 
      const float32_t max_width = calib.k_movable_max_target_width;

      // Update width/wid1/wid2 if previous width is larger than maximum allowed width
      if (width > max_width)
      {
         const float32_t factor = 1.0F/width;
         wid1 = wid1 * factor * max_width;
         wid2 = wid2 * factor * max_width;
         width = max_width;
      }
   }

}
