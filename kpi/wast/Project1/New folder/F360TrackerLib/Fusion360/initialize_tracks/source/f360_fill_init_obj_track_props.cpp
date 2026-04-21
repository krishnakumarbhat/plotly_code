/*===========================================================================*\
* FILE: f360_fill_init_obj_track_props.cpp
*============================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*---------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function definition for Fill_Init_Obj_Track_Props.
*
*
* Applicable Standards (in order of precedence: highest first):
* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
* ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

#include <algorithm>
#include "f360_fill_init_obj_track_props.h"
#include "f360_math_func.h"
#include "f360_trk_fltr_cca_states.h"
#include "f360_trk_fltr_ctca_states.h"
#include "f360_current_msmt_type.h"
#include "f360_uncertainty_propagation.h"
#include "f360_calc_object_init_ep.h"
#include "f360_iterator.h"
#include "f360_static_env_polys_support_functions.h"
#include "f360_update_object_reference_point.h"

namespace f360_variant_A
{
   static void Adjust_Init_Pos_If_No_Dets_Inside_Solid_Circle(
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Calibrations_T& calibrations,
      F360_Object_Track_T& obj_trk_to_init);

   static bool Calc_Init_Obj_Track_Moving_Flag(
      const float32_t object_speed,
      const float32_t object_speed_thresh_stat_mov,
      const float32_t frac_az_error,
      const float32_t frac_az_error_thresh_stat_mov,
      const bool f_valid_for_liberal_tracking);

   static Angle Calc_Init_Obj_VCS_Heading(
      const bool f_moving,
      const F360_VCS_Velocity_T vcs_velocity);

   static void Update_Object_Vehicular_Info(
      F360_Object_Track_T& object_track,
      const float32_t obj_vehicular_spd_thresh);

   /*===========================================================================*\
   * FUNCTION: Fill_Init_Obj_Track_Props
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * const F360_Globals_T& globals
   * const F360_Calibrations_T& calibrations
   * const F360_Host_T& host
   * const F360_Tracked_Object_Init_Info_T& init_info
   * const F360_Cluster_T& cluster_to_init
   * const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS]
   * const uint32_t new_unique_id
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS]
   * F360_Object_Track_T& obj_trk_to_init
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Function which is responsible for filling values for valid initialized track
   * based on different sources.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/

   void Fill_Init_Obj_Track_Props(
      const F360_Globals_T& globals,
      const F360_Calibrations_T& calibrations,
      const F360_Host_T& host,
      const F360_Tracked_Object_Init_Info_T& init_info,
      const F360_Cluster_T& cluster_to_init,
      const Static_Env_Poly_T(&sep)[F360_NUM_OF_STATIC_ENV_POLYS],
      const uint32_t new_unique_id,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& obj_trk_to_init)
   {
      obj_trk_to_init.unique_id = new_unique_id;
      obj_trk_to_init.status = F360_OBJECT_STATUS_NEW_UPDATED;
      obj_trk_to_init.current_msmt_type = F360_MSMT_TYPE_DETS_ONLY;
      obj_trk_to_init.raw_confidence_level = calibrations.k_init_trk_init_confidence;
      obj_trk_to_init.confidenceLevel = calibrations.k_init_trk_init_confidence;
      obj_trk_to_init.f_valid_for_liberal_tracking = init_info.f_valid_for_liberal_tracking;
      obj_trk_to_init.conf_longitudinal_position = CONF9_NONE;
      obj_trk_to_init.conf_lateral_position = CONF9_NONE;
      obj_trk_to_init.conf_longitudinal_velocity = CONF9_NONE;
      obj_trk_to_init.conf_lateral_velocity = CONF9_NONE;
      obj_trk_to_init.conf_speed = CONF9_NONE;

      // Defaults which should be the same as for cleared object but they are initialized by 0 just to be sure that they are set correctly.
      obj_trk_to_init.hdg_ptng_disagmt = 0.0F;
      obj_trk_to_init.time_since_stage_start = 0.0F;
      obj_trk_to_init.time_since_initialization = 0.0F;
      obj_trk_to_init.time_since_track_updated = 0.0F;
      obj_trk_to_init.time_since_split = -1.0F;
      obj_trk_to_init.vcs_accel.longitudinal = 0.0F;
      obj_trk_to_init.vcs_accel.lateral = 0.0F;
      obj_trk_to_init.tang_accel = 0.0F;
      obj_trk_to_init.curvature = 0.0F;
      obj_trk_to_init.heading_rate = 0.0F;
      obj_trk_to_init.num_updates_since_init = 0U;

      // Utilize cluster information.
      obj_trk_to_init.exist_prob = Calc_Object_Init_Existence_Prob(cluster_to_init.exist_prob, init_info.VCS_velocity_plausability, calibrations);
      obj_trk_to_init.priority = cluster_to_init.priority;
      obj_trk_to_init.time_since_cluster_created = cluster_to_init.time_since_created;
      obj_trk_to_init.time_since_measurement = cluster_to_init.time_since_measurement;

      // Utilize initialization information.
      obj_trk_to_init.init_scheme = init_info.init_scheme;

      // Properties related to init velocity.
      obj_trk_to_init.init_vel_source = init_info.init_vel_source;
      obj_trk_to_init.vcs_velocity.longitudinal = init_info.VCS_velocity.longitudinal;
      obj_trk_to_init.vcs_velocity.lateral = init_info.VCS_velocity.lateral;

      obj_trk_to_init.speed = F360_Get_Hypotenuse(init_info.VCS_velocity.longitudinal, init_info.VCS_velocity.lateral);
      obj_trk_to_init.f_moving = Calc_Init_Obj_Track_Moving_Flag(obj_trk_to_init.speed, globals.obj_mov_stat_spd_thresh,
         init_info.frac_az_error_stat_mov, calibrations.k_init_trk_max_frac_az_error_stat_mov, init_info.f_valid_for_liberal_tracking); // TODO: DFT-434 consider using logic from Motion_Classification function (there should be common part extracted).
      obj_trk_to_init.f_moveable = obj_trk_to_init.f_moving;

      Update_Object_Vehicular_Info(obj_trk_to_init, globals.obj_vehicular_spd_thresh); // TODO: DFT-434 consider using logic same logic as elsewhere. Common parts should be extracted

      obj_trk_to_init.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;
      obj_trk_to_init.vcs_heading = Calc_Init_Obj_VCS_Heading(obj_trk_to_init.f_moving, init_info.VCS_velocity);

      // Properties related to init position, reference point and object dimensions.
      Calc_Init_Position_And_Size(init_info, host, calibrations, globals, sensors, cluster_to_init, det_props, obj_trk_to_init);

      // Other properties.
      Init_Obj_Track_Covariances(host, calibrations, init_info, globals.obj_vehicular_spd_thresh, obj_trk_to_init);

      obj_trk_to_init.low_rcs_dets_cnt = cluster_to_init.low_rcs_dets_cnt;

      Flag_Single_Object_On_And_Behind_SEP(sep, calibrations, obj_trk_to_init);
   }

   /*===========================================================================*\
   * FUNCTION: Init_Obj_Track_Covariances
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Calibrations_T & calibrations
   * const F360_Tracked_Object_Init_Info_T & init_info
   * F360_Object_Track_T & obj_trk_to_init
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
   * Initialize Track Object covariances
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Init_Obj_Track_Covariances(
      const F360_Host_T& host,
      const F360_Calibrations_T & calibrations,
      const F360_Tracked_Object_Init_Info_T & init_info,
      const float32_t & obj_mov_stat_spd_thresh,
      F360_Object_Track_T & obj_trk_to_init)
   {
      std::fill(cmn::begin(obj_trk_to_init.errcov), cmn::end(obj_trk_to_init.errcov), 0.0F);

      // Compute position covariance
      const float32_t host_center_vcs_pos[2] = { -0.6F * host.dist_rear_axle_to_vcs_m, 0.0F };
      const float32_t vec_host_center_to_obj[2] = { obj_trk_to_init.vcs_position.x - host_center_vcs_pos[0], obj_trk_to_init.vcs_position.y - host_center_vcs_pos[1] };
      const float32_t range_host_center_to_obj = F360_Get_Hypotenuse(vec_host_center_to_obj[0], vec_host_center_to_obj[1]);
      const float32_t angle_host_center_to_obj = F360_Atan2f(vec_host_center_to_obj[1], vec_host_center_to_obj[0]);
      const float32_t cos_angle_host_center_to_obj = F360_Cosf(angle_host_center_to_obj);
      const float32_t sin_angle_host_center_to_obj = F360_Sinf(angle_host_center_to_obj);
      const float32_t cross_range_variance = std::max(range_host_center_to_obj * range_host_center_to_obj * calibrations.k_init_cca_az_var, calibrations.k_init_cca_range_var);
      const float32_t range_cross_range_pos_var_mat[2][2] = { {calibrations.k_init_cca_range_var, 0.0F}, {0.0F, cross_range_variance} };
      float32_t vcs_pos_var[2][2];
      Rotate_2D_Covariance_Matrix(cos_angle_host_center_to_obj, sin_angle_host_center_to_obj, range_cross_range_pos_var_mat, vcs_pos_var);

      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_X] =
         vcs_pos_var[F360_2D_IDX_X][F360_2D_IDX_X];
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_X][F360_TRK_FLTR_CCA_STATE_Y] =
         vcs_pos_var[F360_2D_IDX_X][F360_2D_IDX_Y];
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_X] =
         vcs_pos_var[F360_2D_IDX_Y][F360_2D_IDX_X];
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_Y][F360_TRK_FLTR_CCA_STATE_Y] =
         vcs_pos_var[F360_2D_IDX_Y][F360_2D_IDX_Y];

      // Propagate velocity covariance from NEES estimation
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VX] =
         calibrations.k_init_vel_errcov_scale_factor * init_info.VCS_velocity_cov[F360_2D_IDX_X][F360_2D_IDX_X];
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_VX][F360_TRK_FLTR_CCA_STATE_VY] =
         calibrations.k_init_vel_errcov_scale_factor * init_info.VCS_velocity_cov[F360_2D_IDX_X][F360_2D_IDX_Y];
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VX] =
         calibrations.k_init_vel_errcov_scale_factor * init_info.VCS_velocity_cov[F360_2D_IDX_Y][F360_2D_IDX_X];
      obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_VY][F360_TRK_FLTR_CCA_STATE_VY] =
         calibrations.k_init_vel_errcov_scale_factor * init_info.VCS_velocity_cov[F360_2D_IDX_Y][F360_2D_IDX_Y];

      // Compute acceleration covariance
      if (!obj_trk_to_init.f_moveable)
      {
         obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = calibrations.k_init_cca_min_acc_var;
         obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = calibrations.k_init_cca_min_acc_var;
      }
      else
      {
         const float32_t accel_error = F360_Linear_Equation_With_Saturation(obj_trk_to_init.speed, obj_mov_stat_spd_thresh,
            calibrations.k_init_cca_k_obj_speed_for_max_acc_var, calibrations.k_init_cca_min_acc_var, calibrations.k_init_cca_max_acc_var);
         obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_AX][F360_TRK_FLTR_CCA_STATE_AX] = accel_error;
         obj_trk_to_init.errcov[F360_TRK_FLTR_CCA_STATE_AY][F360_TRK_FLTR_CCA_STATE_AY] = accel_error;
      }

      // Initialize covariance matrix for pointing filter
      (void)std::copy(cmn::begin(calibrations.init_cca_pnt_filter_cov), cmn::end(calibrations.init_cca_pnt_filter_cov), cmn::begin(obj_trk_to_init.cca_pnt_filter_cov));
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Init_Position_And_Size
   *===========================================================================
   * RETURN VALUE:
   *
   * PARAMETERS:
   * const F360_Tracked_Object_Init_Info_T& init_info
   * const F360_Host_T& host
   * const F360_Calibrations_T& calibrations
   * const F360_Globals_T& globals
   * const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
   * F360_Object_Track_T& obj_trk_to_init
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
   * Function computes the initial placement of object bounding box and its initial dimensions.
   * Function also determines the initial reference point of the object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calc_Init_Position_And_Size(
      const F360_Tracked_Object_Init_Info_T& init_info,
      const F360_Host_T& host,
      const F360_Calibrations_T& calibrations,
      const F360_Globals_T& globals,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Cluster_T &cluster_to_init,
      const F360_Detection_Props_T (&det_props)[MAX_NUMBER_OF_DETECTIONS],
      F360_Object_Track_T& obj_trk_to_init)
   {
      if (obj_trk_to_init.f_moveable)
      {
         /* For moveable objects do the following :
            1. Use the estimated bbox containing all the cluster detections and use reference point in CENTER
            2. Compute a new reference point.
            3. Adjust object size (i.e. allow growth/shrinkage only away from the choosen reference point) */
         obj_trk_to_init.reference_point = F360_REFERENCE_POINT_CENTER;
         obj_trk_to_init.min_projection_reference_point = F360_REFERENCE_POINT_CENTER;
         obj_trk_to_init.bbox = init_info.det_bbox;
         obj_trk_to_init.vcs_position = obj_trk_to_init.bbox.Get_Center();
         Update_Object_Reference_Point(host.dist_rear_axle_to_vcs_m, true, true, calibrations, sensors, globals, obj_trk_to_init);
         Determine_Initial_Size(globals, calibrations, obj_trk_to_init); // TODO: DFT-434 consider using logic same logic as elsewhere. Common parts should be extracted.
      }
      else
      {
         // For nonmovable objects we use center as reference point and default dimension.
         obj_trk_to_init.reference_point = F360_REFERENCE_POINT_CENTER;
         obj_trk_to_init.min_projection_reference_point = F360_REFERENCE_POINT_CENTER;
         obj_trk_to_init.bbox.Set_Center(cluster_to_init.vcs_position);
         obj_trk_to_init.vcs_position = obj_trk_to_init.bbox.Get_Center();
         obj_trk_to_init.Set_Bbox_Orientation(obj_trk_to_init.vcs_heading);
         obj_trk_to_init.bbox.Set_Length(calibrations.k_nonmoveable_target_diameter);
         obj_trk_to_init.bbox.Set_Width(calibrations.k_nonmoveable_target_diameter);

         Adjust_Init_Pos_If_No_Dets_Inside_Solid_Circle(cluster_to_init, det_props, calibrations, obj_trk_to_init);
      }
   }


   /*===========================================================================*\
    * FUNCTION: Adjust_Init_Pos_If_No_Dets_Inside_Solid_Circle()
    *===========================================================================
    * RETURN VALUE:
    *
    * PARAMETERS:
    * const F360_Cluster_T& cluster_to_init,
    * const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
    * const F360_Calibrations_T& calibrations,
    * F360_Object_Track_T& obj_trk_to_init
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
    * This function checks if there are any current detection from the cluster inside
    * of the intial object bbox (a circle). If not then the object position is adjusted
    * (to be closer to a detection). Object position is then adjusted such that at least
    * one detection ends up at the border of the object bbox.
    *
    * PRECONDITIONS:
    * Object should have a circular bounding box (i.e. object should be non-moveable)
    *
    * POSTCONDITIONS:
    * None
    *
   \*===========================================================================*/
   static void Adjust_Init_Pos_If_No_Dets_Inside_Solid_Circle(
      const F360_Cluster_T& cluster_to_init,
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Calibrations_T& calibrations,
      F360_Object_Track_T& obj_trk_to_init)
   {
      if (cluster_to_init.ndets > 1)
      {
         // Loop over all current detection in cluster.
         // Check if any detection is inside the solid circle of the object.
         // Extract longitudinal position of detections.
         bool is_any_det_in_gating = false;
         float32_t det_long_pos_array[MAX_DETS_IN_OBJ_TRK];
         for (int16_t i = 0; i < cluster_to_init.ndets; i++)
         {
            const int16_t det_idx = cluster_to_init.detids[i] - 1;
            if (obj_trk_to_init.bbox.Circle_Contains(det_props[det_idx].vcs_position))
            {
               is_any_det_in_gating = true;
               break;
            }
            det_long_pos_array[i] = det_props[det_idx].vcs_position.x;
         }

         // If no detection were inside of the object solid circle then the initial position of the object has to be adjusted
         // (Due to small association gates for non-moveable objects we will not be able to associate new detections otherwise.)
         if (!is_any_det_in_gating)
         {
            // Find the index of the detection with median longitudinal position
            uint32_t perm[MAX_DETS_IN_OBJ_TRK];
            (void)F360_Sort(det_long_pos_array, static_cast<uint32_t>(cluster_to_init.ndets), true, perm);
            const int16_t median_sorted_array_idx = (cluster_to_init.ndets - 1) / 2; // Note: It is intentionall to round the division downwards.
            const int16_t median_det_idx = cluster_to_init.detids[perm[median_sorted_array_idx]] - 1;

            // Compute vector from the choosen detection to the cluster center
            const Point median_det_pos = det_props[median_det_idx].vcs_position;
            const float32_t vec_from_cluster_center_to_det[2] = { cluster_to_init.vcs_position.x - median_det_pos.x ,
               cluster_to_init.vcs_position.y - median_det_pos.y };

            // Normalize the vector such that it becomes a unit vector
            const float32_t dist = F360_Get_Hypotenuse(vec_from_cluster_center_to_det[0], vec_from_cluster_center_to_det[1]);
            const float32_t inv_dist = 1.0F / dist;
            const float32_t unit_vec_from_cluster_center_to_det[2] = { vec_from_cluster_center_to_det[0] * inv_dist,
               vec_from_cluster_center_to_det[1] * inv_dist };

            // Move object position along the line from choosen detection to the cluster center such that
            // the detection ends up at the border of the solid circle
            const float32_t circle_radius = calibrations.k_nonmoveable_target_diameter * 0.5F;
            const Point new_obj_pos = { median_det_pos.x + circle_radius * unit_vec_from_cluster_center_to_det[0] ,
               median_det_pos.y + circle_radius * unit_vec_from_cluster_center_to_det[1] };

            // Adjust the object position
            obj_trk_to_init.bbox.Set_Center(new_obj_pos);
            obj_trk_to_init.vcs_position = new_obj_pos;
         }
      }
   }



   /*===========================================================================*\
   * FUNCTION: Calc_Init_Obj_Track_Moving_Flag
   *===========================================================================
   * RETURN VALUE:
   * bool - indication if object is moving or not
   *
   * PARAMETERS:
   *
   * const float32_t object_speed,
   * const float32_t obj_mov_stat_spd_thresh,
   * const float32_t frac_az_error_stat_mov,
   * const float32_t max_frac_az_error_stat_mov
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
   * Calculate moving flag for initialized object
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static bool Calc_Init_Obj_Track_Moving_Flag(
      const float32_t object_speed,
      const float32_t object_speed_thresh_stat_mov,
      const float32_t frac_az_error,
      const float32_t frac_az_error_thresh_stat_mov,
      const bool f_valid_for_liberal_tracking)
   {
      const bool speed_above_thr = object_speed > object_speed_thresh_stat_mov;
      const bool frac_az_err_below_thr = frac_az_error < frac_az_error_thresh_stat_mov;

      return (speed_above_thr && (f_valid_for_liberal_tracking || frac_az_err_below_thr));
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Init_Obj_VCS_Heading
   *===========================================================================
   * RETURN VALUE:
   * Angle vcs_heading - initialized VCS heading angle
   *
   * PARAMETERS:
   *
   * const bool f_moving
   * const F360_VCS_Velocity_T vcs_velocity
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
   * Calculate VCS heading angle for initialized object
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static Angle Calc_Init_Obj_VCS_Heading(
      const bool f_moving,
      const F360_VCS_Velocity_T vcs_velocity)
   {
      Angle vcs_heading{};
      if (f_moving)
      {
         (void)vcs_heading.Value(F360_Atan2f(vcs_velocity.lateral, vcs_velocity.longitudinal));
      }
      else
      {
         (void)vcs_heading.Value(0.0F);
      }
      return vcs_heading;
   }

   /*===========================================================================*\
   * FUNCTION: Update_Object_Vehicular_Info
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *
   * F360_Object_Track_T& object_track,
   * const float32_t obj_vehicular_spd_thresh
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
   * Update information related to indication if object can be treat as vehicular type or not
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Update_Object_Vehicular_Info(
      F360_Object_Track_T& object_track,
      const float32_t obj_vehicular_spd_thresh)
   {
      if ((object_track.f_moving) && (object_track.speed > obj_vehicular_spd_thresh))
      {
         object_track.f_vehicular_trk = true;
         object_track.time_since_vehicle_init = 0.0F;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Determine_Initial_Size
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Globals_T & globals
   * const F360_Calibrations_T & calibrations
   * F360_Object_Track_T& obj_trk_to_init
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
   * Determines the intial size of an object based on its movement status and speed.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Determine_Initial_Size(
      const F360_Globals_T & globals,
      const F360_Calibrations_T & calibrations,
      F360_Object_Track_T& obj_trk_to_init)
   {
      if (obj_trk_to_init.speed < globals.obj_mov_stat_spd_thresh)
      {
         // Stationary object, use small bounding box
         obj_trk_to_init.bbox.Set_Length(calibrations.k_init_trk_stationary_length);
         obj_trk_to_init.bbox.Set_Width(calibrations.k_init_trk_stationary_width);
      }
      else
      {
         // Moving object, use more logic
         if (obj_trk_to_init.speed > calibrations.k_init_trk_fast_moving_thresh)
         {
            // Speed above 20 km/h
            if (obj_trk_to_init.f_valid_for_liberal_tracking)
            {
               obj_trk_to_init.bbox.Set_Length(calibrations.liberal_tracking_initial_length);
            }
            else
            {
               obj_trk_to_init.bbox.Set_Length(calibrations.k_init_trk_fast_moving_length);
            }
            obj_trk_to_init.bbox.Set_Width(calibrations.k_init_trk_fast_moving_width);
         }
         else
         {
            obj_trk_to_init.bbox.Set_Length(calibrations.k_init_trk_slow_moving_length);
            obj_trk_to_init.bbox.Set_Width(calibrations.k_init_trk_slow_moving_width);
         }
      }

      obj_trk_to_init.Update_Bbox_Center();

      // Initialize uncertainties for size filter
      obj_trk_to_init.length_uncertainty = calibrations.k_size_filter_length_uncertainty;
      obj_trk_to_init.width_uncertainty = calibrations.k_size_filter_width_uncertainty;
   }
}
