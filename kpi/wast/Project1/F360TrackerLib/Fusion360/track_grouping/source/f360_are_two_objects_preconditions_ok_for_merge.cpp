/*===================================================================================*\
* FILE: f360_are_two_objects_preconditions_ok_for_merge.cpp
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of Are_Two_Objects_Preconditions_OK_For_Merge function.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/


#include "f360_are_two_objects_preconditions_ok_for_merge.h"

#include "f360_math_func.h"
#include "f360_norm_heading_angle.h"
#include "f360_merge_bbox_overlap_test.h"
#include "f360_point.h"
#include "f360_bounding_box.h"
#include <algorithm>

namespace f360_variant_A
{
   static bool Do_Objects_Meet_Distance_Condition(
      const F360_Object_Track_T & first_object,
      const F360_Object_Track_T & second_object);

   static bool Do_Objects_Meet_Hdg_Condition(
      const float32_t first_object_vcs_heading,
      const float32_t second_object_vcs_heading,
      const float32_t heading_threshold);

   static bool Do_Objects_Meet_Speed_Condition(
      const float32_t first_object_speed,
      const float32_t second_object_speed,
      const float32_t speed_threshold);

   static bool Do_Objects_Meet_Curvature_Condition(
       const float32_t first_object_curvature,
       const float32_t second_object_curvature,
       const float32_t curvature_threshold);

   static BoundingBox Create_Bounding_Box_On_Object_Detections(const F360_Object_Track_T & object, const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]);

   static void Create_Points_On_Obj_Detections(Point(&det_points)[MAX_DETS_IN_OBJ_TRK], const F360_Object_Track_T & object, const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]);

   /*===========================================================================*\
   * FUNCTION: Are_Two_Objects_Preconditions_OK_For_Merge()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calib,
   * const F360_Object_Track_T & first_object,
   * const F360_Object_Track_T & second_object,
   * const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
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
   * Functions checks whether two objects meet preliminary conditions for merge.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Are_Two_Objects_Preconditions_OK_For_Merge(
      const F360_Calibrations_T & calib,
      const F360_Object_Track_T & first_object,
      const F360_Object_Track_T & second_object,
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      const bool f_preconditions_meet = 
         ((Are_Merging_Coarse_Gate_Conditions_Met(first_object, second_object, calib.k_track_grouping_hdg_gate, calib.k_track_grouping_speed_gate, calib.k_track_grouping_curvature_gate))
         && (Merge_Bbox_Overlap_Test(first_object, second_object, calib.k_track_grouping_half_long_extension, calib.k_track_grouping_half_lat_extension))
         && (Merge_Metal_To_Metal_Test(first_object, second_object, detection_props, calib)));
      return f_preconditions_meet;
   }

   /*===========================================================================*\
   * FUNCTION: Merge_Metal_To_Metal_Test()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const F360_Object_Track_T & first_object,
   * const F360_Object_Track_T & second_object,
   * const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
   * const F360_Calibrations_T & calib
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
   * Test performed only for slow moving objects. For objects above threshold function returns true.
   * Function creates bounding box spreaded over both objects' detections and calculate
   * two metrics of distance between bboxes:
   * - closest possible - distance AKA metal to metal distance
   * - lateral spread   - largest possible lateral distance between bboxes corners - this is only
   *                      calculated from first_object perspective.
   * If these values are below calibration values, then function returns true.
   * If object does not have any assigned detection, then created bbox is infinitly small and 
   * is located on object's center. 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Merge_Metal_To_Metal_Test(
      const F360_Object_Track_T & first_object,
      const F360_Object_Track_T & second_object,
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Calibrations_T & calib)
   {
      bool conditions_valid;
      if ((first_object.speed < calib.merging_m2m_max_obj_speed) || (second_object.speed < calib.merging_m2m_max_obj_speed))
      {
         const BoundingBox bbox_A = Create_Bounding_Box_On_Object_Detections(first_object, detection_props);
         const BoundingBox bbox_B = Create_Bounding_Box_On_Object_Detections(second_object, detection_props);
         const Distance_Between_Bboxes dist_between_bboxes = bbox_A.Combined_Distance_To(bbox_B);

         const bool metal_to_metal_condition_valid = (dist_between_bboxes.closest_distance < calib.merging_m2m_distance_threshold);
         const bool lateral_spread_condtition_valid = (dist_between_bboxes.lateral_spread < calib.merging_lateral_det_spread_threshold);
         conditions_valid = (metal_to_metal_condition_valid && lateral_spread_condtition_valid);
      }
      else
      {
         conditions_valid = true;
      }
      return conditions_valid;
   }

   /*===========================================================================*\
   * FUNCTION: Create_Bounding_Box_On_Object_Detections()
   *===========================================================================
   * RETURN VALUE:
   * BoundingBox
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object,
   * const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
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
   * Function creates bounding box spreaded over object detections. If there is none
   * detection assigned then created bbox is infinitly small and 
   * is located on object's center. 
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static BoundingBox Create_Bounding_Box_On_Object_Detections(
      const F360_Object_Track_T & object, 
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      BoundingBox bbox;
      if (object.ndets > 0U)
      {
         Point det_points_A[MAX_DETS_IN_OBJ_TRK] = {};
         Create_Points_On_Obj_Detections(det_points_A, object, detection_props);
         bbox = BoundingBox(det_points_A, object.ndets, object.vcs_heading);
      }
      else // If there is no any detections associated, create infinite small bbox around object center
      {
         bbox = BoundingBox(object.bbox.Get_Center(), 0.0F, 0.0F, object.bbox.Get_Orientation());
      }
      return bbox;
   }

   /*===========================================================================*\
   * FUNCTION: Create_Points_On_Obj_Detections()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * Point(&det_points)[MAX_DETS_IN_OBJ_TRK],
   * const F360_Object_Track_T & object,
   * const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS]
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
   * Edit N Points which are stored in det_points array to be located in same place
   * as N detections whicha are associated to object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static void Create_Points_On_Obj_Detections(
      Point(&det_points)[MAX_DETS_IN_OBJ_TRK], 
      const F360_Object_Track_T & object, 
      const F360_Detection_Props_T(&detection_props)[MAX_NUMBER_OF_DETECTIONS])
   {
      for (uint32_t det_index = 0U; det_index < object.ndets; det_index++)
      {
         const F360_Detection_Props_T& detection = detection_props[object.detids[det_index] - 1U];
         det_points[det_index].Set_Position(detection.vcs_position.x, detection.vcs_position.y);
      }
   }
   /*===========================================================================*\
   * FUNCTION: Are_Merging_Coarse_Gate_Conditions_Met()
   *===========================================================================
   * RETURN VALUE:
   * bool gating_conditions_met
   *
   * PARAMETERS:
   * const F360_Object_Track_T & object,
   * const F360_Object_Track_T & second_object,
   * const float32_t heading_gate,
   * const float32_t speed_gate
   * const float32_t curvature_gate
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
   * Checks whether coarse conditions for merging two objects are met:
   * - both objects are moving or both objects are not moveable
   * - distance between objects is low enough
   * - difference between objects' heading is below heading_gate value
   * - difference between objects' speed is below speed_gate value
   * - difference between objects' curvature is below curvature gate value
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   bool Are_Merging_Coarse_Gate_Conditions_Met(
      const F360_Object_Track_T & first_object,
      const F360_Object_Track_T & second_object,
      const float32_t heading_gate,
      const float32_t speed_gate,
      const float32_t curvature_gate
   )
   {
      const bool both_objects_moving = (first_object.f_moving) && (second_object.f_moving);
      const bool both_objects_not_moveable = (!first_object.f_moveable) && (!second_object.f_moveable);
      const bool both_moving_or_both_not_moveable = both_objects_moving || both_objects_not_moveable;

      return ((both_moving_or_both_not_moveable)
         && (Do_Objects_Meet_Distance_Condition(first_object, second_object))
         && (Do_Objects_Meet_Hdg_Condition(first_object.vcs_heading.Value(), second_object.vcs_heading.Value(), heading_gate))
         && (Do_Objects_Meet_Speed_Condition(first_object.speed, second_object.speed, speed_gate))
         && (Do_Objects_Meet_Curvature_Condition(first_object.curvature, second_object.curvature, curvature_gate)));
   }

   /*===========================================================================*\
   * FUNCTION: Do_Objects_Meet_Distance_Condition()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const F360_Object_Track_T object,
   * const F360_Object_Track_T second_object
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
   * Functions checks whether objects meet distance conditions in respect to its 
   * length values.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static bool Do_Objects_Meet_Distance_Condition(
      const F360_Object_Track_T & first_object,
      const F360_Object_Track_T & second_object)
   {
      const float32_t max_dim = first_object.bbox.Get_Length() + second_object.bbox.Get_Length();
      const bool f_distance_ok = (std::abs(first_object.bbox.Get_Center().x - second_object.bbox.Get_Center().x) < max_dim) &&
         (std::abs(first_object.bbox.Get_Center().y - second_object.bbox.Get_Center().y) < max_dim);
      return f_distance_ok;
   }
   
   /*===========================================================================*\
   * FUNCTION: Do_Objects_Meet_Hdg_Condition()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const float32_t first_object_vcs_heading
   * const float32_t second_object_vcs_heading
   * const float32_t heading_threshold
   *
   * EXTERNAL REFERENCES:
   * bool
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * See if two objects heading difference is close enough to allow merge.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static bool Do_Objects_Meet_Hdg_Condition(
      const float32_t first_object_vcs_heading,
      const float32_t second_object_vcs_heading,
      const float32_t heading_threshold)
   {
      const float32_t normalized_hdg = Normalize_Heading_Angle(second_object_vcs_heading, first_object_vcs_heading); // Makes second object vcs heading "near" first object heading. 
      const bool f_hdg_ok = std::abs(first_object_vcs_heading - normalized_hdg) <= heading_threshold;
      return f_hdg_ok;
   }

   /*===========================================================================*\
   * FUNCTION: Do_Objects_Meet_Speed_Condition()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const float32_t first_object_speed
   * const float32_t second_object_speed
   * const float32_t speed_threshold
   *
   * EXTERNAL REFERENCES:
   * bool
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * See if two objects speed difference is close enough to allow merge.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static bool Do_Objects_Meet_Speed_Condition(
      const float32_t first_object_speed,
      const float32_t second_object_speed,
      const float32_t speed_threshold)
   {
      const bool f_speed_ok = std::abs(first_object_speed - second_object_speed) < speed_threshold;
      return f_speed_ok;
   }

   /*===========================================================================*\
   * FUNCTION: Do_Objects_Meet_Curvature_Condition()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const float32_t first_object_curvature
   * const float32_t second_object_curvature
   * const float32_t curvature_threshold
   *
   * EXTERNAL REFERENCES:
   * bool
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * See if two objects' curvature difference is small enough to allow merge.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   \*===========================================================================*/
   static bool Do_Objects_Meet_Curvature_Condition(
       const float32_t first_object_curvature,
       const float32_t second_object_curvature,
       const float32_t curvature_threshold)
   {
       const bool f_curvature_ok = (std::abs(first_object_curvature - second_object_curvature) < curvature_threshold);
       return f_curvature_ok;
   }
}
