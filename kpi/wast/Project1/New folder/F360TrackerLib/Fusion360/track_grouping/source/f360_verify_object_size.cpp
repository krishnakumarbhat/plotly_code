/*===================================================================================*\
* FILE: f360_verify_object_size.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Verify_Object_Size function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
* None.
*
\*===================================================================================*/

#include "f360_verify_object_size.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Verify_Object_Size()
   *===========================================================================
   * RETURN VALUE:
   * bool f_merged_obj_size_ok
   *
   * PARAMETERS:
   * const F360_Object_Track_T & obj_to_keep
   * const F360_Object_Track_T & obj_to_kill
   * const F360_Calibrations_T & calibs
   * F360_Dimensions_T & initial_merged_obj_dimensions
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
   * Function verifies merged object dimensions and verifies whether object 
   * dimensions after merge are smaller than maximum allowed values.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Verify_Object_Size(
      const F360_Object_Track_T & obj_to_keep,
      const F360_Object_Track_T & obj_to_kill,
      const F360_Calibrations_T & calibs,
      const F360_Dimensions_T & initial_merged_obj_dimensions)
   {
       bool f_merged_obj_size_ok = true;

       // For moveable objects that has never been seen with a high speed, we limit their length and width.
       // If the merged moveable object has big length or width, the merge will be canceled.
       if ((!obj_to_kill.f_vehicular_trk) &&
          (!obj_to_keep.f_vehicular_trk))
       {
          f_merged_obj_size_ok = (initial_merged_obj_dimensions.length <= calibs.k_slow_movable_max_target_length) &&
                          (initial_merged_obj_dimensions.width <= calibs.k_movable_max_target_width);
       }
       else 
       {
          constexpr float32_t max_allowed_merged_object_width = 3.9F;
          constexpr float32_t max_allowed_merged_object_length_fast_moving_buffer = 3.0F;
          constexpr float32_t max_allowed_merged_object_length_slow_moving = 10.0F;
          constexpr float32_t max_speed_for_slow_moving_merge = 3.0F;

          // Set maximum allowed merge object length depending on if both objects are slow moving or not
          float32_t merged_obj_max_length;
          const bool f_both_objects_slow_moving = ((obj_to_keep.speed < max_speed_for_slow_moving_merge) && (obj_to_kill.speed < max_speed_for_slow_moving_merge));
          if (f_both_objects_slow_moving)
          {
             merged_obj_max_length = max_allowed_merged_object_length_slow_moving;
          }
          else
          {
             merged_obj_max_length = calibs.k_fast_movable_max_target_length + max_allowed_merged_object_length_fast_moving_buffer;
          }

          const bool f_obj_not_too_long = (initial_merged_obj_dimensions.length <= merged_obj_max_length);
          const bool f_obj_not_too_wide = (initial_merged_obj_dimensions.width <= max_allowed_merged_object_width);

          // The time_since_split is reset to -1.0 when it's been long enough time since split. So a time above 0 indicates that a split has recently occured.
          const bool f_both_object_have_been_split_recently =  (obj_to_keep.time_since_split >= 0.0F) && (obj_to_kill.time_since_split >= 0.0F);
          if (f_both_object_have_been_split_recently)
          {
             // Only restrict maximum length, i.e. allow merges in TCS y direction if objects have been split recently
             f_merged_obj_size_ok = f_obj_not_too_long;
          }
          else
          {
             f_merged_obj_size_ok = f_obj_not_too_long && f_obj_not_too_wide;
          }
       }
      return f_merged_obj_size_ok;
   }
}
