/*===========================================================================*\
* FILE: f360_split_objects_in_orth_direction.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Split_Objects_In_Orth_Direction()
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_split_objects_in_orth_direction.h"
#include "f360_allocate_id_for_initialized_object.h"
#include "f360_convert_tcs_posn_to_vcs_posn.h"
#include "f360_convert_vcs_posn_to_tcs_posn.h"
#include "f360_sort_priority.h"
#include "f360_sorted_tracks_mgmt.h"
#include "f360_static_env_polys_support_functions.h"
#include "f360_find_detection_inliers.h"
#include <algorithm>
#include "f360_iterator.h"
#include "f360_update_object_reference_point.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Split_Objects_In_Orth_Direction()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Host_T & host
   * const F360_Calibrations_T& calibs
   * const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Globals_T& globals
   * F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS]
   * F360_Tracker_Info_T& tracker_info
   * F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS]
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
   * This function identifies which objects needs to be splitted and performs
   * the actual split
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Split_Objects_In_Orth_Direction(
      const F360_Host_T & host,
      const F360_Calibrations_T& calibs,
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Globals_T& globals,
      F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS])
   {
      // For all objects, reset the timer indicating that an object has recently been split if time since split is large enough
      constexpr float32_t k_time_to_reset_split_timer = 5.0F;
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const uint32_t obj_idx = static_cast<uint32_t>(tracker_info.active_obj_ids[i]) - 1U;
         if (objects[obj_idx].time_since_split > k_time_to_reset_split_timer)
         {
            objects[obj_idx].time_since_split = -1.0F;
         }
      }

      uint32_t nr_objects_to_split;
      uint32_t obj_idx_to_split[NUMBER_OF_OBJECT_TRACKS] = {};
      Find_Objects_To_Split(
         calibs,
         objects,
         tracker_info,
         nr_objects_to_split,
         obj_idx_to_split);

      for (uint32_t i = 0U; i < nr_objects_to_split; i++)
      {
         const uint32_t idx_to_split = obj_idx_to_split[i];

         const int32_t new_obj_id = Split_Single_Object_In_Ortho_Direction(
            host,
            calibs,
            raw_detect_list,
            sensors,
            globals,
            objects[idx_to_split],
            objects,
            tracker_info,
            det_p);

         // Re-evaluate on/behind SEP information
         const int32_t new_obj_idx = new_obj_id - 1;
         Flag_Single_Object_On_And_Behind_SEP(static_env_polys, calibs, objects[idx_to_split]);
         Flag_Single_Object_On_And_Behind_SEP(static_env_polys, calibs, objects[new_obj_idx]);

         // For both objects, start timer after split has occured
         objects[idx_to_split].time_since_split = 0.0F;
         objects[new_obj_idx].time_since_split = 0.0F;
      }

      // Resort in longitudinal direction as we have shifted position of original objects
      if (nr_objects_to_split > 0U)
      {
         Sorted_Tracks_Re_Sort(tracker_info);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Find_Objects_To_Split()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibs,
   * const F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
   * const F360_Tracker_Info_T& tracker_info,
   * uint32_t& nr_objects_to_split,
   * uint32_t(&obj_idx_to_split)[NUMBER_OF_OBJECT_TRACKS]
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
   * This function identifies which objects needs to be splitted and returns
   * the number of objects and an array of object indexes which needs split.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Find_Objects_To_Split(
      const F360_Calibrations_T& calibs,
      const F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Tracker_Info_T& tracker_info,
      uint32_t& nr_objects_to_split,
      uint32_t(&obj_idx_to_split)[NUMBER_OF_OBJECT_TRACKS])
   {
      nr_objects_to_split = 0U;
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const uint32_t obj_idx = static_cast<uint32_t>(tracker_info.active_obj_ids[i]) - 1U;

         if (Is_Object_Valid_For_Split(calibs, objects[obj_idx]))
         {
            obj_idx_to_split[nr_objects_to_split] = obj_idx;
            nr_objects_to_split++;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Is_Object_Valid_For_Split()
   * ===========================================================================
   * RETURN VALUE:
   * bool f_need_split
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibs,
   * const F360_Object_Track_T& object
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
   * This function determines if an object should be splitted
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Is_Object_Valid_For_Split(
      const F360_Calibrations_T& calibs,
      const F360_Object_Track_T& object)
   {
      const bool f_need_split = ((F360_TRACKER_TRKFLTR_CTCA == object.trk_fltr_type) || (F360_TRACKER_TRKFLTR_CCA == object.trk_fltr_type)) &&
         (object.speed > calibs.k_orth_split_min_speed) &&
         (object.orth_gap_filtered > calibs.k_orth_split_min_orth_gap_for_split);

      return f_need_split;
   }

   /*===========================================================================*\
   * FUNCTION: Split_Single_Object_In_Ortho_Direction()
   * ===========================================================================
   * RETURN VALUE:
   * int32_t new_id
   *
   * PARAMETERS:
   * const F360_Host_T & host
   * const F360_Calibrations_T& calibs
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Globals_T& globals
   * F360_Object_Track_T& object_to_split
   * F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS]
   * F360_Tracker_Info_T& tracker_info
   * F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS]
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
   * This function splits a given object in orthogonal direction.
   * The objects are displaced in position by by the objects signal
   * "orth_delta_filtered"/2. The given object will be shifted in position
   * by -"orth_delta_filtered"/2 and the new object is created at position
   * "orth_delta_filtered"/2 in TCS. The new object inherits all other
   * properties of the original object. 
   * The function returns the id of the new created object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * Since we modify the position of the original object that needs split.
   * Resort the longitudinal sorted list by calling function Sorted_Tracks_Re_Sort()
   * after call to this function.
   *
   \*===========================================================================*/
   int32_t Split_Single_Object_In_Ortho_Direction(
      const F360_Host_T & host,
      const F360_Calibrations_T& calibs,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Globals_T& globals,
      F360_Object_Track_T& object_to_split,
      F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
      F360_Tracker_Info_T& tracker_info,
      F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS])
   {
      // Create a new object
      const int32_t new_id = Allocate_Id_For_Initialized_Object(tracker_info, objects, det_p);
      F360_Object_Track_T& new_obj = objects[new_id - 1];

      // Inherit object properties
      Fill_New_Object_Properties(object_to_split, new_id, tracker_info.num_unique_objs, new_obj);

      // Insert created object to the priority sorted list
      Sort_Priority_With_New_Track(tracker_info, &new_obj);

      // Re-associate detections between original and newly created object
      Re_Associate_Detections(
         calibs,
         raw_detect_list,
         det_p,
         object_to_split,
         new_obj);

      // Shift position and adapt both object properties
      Adapt_Objects_Properties_After_Orth_Split(host, calibs, sensors, globals, object_to_split, new_obj);
      
      // Insert the newly created track on the vcs-longitudinal sorted list
      Sorted_Tracks_Insert(tracker_info, &new_obj);

      return new_id;
   }

   /*===========================================================================*\
   * FUNCTION: Fill_New_Object_Properties()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object_to_split,
   * const int32_t new_obj_id,
   * const uint32_t new_unique_id,
   * F360_Object_Track_T& new_obj
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
   * This function copies relevant properties of the object that was splitted
   * to the newly created object
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Fill_New_Object_Properties(
      const F360_Object_Track_T& object_to_split,
      const int32_t new_obj_id,
      const uint32_t new_unique_id,
      F360_Object_Track_T& new_obj)
   {
      // Copy all properties of the object to split
      new_obj = object_to_split;

      // Adapt certain fields
      new_obj.id = new_obj_id;
      new_obj.reduced_id = F360_INVALID_REDUCED_ID;
      new_obj.reduced_status = F360_OBJECT_STATUS_INVALID;
      new_obj.unique_id = new_unique_id;
      new_obj.lsc_next_in_cluster = NULL;
      new_obj.lsc_prev_in_cluster = NULL;
      new_obj.p_higher_priority_track = NULL;
      new_obj.p_lower_priority_track = NULL;
   }

   /*===========================================================================*\
   * FUNCTION: Re_Associate_Detections()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibs,
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Object_Track_T& object_to_split,
   * F360_Object_Track_T& new_object
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
   * This function performs the association of detections that should be
   * associated to the original or the newly created object after a split.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Re_Associate_Detections(
      const F360_Calibrations_T& calibs,
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& object_to_split,
      F360_Object_Track_T& new_object)
   {
      uint32_t nr_dets_org_obj;
      uint32_t det_ids_org_obj[MAX_DETS_IN_OBJ_TRK] = {};
      uint32_t nr_dets_new_obj;
      uint32_t det_ids_new_obj[MAX_DETS_IN_OBJ_TRK] = {};
      Find_Re_Associated_Detections_Ids(
         object_to_split,
         det_p,
         nr_dets_org_obj,
         det_ids_org_obj,
         nr_dets_new_obj,
         det_ids_new_obj);

      Re_Associate_Detections_Single_Object(
         calibs,
         nr_dets_org_obj,
         det_ids_org_obj,
         raw_detect_list,
         det_p,
         object_to_split);

      Re_Associate_Detections_Single_Object(
         calibs,
         nr_dets_new_obj,
         det_ids_new_obj,
         raw_detect_list,
         det_p,
         new_object);

   }

   /*===========================================================================*\
   * FUNCTION: Find_Re_Associated_Detections_Ids()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Object_Track_T& object_to_split,
   * const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
   * uint32_t& nr_dets_org_obj,
   * int32_t(&org_obj_det_ids)[MAX_DETS_IN_OBJ_TRK],
   * uint32_t& nr_dets_new_obj,
   * int32_t(&new_obj_det_ids)[MAX_DETS_IN_OBJ_TRK]
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
   * This function evaluates which detections should be associated to which object
   * when an object needs to be splitted. Convention is that object that needs
   * to splitted will be shifted to the left in orthogonal direction while the
   * newly created object is shifted to the right. Thus, we determine detection
   * association based on the detections orthogonal position before the object is 
   * splitted. Detections with negative ortho position should be associated to
   * the original object while positive ortho position should be associated to
   * the new object that will be created.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Find_Re_Associated_Detections_Ids(
      const F360_Object_Track_T& object_to_split,
      const F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      uint32_t& nr_dets_org_obj,
      uint32_t(&org_obj_det_ids)[MAX_DETS_IN_OBJ_TRK],
      uint32_t& nr_dets_new_obj,
      uint32_t(&new_obj_det_ids)[MAX_DETS_IN_OBJ_TRK])
   {
      nr_dets_org_obj = 0U;
      nr_dets_new_obj = 0U;

      // Convert all associated detections into TCS
      float32_t orth_pos[MAX_DETS_IN_OBJ_TRK] = {};
      for (uint32_t i = 0U; i < object_to_split.ndets; i++)
      {
         const uint32_t det_idx = object_to_split.detids[i] - 1U;

         Point det_pos_tcs = {};
         Convert_VCS_Posn_To_TCS_Posn(
            det_p[det_idx].vcs_position.x,
            det_p[det_idx].vcs_position.y,
            object_to_split.bbox.Get_Center().x,
            object_to_split.bbox.Get_Center().y,
            object_to_split.bbox.Get_Orientation(),
            det_pos_tcs.x,
            det_pos_tcs.y);

         orth_pos[i] = det_pos_tcs.y;
      }

      for (uint32_t i = 0U; i < object_to_split.ndets; i++)
      {
         if (orth_pos[i] < 0.0F)
         {
            // Original object will shift to the left
            org_obj_det_ids[nr_dets_org_obj] = object_to_split.detids[i];
            nr_dets_org_obj++;
         }
         else
         {
            // New object will shift to the right
            new_obj_det_ids[nr_dets_new_obj] = object_to_split.detids[i];
            nr_dets_new_obj++;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Re_Associate_Detections_Single_Object()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibs,
   * const uint32_t& nr_dets,
   * const int32_t(&obj_det_ids)[MAX_DETS_IN_OBJ_TRK],
   * const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
   * F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
   * F360_Object_Track_T& object
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
   * This function performs the association of given detections to a
   * given object
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Re_Associate_Detections_Single_Object(
      const F360_Calibrations_T& calibs,
      const uint32_t nr_dets,
      const uint32_t(&obj_det_ids)[MAX_DETS_IN_OBJ_TRK],
      const rspp_variant_A::RSPP_Detection_List_T& raw_detect_list,
      F360_Detection_Props_T(&det_p)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& object)
   {
      object.ndets = nr_dets;
      object.num_types_of_dets[0] = 0;
      object.num_types_of_dets[1] = 0;
      std::fill(cmn::begin(object.detids), cmn::end(object.detids), 0U);
      for (uint32_t i = 0U; i < nr_dets; i++)
      {
         object.detids[i] = obj_det_ids[i];

         const uint32_t det_idx = obj_det_ids[i] - 1U;
         // Update counter of associated detection types
         if (rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING == raw_detect_list.detections[det_idx].processed.motion_status)
         {
            object.num_types_of_dets[0]++;
         }
         else
         {
            object.num_types_of_dets[1]++;
         }

         // Associate detection to object
         det_p[det_idx].object_track_id = object.id;
      }

      Find_Detection_Inliers_For_Single_Object(calibs, object, det_p);
   }

  /*===========================================================================*\
  * FUNCTION: Adapt_Objects_Properties_After_Orth_Split()
  * ===========================================================================
  * RETURN VALUE:
  * None.
  *
  * PARAMETERS:
  * const F360_Host_T & host
  * const F360_Calibrations_T& calibrations
  * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
  * const F360_Globals_T& globals
  * F360_Object_Track_T& org_object
  * F360_Object_Track_T& new_object
  * 
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
  * This function adapts both the splitted and the newly created object's
  * properties.
  *
  * PRECONDITIONS:
  * None
  *
  * POSTCONDITIONS:
  * None
  *
  \*===========================================================================*/
   void Adapt_Objects_Properties_After_Orth_Split(
      const F360_Host_T & host,
      const F360_Calibrations_T& calibrations,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Globals_T& globals,
      F360_Object_Track_T& org_object,
      F360_Object_Track_T& new_object)
   {
      org_object.reference_point = F360_REFERENCE_POINT_CENTER;
      org_object.min_projection_reference_point = F360_REFERENCE_POINT_CENTER;
      new_object.reference_point = F360_REFERENCE_POINT_CENTER;
      new_object.min_projection_reference_point = F360_REFERENCE_POINT_CENTER;

      // Shift position of newly created object - New object is shifted to the right in TCS
      Point new_updated_pos = {};

      Convert_TCS_Posn_To_VCS_Posn(
         0.0F,
         0.5F * org_object.orth_delta_filtered,
         org_object.bbox.Get_Center().x,
         org_object.bbox.Get_Center().y,
         org_object.bbox.Get_Orientation(),
         new_updated_pos.x,
         new_updated_pos.y);

      new_object.vcs_position.x = new_updated_pos.x;
      new_object.vcs_position.y = new_updated_pos.y;

      // Shift position of original object - Orginal object is shifted to the left in TCS
      Point org_updated_pos = {};

      Convert_TCS_Posn_To_VCS_Posn(
         0.0F,
         -0.5F * org_object.orth_delta_filtered,
         org_object.bbox.Get_Center().x,
         org_object.bbox.Get_Center().y,
         org_object.bbox.Get_Orientation(),
         org_updated_pos.x,
         org_updated_pos.y);

      org_object.vcs_position.x = org_updated_pos.x;
      org_object.vcs_position.y = org_updated_pos.y;
     
      // Update predicted position to mitigate drop in position confidence downstream
      org_object.predicted_vcs_position = org_object.vcs_position;
      new_object.predicted_vcs_position = new_object.vcs_position;

      // Adapt width and saturate to 1.8 m 
      const float32_t updated_width = std::min(0.5F * org_object.orth_delta_filtered * calibrations.k_orth_split_width_gain, calibrations.k_orth_split_width_threshold);
      org_object.bbox.Set_Width(updated_width);
      new_object.bbox.Set_Width(org_object.bbox.Get_Width());
      new_object.bbox.Set_Length(org_object.bbox.Get_Length());
      
      // Adapt heading, pointing and speed 
      org_object.vcs_heading = Angle{ org_object.filtered_pos_diff_heading };
      org_object.Set_Bbox_Orientation(org_object.vcs_heading);
      org_object.hdg_ptng_disagmt = 0.0F;
      org_object.curvature = 0.0F;
      org_object.heading_rate = 0.0F;
      const float32_t proj_of_old_speed_on_new_hdg_dir = org_object.vcs_velocity.longitudinal * org_object.vcs_heading.Cos() + org_object.vcs_velocity.lateral * org_object.vcs_heading.Sin();
      org_object.speed = proj_of_old_speed_on_new_hdg_dir;
      org_object.vcs_velocity.longitudinal = org_object.speed * org_object.vcs_heading.Cos();
      org_object.vcs_velocity.lateral = org_object.speed * org_object.vcs_heading.Sin();

      new_object.vcs_heading = org_object.vcs_heading;
      new_object.Set_Bbox_Orientation(org_object.bbox.Get_Orientation());
      new_object.hdg_ptng_disagmt = 0.0F;
      new_object.curvature = 0.0F;
      new_object.heading_rate = 0.0F;
      new_object.speed = org_object.speed;
      new_object.vcs_velocity.longitudinal = org_object.vcs_velocity.longitudinal;
      new_object.vcs_velocity.lateral = org_object.vcs_velocity.lateral;

      (void)std::copy(cmn::begin(calibrations.init_cca_pnt_filter_cov), cmn::end(calibrations.init_cca_pnt_filter_cov), cmn::begin(org_object.cca_pnt_filter_cov));
      (void)std::copy(cmn::begin(calibrations.init_cca_pnt_filter_cov), cmn::end(calibrations.init_cca_pnt_filter_cov), cmn::begin(new_object.cca_pnt_filter_cov));

      // Reset split signals
      org_object.orth_delta_filtered = 0.0F;
      new_object.orth_delta_filtered = 0.0F;
      org_object.orth_gap_filtered = 0.0F;
      new_object.orth_gap_filtered = 0.0F;

      org_object.filtered_pos_diff_heading = INFTY;
      new_object.filtered_pos_diff_heading = INFTY;
      org_object.prev_vcs_center_pos.x = org_object.bbox.Get_Center().x;
      org_object.prev_vcs_center_pos.y = org_object.bbox.Get_Center().y;
      new_object.prev_vcs_center_pos.x = new_object.bbox.Get_Center().x;
      new_object.prev_vcs_center_pos.y = new_object.bbox.Get_Center().y;
      
      /* 
      Update Reference Point - Update object position but don't adjust velocity estimates etc.
      Reason for not updating velocity etc is that curvature/yawrate estimate is not good for
      two objects tracked as one so we cant trust this value (curvature is set to 0 in above code)
      */
      Update_Object_Reference_Point(host.dist_rear_axle_to_vcs_m, true, true, calibrations, sensors, globals, org_object);
      Update_Object_Reference_Point(host.dist_rear_axle_to_vcs_m, true, true, calibrations, sensors, globals, new_object);
   }
}

