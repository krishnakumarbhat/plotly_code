/*===================================================================================*\
* FILE: f360_tracker.cpp
*====================================================================================
* Copyright 2023 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: AIT-69%
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains all functions of F360_Tracker class which form the tracker feature
*
* ABBREVIATIONS:
*   OTG	Over-The-ground
*
* TRACEABILITY INFO:
*   Design Document(s): All F360 tracker .m scripts apart from host_props.m
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/


/******************************
* Includes
*******************************/
#include <cstring>
#include <algorithm>
#include "f360_math.h"
#include "f360_math_func.h"

#include "f360_tracker.h"
#include "f360_tracker_version.h"
#include "f360_inputs_preprocessing.h"
#include "f360_internal_preprocessing.h"
#include "f360_sensor_postprocessing.h"
#include "f360_time_update_tracks.h"
#include "f360_pre_association_track_management.h"
#include "f360_detection_to_track_association.h"
#include "f360_clustering.h"
#include "f360_cluster_grouping.h"
#include "f360_track_grouping.h"
#include "f360_initialize_tracks.h"
#include "f360_measurement_update_tracks.h"
#include "f360_post_update_track_adjustments.h"
#include "f360_track_classification.h"
#include "f360_track_validity.h"
#include "f360_track_downselection.h"
#include "f360_e2e_protection.h"
#include "f360_sanity_check.h"
#include "f360_get_wall_time.h"
#include "f360_sorted_tracks_mgmt.h"
#include "f360_sort_priority.h"
#include "f360_update_relative_timestamps.h"
#include "f360_sorted_clusters_mgmt.h"
#include "f360_iterator.h"
#include "f360_occlusion.h"
#include "f360_static_environment_class.h"

#include "f360_populate_detections_log.h"
#include "f360_populate_objects_log.h"
#include "f360_populate_internal_clusters_log.h"
#include "f360_populate_internal_cwd_log.h"
#include "f360_populate_internal_detection_history_log.h"
#include "f360_populate_internal_objects_log.h"
#include "f360_populate_internal_reflection_buffer_log.h"
#include "f360_populate_internal_trailer_detector_log.h"
#include "f360_rot_object_output.h"

#include "TrackerInternal.h"
#include "f360_clear_detections_props.h"
#include "f360_clear_cluster.h"
#include "f360_clear_object_track.h"
#include "f360_trailer_detector_core.h"

#ifdef _DEBUG
#include "f360_xtrk_logging.h"
#endif

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

static f360_variant_A::F360_Detection_Props_T detections[f360_variant_A::MAX_NUMBER_OF_DETECTIONS] = {};
static f360_variant_A::F360_Detection_Hist_T det_hist;
static f360_variant_A::F360_Cluster_T clusters[f360_variant_A::NUMBER_OF_CLUSTERS] = {};
static f360_variant_A::F360_Object_Track_T object_tracks[f360_variant_A::NUMBER_OF_OBJECT_TRACKS] = {};
static f360_variant_A::F360_Tracker_Info_T tracker_info = {};
static f360_variant_A::F360_Host_Props_T host_props;
static f360_variant_A::F360_Radar_Sensor_Props_T sensor_props[f360_variant_A::MAX_NUMBER_OF_SENSORS] = {};
static f360_variant_A::F360_Calibrations_T calibrations = {};
static f360_variant_A::F360_Globals_T globals = {};
static f360_variant_A::F360_TRKR_TIMING_INFO_T timing_info = {};
static f360_variant_A::Static_Env_T static_environment = {};
static f360_variant_A::Trailer_Detector_Core trailer_detector_core = {};

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::F360_Tracker()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * constructor
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   F360_Tracker::F360_Tracker()
   {

   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::~F360_Tracker()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * destructor
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   F360_Tracker::~F360_Tracker()
   {
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function is used to initialize the member variables
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize() const
   {
      //Copyright fingerprint string is not used so warning must be ignored
      
      #if defined(_MSC_VER) 
         #pragma warning(push)
         #pragma warning(disable : 4189)
      #elif GCC_VERSION > 40600 && !defined(__TASKING__) /* Test for GCC > 4.6.0 */
         #pragma GCC diagnostic push
         #pragma GCC diagnostic ignored "-Wunused-variable" 
      #else
         //Do nothing
      #endif
      volatile const char ACS_Copyright_Full[] = "Copyright \xA9 2023 Aptiv Advanced Safety and User Experience. All rights reserved.";
      #if defined(_MSC_VER)
         #pragma warning(pop)
      #elif GCC_VERSION > 40600  && !defined(__TASKING__) /* Test for GCC > 4.6.0 */
         #pragma GCC diagnostic pop
      #else
         //Do nothing
      #endif 

      this->Initialize_Variant();
      this->Initialize_Host_Props();
      this->Initialize_Sensor_Props();
      this->Initialize_Clusters();
      this->Initialize_Obj_Trck();
      this->Initialize_Trkr_Info();
      this->Initialize_Calibrations();
      this->Initialize_Globals();
      this->Initialize_Det_Prop();
      this->Initialize_Det_Hist();
      this->Initialize_Timing_Info();
      this->Initialize_Static_Env_Polys();
      this->Initialize_Trailer_Detector();
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function is used to initialize the member variables
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize(const F360_Variant_T& variant) const
   {
      this->Initialize_Variant(variant);
      this->Initialize_Host_Props();
      this->Initialize_Sensor_Props();
      this->Initialize_Clusters();
      this->Initialize_Obj_Trck();
      this->Initialize_Trkr_Info();
      this->Initialize_Calibrations();
      this->Initialize_Globals();
      this->Initialize_Det_Prop();
      this->Initialize_Det_Hist();
      this->Initialize_Timing_Info();
      this->Initialize_Static_Env_Polys();
   }

   /*===========================================================================*\
    * FUNCTION: F360_Tracker::Reset()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * None
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
    * This function is used to reset the member variables
    *
    * PRECONDITIONS:
    * To be called by an object/reference of F360_Tracker.
    *
    * POSTCONDITIONS:
    * None
    *
   \*===========================================================================*/
   void F360_Tracker::Reset() const
   {
      this->Initialize_Host_Props();
      this->Initialize_Sensor_Props();
      this->Initialize_Clusters();
      this->Initialize_Obj_Trck();
      this->Initialize_Trkr_Info();
      this->Initialize_Globals();
      this->Initialize_Det_Prop();
      this->Initialize_Det_Hist();
      this->Initialize_Timing_Info();
      this->Initialize_Static_Env_Polys();
      this->Initialize_Trailer_Detector();
   }

   /*===========================================================================*\
    * FUNCTION: F360_Tracker::Initialize_Tracker_State_From_Log()
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    *    const F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS]
    *    const F360_Host_T& r_host
    *    const All_Objects_Log_T& r_all_objects
    *    const Tracker_Info_Log_T& r_tracker_info
    *    const F360_Host_Props_Log_T& r_host_props
    *    const F360_Static_Env_Poly_Log_T& r_static_env_polys
    *    const Tracker_Internal_Log_T& r_tracker_internals
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
    * This function is used to initialize the tracker states from logged data
    *
    * PRECONDITIONS:
    * To be called by an object/reference of F360_Tracker.
    *
    * POSTCONDITIONS:
    * None
    *
   \*===========================================================================*/
   bool F360_Tracker::Initialize_Tracker_State_From_Log(
      const F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Host_T& r_host,
      const F360_Object_Log_Output_T& object_log,
      const Tracker_Info_Log_T& r_tracker_info,
      const F360_Host_Props_Log_T& r_host_props,
      const F360_Static_Env_Poly_Log_T& r_static_env_polys,
      const Trailer_Detector_Log_T& r_trailer_detector,
      const Tracker_Internal_T& r_tracker_internals,
      const F360_Internal_Trailer_Detector_T& r_trailer_detector_internal) const
   {
      bool f_init_ok = true;

      // Tracker Info
      tracker_info.time_us = r_tracker_info.timestamp_us;
      tracker_info.object_list_timestamp = r_tracker_info.object_list_timestamp;
      tracker_info.cnt_loops = r_tracker_info.tracker_index;
      tracker_info.elapsed_time_s = r_tracker_info.elapsed_time_s;

      // Host Props
      host_props.position.x = r_host_props.position_x;
      host_props.position.y = r_host_props.position_y;
      host_props.heading_angle = r_host_props.heading;
      host_props.cos_heading = F360_Cosf(host_props.heading_angle);
      host_props.sin_heading = F360_Sinf(host_props.heading_angle);
      host_props.delta_pointing = r_host_props.delta_pointing;
      host_props.delta_position.x = r_host_props.delta_position_x;
      host_props.delta_position.y = r_host_props.delta_position_y;
      for (int8_t i = 0; i < 2; i++)
      {
         for (int8_t j = 0; j < 2; j++)
         {
            host_props.vel_cov_scm[i][j] = r_host_props.vel_cov_scm[i][j];
            host_props.vel_cov[i][j] = r_host_props.vel_cov[i][j];
            host_props.position_inc_cov_scm[i][j] = r_host_props.position_inc_cov_scm[i][j];
            host_props.position_inc_cov[i][j] = r_host_props.position_inc_cov[i][j];
         }
      }
      host_props.std_speed_scm = r_host_props.std_speed_scm;
      host_props.std_yaw_rate_scm = r_host_props.std_yaw_rate_scm;

      // Object Tracks
      Populate_Objects_Data(object_tracks, tracker_info, object_log.object, calibrations, r_host);

      // Detection History
      Populate_Internal_Detection_History_Data(det_hist, r_tracker_internals.det_hist);

      // Clusters
      Populate_Internal_Clusters_Data(clusters, tracker_info, r_tracker_internals.clusters);

      // Compute cluster detids and old_det_idx
      int16_t cluster_idx = 0;
      for (int16_t hist_det_idx = 0; hist_det_idx < static_cast<int16_t>(MAX_NUMBER_OF_HISTORIC_DETECTIONS); hist_det_idx++)
      {
         if (det_hist.f_idx_occupied[hist_det_idx])
         {
            cluster_idx = det_hist.det_data[hist_det_idx].cluster_idx;
            clusters[cluster_idx].old_det_idx[clusters[cluster_idx].num_old_dets] = hist_det_idx;
            clusters[cluster_idx].num_old_dets++;
         }
      }

      // Objects
      Populate_Internal_Objects_Data(object_tracks, calibrations, r_tracker_internals.obj);

      // CWD
      Populate_Internal_CWD_Data(static_environment, r_tracker_internals.cwd, r_sensors, calibrations);

      // Trailer Detector
      Populate_Internal_Trailer_Detector_Data(trailer_detector_core, r_trailer_detector_internal, r_trailer_detector);

      // Static environment polynomials
      Static_Env_Poly_T static_polys[F360_NUM_OF_STATIC_ENV_POLYS]{};
      for (uint8_t i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
      {
         static_polys[i].status = static_cast<Static_Env_Poly_Status_T>(r_static_env_polys.static_env_polys[i].status);
         static_polys[i].poly_type = static_cast<Static_Env_Poly_Type_T>(r_static_env_polys.static_env_polys[i].poly_type);
         static_polys[i].age = r_static_env_polys.static_env_polys[i].age;
         static_polys[i].confidence = r_static_env_polys.static_env_polys[i].confidence;
         static_polys[i].lower_limit = r_static_env_polys.static_env_polys[i].lower_limit;
         static_polys[i].upper_limit = r_static_env_polys.static_env_polys[i].upper_limit;
         static_polys[i].p0 = r_static_env_polys.static_env_polys[i].p0;
         static_polys[i].p1 = r_static_env_polys.static_env_polys[i].p1;
         static_polys[i].p2 = r_static_env_polys.static_env_polys[i].p2;
      }
      static_environment.Set_Polynomials(static_polys);

      // Internal reflection buffers
      Populate_Internal_Reflection_Buffer_Data(sensor_props, r_tracker_internals.reflection_buffer);

      // Rebuild_Sorted_Info
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[i] - 1;
         Sort_Priority_With_New_Track(tracker_info, &object_tracks[obj_idx]);
         Sorted_Tracks_Insert(tracker_info, &object_tracks[obj_idx]);
      }
      Sorted_Tracks_Re_Sort(tracker_info);

      for (int16_t i = 0; i < tracker_info.num_active_clusters; i++)
      {
         const int16_t idx = tracker_info.active_cluster_ids[i] - 1;
         Sorted_Clusters_Insert(tracker_info, clusters, clusters[idx], 0);
      }
      Sorted_Clusters_Update_List(tracker_info);

      // Assert that the active and inactive object lists are packed
      for (uint32_t i = 1U; i < NUMBER_OF_OBJECT_TRACKS; i++)
      {
         const bool f_activeNOK = (0 == tracker_info.active_obj_ids[i - 1U]) && (0 < tracker_info.active_obj_ids[i]);
         const bool f_inactiveNOK = (0 == tracker_info.inactive_obj_ids[i - 1U]) && (0 < tracker_info.inactive_obj_ids[i]);
         f_init_ok = f_init_ok && (false == f_activeNOK) && (false == f_inactiveNOK);
      }
      // Assert that the active and inactive cluster lists are packed
      for (uint16_t i = 1U; i < NUMBER_OF_CLUSTERS; i++)
      {
         const bool f_activeNOK = (0 == tracker_info.active_cluster_ids[i - 1U]) && (0 < tracker_info.active_cluster_ids[i]);
         const bool f_inactiveNOK = (0 == tracker_info.inactive_cluster_ids[i - 1U]) && (0 < tracker_info.inactive_cluster_ids[i]);
         f_init_ok = f_init_ok && (false == f_activeNOK) && (false == f_inactiveNOK);
      }
      // Assert that the active and inactive reduced object lists are packed
      for (uint32_t i = 1U; i < NUMBER_OF_REDUCED_OBJECT_TRACKS; i++)
      {
         const bool f_activeNOK = (0 == tracker_info.reduced_active_obj_ids[i - 1U]) && (0 < tracker_info.reduced_active_obj_ids[i]);
         const bool f_inactiveNOK = (0 == tracker_info.reduced_inactive_obj_ids[i - 1U]) && (0 < tracker_info.reduced_inactive_obj_ids[i]);
         f_init_ok = f_init_ok && (false == f_activeNOK) && (false == f_inactiveNOK);
      }

      // Assert object ID:s are correct
      bool f_obj_id_used[NUMBER_OF_OBJECT_TRACKS];
      std::fill(cmn::begin(f_obj_id_used), cmn::end(f_obj_id_used), false);
      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         f_obj_id_used[tracker_info.active_obj_ids[i] - 1] = true;
      }
      for (int32_t i = 0; i < (static_cast<int32_t>(tracker_info.variant.num_tracks) - tracker_info.num_active_objs); i++)
      {
         f_obj_id_used[tracker_info.inactive_obj_ids[i] - 1] = true;
      }
      for (int32_t i = 0; i < static_cast<int32_t>(tracker_info.variant.num_tracks); i++)
      {
         f_init_ok = f_init_ok && f_obj_id_used[i];
         f_init_ok = f_init_ok && ((i + 1) == object_tracks[i].id);
      }

      // Assert cluster ID:s are correct
      bool f_cluster_id_used[NUMBER_OF_CLUSTERS];
      std::fill(cmn::begin(f_cluster_id_used), cmn::end(f_cluster_id_used), false);
      for (int16_t i = 0; i < tracker_info.num_active_clusters; i++)
      {
         f_cluster_id_used[tracker_info.active_cluster_ids[i] - 1] = true;
      }
      for (int16_t i = 0; i < (static_cast<int16_t>(tracker_info.variant.num_clusters) - tracker_info.num_active_clusters); i++)
      {
         f_cluster_id_used[tracker_info.inactive_cluster_ids[i] - 1] = true;
      }
      for (int16_t i = 0; i < static_cast<int16_t>(tracker_info.variant.num_clusters); i++)
      {
         f_init_ok = f_init_ok && f_cluster_id_used[i];
         f_init_ok = f_init_ok && ((i + 1) == clusters[i].id);
      }

      // Assert reduced ID:s are correct
      bool f_reduced_id_used[NUMBER_OF_REDUCED_OBJECT_TRACKS];
      std::fill(cmn::begin(f_reduced_id_used), cmn::end(f_reduced_id_used), false);
      for (int32_t i = 0; i < tracker_info.reduced_num_active_objs; i++)
      {
         f_reduced_id_used[tracker_info.reduced_active_obj_ids[i] - 1] = true;
         f_init_ok = f_init_ok && (F360_OBJECT_STATUS_INVALID != object_tracks[tracker_info.reduced_obj_ids[i] - 1].reduced_status);
      }
      for (int32_t i = 0; i < (static_cast<int32_t>(tracker_info.variant.num_reduced_tracks) - tracker_info.reduced_num_active_objs); i++)
      {
         f_reduced_id_used[tracker_info.reduced_inactive_obj_ids[i] - 1] = true;
      }
      for (int32_t i = 0; i < static_cast<int32_t>(tracker_info.variant.num_reduced_tracks); i++)
      {
         f_init_ok = f_init_ok && f_reduced_id_used[i];
      }

      return f_init_ok;
   }

   void F360_Tracker::Get_Version(uint8_t* const major, uint8_t* const minor, uint8_t* const patch, uint64_t* const build_version)
   {
      *major = static_cast<uint8_t>(Tracker_Version_Major);
      *minor = static_cast<uint8_t>(Tracker_Version_Minor);
      *patch = static_cast<uint8_t>(Tracker_Version_Patch);
      *build_version = static_cast<uint64_t>(F360_TRACKER_VERSION_BUILD_ID);
   }


   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Execute()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Core_Info_T* const core_info,
   * const F360_Host_T &host,
   * const ocg::OCG_Outputs_T* occupancy_grid,
   * rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
   * const F360_Radar_Sensor_T const (&sensors)[MAX_NUMBER_OF_SENSORS])
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function executes the 360 tracker and calls the required functions
   * for the tracker code.
   *
   * PRECONDITIONS:
   *  To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Execute(
      const F360_Core_Info_T& core_info,
      const F360_Host_T &host,
      const ocg::OCG_Outputs_T* const p_occupancy_grid,
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS]) const
   {

      const float32_t start_time = get_wall_time();
      tracker_info.time_us = core_info.time_us;
      tracker_info.elapsed_time_s = core_info.elapsed_time_s;
      tracker_info.cnt_loops = core_info.cnt_loops;

      Inputs_Preprocessing(core_info, host, globals.max_otg_speed, host_props, timing_info);

      Update_Relative_Timestamps(core_info.elapsed_time_s, tracker_info, object_tracks, clusters, det_hist);

      Time_Update_Tracks(calibrations, core_info.elapsed_time_s, host, sensors, host_props, globals, object_tracks, tracker_info, timing_info);

      static_environment.Run_Longi_Stat_Curves(tracker_info, calibrations, host, object_tracks, timing_info);

      const Occlusion_T occlusion(calibrations.k_occlusion_enabled, calibrations, tracker_info, sensors, timing_info, object_tracks);

      Internal_Preprocessing(host, raw_detect_list, sensors, calibrations, static_environment.Get_Polynomials(), trailer_detector_core.Get_Trailer_Detector_Output(), host_props, globals, det_hist, object_tracks, detections, sensor_props, clusters, tracker_info, timing_info);

      trailer_detector_core.Run_Trailer_Detector(host, raw_detect_list, detections, sensors, tracker_info.elapsed_time_s, timing_info);

      Pre_Association_Track_Management(tracker_info, object_tracks, raw_detect_list, sensors, host, calibrations, static_environment.Get_Polynomials(), detections, timing_info);

      Detection_To_Track_Association(host, sensors, tracker_info, raw_detect_list, calibrations, static_environment.Get_Polynomials(), detections, object_tracks, timing_info);

      Clustering(calibrations, sensors, host, tracker_info, raw_detect_list, detections, clusters, timing_info);

      Cluster_Grouping(globals, calibrations, sensors, raw_detect_list, tracker_info, detections, det_hist, clusters, timing_info);

      Track_Grouping(calibrations, static_environment.Get_Polynomials(), host, sensors, raw_detect_list, globals, object_tracks, detections, tracker_info, timing_info);

      Measurement_Update_Tracks(calibrations, host, raw_detect_list, sensors, globals, tracker_info, detections, object_tracks, timing_info);

      Initialize_Tracks(host, host_props, raw_detect_list, det_hist, globals, calibrations, static_environment.Get_Polynomials(), occlusion, sensors, clusters, object_tracks, tracker_info, detections, timing_info);

      Post_Update_Track_Adjustments(calibrations, globals, raw_detect_list, host, sensors, tracker_info, detections, object_tracks, timing_info);

      Track_Classification(host, tracker_info, p_occupancy_grid, globals, raw_detect_list, sensors, calibrations, occlusion, timing_info, object_tracks);

      Track_Validity(host, tracker_info, calibrations, raw_detect_list, sensors, sensor_props, static_environment.Get_Polynomials(), object_tracks, timing_info);

      Track_Downselection(host, calibrations, static_environment.Get_Polynomials(), object_tracks, tracker_info, timing_info);

      static_environment.Run_Concrete_Wall_Detector(detections, raw_detect_list, sensors, sensor_props, calibrations, object_tracks, host, timing_info);

      E2E_Protection(timing_info);

      (void)Sanity_Check(raw_detect_list, detections, det_hist, clusters, object_tracks, tracker_info, timing_info);

      Sensor_Postprocessing(tracker_info, detections, raw_detect_list, sensors, det_hist, clusters, timing_info);

      timing_info.core_tracker = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Log_Tracker_Info()
   \*===========================================================================*/
   void F360_Tracker::Log_Tracker_Info(Tracker_Info_Log_T* const log) const
   {
      log->sw_version_buildID = static_cast<uint64_t>(F360_TRACKER_VERSION_BUILD_ID);
      log->sw_version_major = static_cast<uint8_t>(Tracker_Version_Major);
      log->sw_version_minor = static_cast<uint8_t>(Tracker_Version_Minor);
      log->sw_version_patch = static_cast<uint8_t>(Tracker_Version_Patch);

      log->tracker_variant_type = static_cast<uint8_t>(tracker_info.variant.type);

      for (unsigned int i = 0U; i < tracker_info.variant.num_tracks; i++)
      {
         log->active_obj_ids[i] = static_cast<uint16_t>(tracker_info.active_obj_ids[i]);
         log->inactive_obj_ids[i] = static_cast<uint16_t>(tracker_info.inactive_obj_ids[i]);
      }
      for (unsigned int i = 0U; i < tracker_info.variant.num_reduced_tracks; i++)
      {
         log->reduced_active_obj_ids[i] = static_cast<uint16_t>(tracker_info.reduced_active_obj_ids[i]);
         log->reduced_inactive_obj_ids[i] = static_cast<uint16_t>(tracker_info.reduced_inactive_obj_ids[i]);
         log->reduced_obj_ids[i] = static_cast<uint16_t>(tracker_info.reduced_obj_ids[i]);
      }
      log->timestamp_us = tracker_info.time_us;
      log->object_list_timestamp = tracker_info.object_list_timestamp;
      log->elapsed_time_s = tracker_info.elapsed_time_s;
      log->tracker_index = tracker_info.cnt_loops;

      log->num_active_objs = static_cast<uint16_t>(tracker_info.num_active_objs);
      log->reduced_num_active_objs = static_cast<uint16_t>(tracker_info.reduced_num_active_objs);
      log->num_unique_objs = tracker_info.num_unique_objs;

      log->num_active_clusters = static_cast<uint16_t>(tracker_info.num_active_clusters);
      log->number_of_historic_detections = static_cast<uint16_t>(det_hist.n_occupied);
   }

   /*===========================================================================*\
    * FUNCTION: F360_Tracker::Log_F360_Host_Props()
   \*===========================================================================*/
   void F360_Tracker::Log_F360_Host_Props(F360_Host_Props_Log_T& log) const
   {
      log.position_x = host_props.position.x;
      log.position_y = host_props.position.y;
      log.heading    = host_props.heading_angle;
      log.delta_pointing   = host_props.delta_pointing;
      log.delta_position_x = host_props.delta_position.x;
      log.delta_position_y = host_props.delta_position.y;
      (void)memcpy(log.vel_cov_scm, host_props.vel_cov_scm,sizeof(host_props.vel_cov_scm)); 
      (void)memcpy(log.vel_cov,     host_props.vel_cov,    sizeof(host_props.vel_cov));
      (void)memcpy(log.position_inc_cov_scm, host_props.position_inc_cov_scm, sizeof(host_props.position_inc_cov_scm));
      (void)memcpy(log.position_inc_cov,     host_props.position_inc_cov,     sizeof(host_props.position_inc_cov));
      log.std_speed_scm    = host_props.std_speed_scm;
      log.std_yaw_rate_scm = host_props.std_yaw_rate_scm;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Log_Timing_Info()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * Timing_Info_Log_T * const log - Pointer to timing info log
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
   * This function fills the timing info log.
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Log_Timing_Info(Timing_Info_Log_T * const log) const
   {
      log->tracker_index = tracker_info.cnt_loops;
      log->timestamp = static_cast<float>(tracker_info.time_us) * 1e-6F;
      log->prev_timestamp = log->timestamp - tracker_info.elapsed_time_s;

      log->overall_execution_times.time_taken_core_and_udp_packing = 0.0F;
      log->overall_execution_times.time_taken_full_tracker_DLL_func = 0.0F;
      log->overall_execution_times.time_taken_core_tracker = timing_info.core_tracker;
      log->overall_execution_times.inputs_preprocessing = timing_info.inputs_preprocessing;
      log->overall_execution_times.internal_preprocessing = timing_info.internal_preprocessing;
      log->overall_execution_times.time_update_tracks = timing_info.time_update_tracks;
      log->overall_execution_times.trailer_detector = timing_info.trailer_detector;
      log->overall_execution_times.pre_association_track_management = timing_info.pre_association_track_management;
      log->overall_execution_times.detection_to_track_association = timing_info.detection_to_track_association;
      log->overall_execution_times.clustering = timing_info.clustering;
      log->overall_execution_times.cluster_grouping = timing_info.cluster_grouping;
      log->overall_execution_times.track_grouping = timing_info.track_grouping;
      log->overall_execution_times.measurement_update_tracks = timing_info.measurement_update_tracks;
      log->overall_execution_times.initialize_tracks = timing_info.initialize_tracks;
      log->overall_execution_times.post_update_track_adjustments = timing_info.post_update_track_adjustments;
      log->overall_execution_times.track_classification = timing_info.track_classification;
      log->overall_execution_times.track_validity = timing_info.track_validity;
      log->overall_execution_times.track_downselection = timing_info.track_downselection;
      log->overall_execution_times.e2e_protection = timing_info.e2e_protection;
      log->overall_execution_times.occlusion = timing_info.occlusion;
      log->overall_execution_times.lsc_module = timing_info.lsc_module;
      log->overall_execution_times.concrete_wall_detector = timing_info.concrete_wall_detector;
      log->overall_execution_times.sensor_postprocessing = timing_info.sensor_postprocessing;

      log->itemized_execution_times.host_state_update = timing_info.host_state_update;
      log->itemized_execution_times.calc_obj_mov_stat_thresh = timing_info.calc_obj_mov_stat_thresh;
      log->itemized_execution_times.update_sensor_valid_info = timing_info.update_sensor_valid_info;
      log->itemized_execution_times.configure_rdot_interval_compability = timing_info.configure_rdot_interval_compability;
      log->itemized_execution_times.sensor_motion = timing_info.sensor_motion;
      log->itemized_execution_times.update_det_hist = timing_info.update_det_hist;
      log->itemized_execution_times.ego_motion_compensate_dets = timing_info.ego_motion_compensate_dets;
      log->itemized_execution_times.detect_wheelspin = timing_info.detect_wheelspin;
      log->itemized_execution_times.check_az_el_conf = timing_info.check_az_el_conf;
      log->itemized_execution_times.mark_out_det_pairs = timing_info.mark_out_det_pairs;
      log->itemized_execution_times.sensor_capability_module = timing_info.sensor_capability_module;
      log->itemized_execution_times.double_bounce_detection_countermeasure = timing_info.double_bounce_detection_countermeasure;

      log->itemized_execution_times.time_update_obj_trks_cca = timing_info.time_update_obj_trks_cca;
      log->itemized_execution_times.time_update_obj_trks_ctca = timing_info.time_update_obj_trks_ctca;

      log->itemized_execution_times.split_tracks = timing_info.split_tracks;
      log->itemized_execution_times.kill_coasted_tracks = timing_info.kill_coasted_tracks;

      log->itemized_execution_times.assoc_dets_with_obj_trks = timing_info.assoc_dets_with_obj_trks;
      log->itemized_execution_times.assoc_countermeasure = timing_info.assoc_countermeasure;
      log->itemized_execution_times.store_det_data = timing_info.store_det_data;
      log->itemized_execution_times.det_downselect = timing_info.det_downselect;
      log->itemized_execution_times.cluster_moving_detections = timing_info.cluster_moving_detections;
      log->itemized_execution_times.cluster_leftover_detections = timing_info.cluster_leftover_detections;
      log->itemized_execution_times.initialize_clusters = timing_info.m_initialize_clusters;
      log->itemized_execution_times.assoc_unconf_obj_trks = timing_info.assoc_unconf_obj_trks;

      log->itemized_execution_times.pseudo_estimations = timing_info.pseudo_estimations;

      log->itemized_execution_times.obj_trk_status_bookkeeping = timing_info.obj_trk_status_bookkeeping;
      log->itemized_execution_times.adjust_fltr_type_dependent_params = timing_info.adjust_fltr_type_dependent_params;
      log->itemized_execution_times.obj_trk_properties = timing_info.obj_trk_properties;
      log->itemized_execution_times.cancel_new_updated_trk_overlapping_confirmed_trks = timing_info.cancel_new_updated_trk_overlapping_confirmed_trks;
      log->itemized_execution_times.adjust_overlapping_confirmed_trks = timing_info.adjust_overlapping_confirmed_trks;
      log->itemized_execution_times.det_mean_var = timing_info.det_mean_var;
      log->itemized_execution_times.object_underdrivability_classification = timing_info.object_underdrivability_classification;
      log->itemized_execution_times.assign_underdrivability_status_to_tracks_ocg = timing_info.assign_underdrivability_status_to_tracks_ocg;
      log->itemized_execution_times.determine_underdrivable_for_movable = timing_info.determine_underdrivability_for_movable;

      log->itemized_execution_times.mark_object_tracks_next_to_sensors = timing_info.mark_object_tracks_next_to_sensors;

      log->itemized_execution_times.sanity_check = timing_info.sanity_check;

      log->itemized_execution_times.reserve1 = 0.0F;
      log->itemized_execution_times.reserve2 = 0.0F;
      log->itemized_execution_times.reserve3 = 0.0F;
      log->itemized_execution_times.reserve4 = 0.0F;
      log->itemized_execution_times.reserve5 = 0.0F;

      log->itemized_execution_times.msmt_update_obj_trks_cca_non_moveable = timing_info.msmt_update_obj_trks_cca_non_moveable;
      log->itemized_execution_times.msmt_update_obj_trks_cca_moveable = timing_info.msmt_update_obj_trks_cca_moveable;
      log->itemized_execution_times.msmt_update_obj_trks_cca = timing_info.msmt_update_obj_trks_cca;
      log->itemized_execution_times.msmt_update_obj_trks_ctca = timing_info.msmt_update_obj_trks_ctca;

      log->itemized_execution_times.assoc_unconf_obj_trks_sub_time_1 = timing_info.assoc_unconf_obj_trks_sub_time_1;
      log->itemized_execution_times.assoc_unconf_obj_trks_sub_time_2 = timing_info.assoc_unconf_obj_trks_sub_time_2;
      log->itemized_execution_times.assoc_unconf_obj_trks_sub_time_3 = timing_info.assoc_unconf_obj_trks_sub_time_3;
      log->itemized_execution_times.assoc_unconf_obj_trks_sub_time_4 = timing_info.assoc_unconf_obj_trks_sub_time_4;
      log->itemized_execution_times.assoc_unconf_obj_trks_sub_time_5 = timing_info.assoc_unconf_obj_trks_sub_time_5;
      log->itemized_execution_times.assoc_unconf_obj_trks_sub_time_6 = timing_info.assoc_unconf_obj_trks_sub_time_6;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Log_Static_Env_Polys()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Static_Env_Poly_Log_T * const log
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
   * This function fills static environment polynomial logging struct
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker. 
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Log_Static_Env_Polys(F360_Static_Env_Poly_Log_T* const log) const
   {
      log->tracker_index = tracker_info.cnt_loops;
      log->timestamp_us = tracker_info.time_us;

      const Static_Env_Poly_T (&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS] = static_environment.Get_Polynomials();

      for (unsigned int i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
      {
         log->static_env_polys[i].age = static_env_polys[i].age;
         log->static_env_polys[i].confidence = static_env_polys[i].confidence;
         log->static_env_polys[i].p0 = static_env_polys[i].p0;
         log->static_env_polys[i].p1 = static_env_polys[i].p1;
         log->static_env_polys[i].p2 = static_env_polys[i].p2;
         log->static_env_polys[i].upper_limit = static_env_polys[i].upper_limit;
         log->static_env_polys[i].lower_limit = static_env_polys[i].lower_limit;
         log->static_env_polys[i].status = static_cast<uint8_t>(static_env_polys[i].status);
         log->static_env_polys[i].poly_type = static_cast<uint8_t>(static_env_polys[i].poly_type);
      }
   }

#ifdef _DEBUG   
   bool F360_Tracker::Open_Debug_Info(const char* filename)
   {
      return f360_variant_A::open_xtrk_log(filename);
   }

   void F360_Tracker::Close_Debug_Info(void)
   {
      f360_variant_A::close_xtrk_log();
   }

   void F360_Tracker::Write_Debug_Info(
      const F360_Host_Calib_T* const host_calib,
      const F360_Host_Raw_T* const host_raw,
      const F360_Host_T* const host,
      const rspp_variant_A::RSPP_Detection_List_T* const raw_detect_list,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const ocg::OCG_Outputs_T* occupancy_grid)
   {
      write_xtrk_log(
         host_calib,
         host_raw,
         host,
         raw_detect_list,
         sensors,
         sensor_props,
         detections,
         det_hist,
         clusters,
         object_tracks,
         tracker_info,
         host_props,
         static_environment.Get_Polynomials(),
         trailer_detector_core.Get_Trailer_Detector_Output(),
         occupancy_grid);
   };
#endif

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Log_Trailer_Detector()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * Trailer_Detector_Log_T* const log
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
   * This function fills trailer detector logging struct
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Log_Trailer_Detector(Trailer_Detector_Log_T* const log) const
   {
      const Trailer_Detector_Flt_Fus_Output trailer_detector_output = trailer_detector_core.Get_Trailer_Detector_Output();
      log->trailer_length = trailer_detector_output.trailer_length;
      log->trailer_width = trailer_detector_output.trailer_width;
      log->trailer_angle =  trailer_detector_output.trailer_angle;
      log->trailer_angle_rate = trailer_detector_output.trailer_angle_rate;
      log->radar_detection_timer = trailer_detector_output.radar_detection_timer;
      log->stationary_timer = trailer_detector_output.stationary_timer;
      log->trailer_detection_status = static_cast<uint8_t>(trailer_detector_output.trailer_detection_status);
      log->trailer_presence = static_cast<uint8_t>(trailer_detector_output.trailer_presence);
      log->trailer_presence_conf = static_cast<uint8_t>(trailer_detector_output.trailer_presence_conf);
      log->trailer_width_conf = static_cast<uint8_t>(trailer_detector_output.trailer_width_conf);
      log->trailer_length_conf = static_cast<uint8_t>(trailer_detector_output.trailer_length_conf);
      log->trailer_angle_conf = static_cast<uint8_t>(trailer_detector_output.trailer_angle_conf);
      log->trailer_angle_rate_conf = static_cast<uint8_t>(trailer_detector_output.trailer_angle_rate_conf);
      log->trailer_HV_gap = trailer_detector_output.trailer_HV_gap;

   }
   
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Calibrations()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes calibration variables
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Calibrations()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
  
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Det_Prop()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes detection property Structure
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Det_Prop()
   {
      // Cannot use memset on struct due to class members
      Clear_Detections_Props(detections);
   }

   void F360_Tracker::Initialize_Det_Hist()
   {
      for (uint32_t i = 0U; i < tracker_info.variant.num_hist_dets; i++)
      {
         det_hist.det_data[i].vcs_position.x = 0.0F;
         det_hist.det_data[i].vcs_position.y = 0.0F;
         det_hist.det_data[i].rdot = 0.0F;
         det_hist.det_data[i].rdot_comp = 0.0F;
         det_hist.det_data[i].vcs_az = 0.0F;
         det_hist.det_data[i].time_since_meas = -1.0F;
         det_hist.det_data[i].f_FOV_edge = false;
         det_hist.det_data[i].f_selected = false;
         det_hist.det_data[i].f_azimuth_error_stat_mov = false;
         det_hist.det_data[i].f_potential_angle_jump = false;
         det_hist.det_data[i].look_type = F360_DET_LOOK_TYPE_INVALID;
         det_hist.det_data[i].wheel_spin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
         det_hist.det_data[i].motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_INVALID;
         det_hist.det_data[i].v_wrapping = 0.0F;
         det_hist.det_data[i].r_wrapping = 0.0F;
         det_hist.det_data[i].cluster_idx = 0;

         det_hist.f_idx_occupied[i] = false;
      }

      det_hist.n_occupied = 0;
      det_hist.max_occupation = 0;
   }

   void F360_Tracker::Initialize_Host_Props()
   {
      // Cannot use memset on struct due to class members
      host_props.position.Set_Position(0.0F, 0.0F);
      host_props.delta_position.Set_Position(0.0F, 0.0F);
      (void)memset(host_props.vel_cov_scm, 0, sizeof(host_props.vel_cov_scm));
      (void)memset(host_props.vel_cov, 0, sizeof(host_props.vel_cov));
      (void)memset(host_props.position_inc_cov_scm, 0, sizeof(host_props.position_inc_cov_scm));
      (void)memset(host_props.position_inc_cov, 0, sizeof(host_props.position_inc_cov));
      host_props.std_speed_scm = 0.0F;
      host_props.std_yaw_rate_scm = 0.0F;
      host_props.heading_angle = 0.0F;
      host_props.cos_heading = 0.0F;
      host_props.sin_heading = 0.0F;
      host_props.delta_pointing = 0.0F;
      host_props.cos_delta_pointing = 0.0F;
      host_props.sin_delta_pointing = 0.0F;
   }

   void F360_Tracker::Initialize_Sensor_Props()
   {
      (void)memset(&sensor_props[0], 0, sizeof(sensor_props));
      for (uint32_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         sensor_props[i].time_since_measurement_s = -1.0F;
      }
   }
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Timing_Info()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes Timing_Info Structure
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Timing_Info()
   {
      // initialization of detection property Structure
      (void)memset(&timing_info, 0, sizeof(timing_info));
   }


   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Clusters()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes cluster Structure
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Clusters()
   {
      // Cannot use memset due to class members
      for (uint16_t index = 0U; index < tracker_info.variant.num_clusters; index++)
      {
         Clear_Cluster(clusters[index]);
         clusters[index].id = static_cast<int16_t>(index) + 1;
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Obj_Trck()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes object track Structure
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Obj_Trck()
   {
      // Cannot use memset due to class members
      for (uint32_t index = 0U; index < tracker_info.variant.num_tracks; index++)
      {
         Clear_Object_Track(object_tracks[index]);
         object_tracks[index].id = static_cast<int32_t>(index) + 1;
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Trkr_Info()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes tracker info Structure
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Trkr_Info()
   {
      // initialization of tracker info Structure
            // Clear everything but the variant info
      const F360_Variant_T temp = tracker_info.variant;
      (void)memset(&tracker_info, 0, sizeof(F360_Tracker_Info_T));
      tracker_info.variant = temp;

      for (uint32_t index = 0U; index < tracker_info.variant.num_tracks; index++)
      {
         const int32_t id = static_cast<int32_t>(index) + static_cast<int32_t>(1U);
         tracker_info.inactive_obj_ids[index] = id;
      }
      for (uint32_t index = 0U; index < tracker_info.variant.num_reduced_tracks; index++)
      {
         const int32_t id = static_cast<int32_t>(index) + static_cast<int32_t>(1U);
         tracker_info.reduced_inactive_obj_ids[index] = id;
      }
      for (uint16_t index = 0U; index < tracker_info.variant.num_clusters; index++)
      {
         const int16_t id = static_cast<int16_t>(index) + static_cast<int16_t>(1U);
         tracker_info.inactive_cluster_ids[index] = id;
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Globals()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes global variables
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Globals()
   {
      // Initialization of global variables
      globals.obj_mov_stat_spd_thresh = 1.0F;

      // verify if this is proper place
      globals.rear_cornering_compliance = 0.0053F;
      globals.oncoming_speed_thresh = -1.0F;
      globals.obj_vehicular_spd_thresh = 3.0F;
      globals.default_half_length = 3.0F;
      globals.default_half_width = 1.0F;
      globals.default_height = 1.0F;
      globals.default_length = 6.0F;
      globals.default_width = 2.0F;
      globals.max_otg_speed = 70.0F;

   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Static_Env_Polys()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes all Static Environment Polynomials
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Static_Env_Polys()
   {
      static_environment.Initialize_Static_Env();
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Trailer_Detector()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes Trailer_Detector_Core
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Trailer_Detector()
   {
      trailer_detector_core.Initialize();
   }
   
   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Variant()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * None
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
   * This function initializes variant variables to their defaults
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Variant()
   {
      tracker_info.variant.num_tracks = NUMBER_OF_OBJECT_TRACKS;
      tracker_info.variant.num_reduced_tracks = NUMBER_OF_REDUCED_OBJECT_TRACKS;
      tracker_info.variant.num_posn_clusters = MAX_TRACKER_POSN_CLUSTERS;
      tracker_info.variant.num_dets_in_track = MAX_DETS_IN_OBJ_TRK;
      tracker_info.variant.num_hist_dets_in_track = MAX_HIST_DETS_IN_OBJ_TRACK;
      tracker_info.variant.num_clusters = NUMBER_OF_CLUSTERS;
      tracker_info.variant.num_hist_dets = MAX_NUMBER_OF_HISTORIC_DETECTIONS;
      tracker_info.variant.type = VARIANT_TYPE;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Tracker::Initialize_Variant(F360_Variant_T* input_variant)
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * F360_Variant_T* input_variant
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
   * This function initializes variant variables to specific values
   *
   * PRECONDITIONS:
   * To be called by an object/reference of F360_Tracker.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void F360_Tracker::Initialize_Variant(const F360_Variant_T& input_variant)
   {
      tracker_info.variant.num_tracks = input_variant.num_tracks;
      tracker_info.variant.num_reduced_tracks = input_variant.num_reduced_tracks;
      tracker_info.variant.num_posn_clusters = input_variant.num_posn_clusters;
      tracker_info.variant.num_dets_in_track = input_variant.num_dets_in_track;
      tracker_info.variant.num_hist_dets_in_track = input_variant.num_hist_dets_in_track;
      tracker_info.variant.num_clusters = input_variant.num_clusters;
      tracker_info.variant.num_hist_dets = input_variant.num_hist_dets;
      tracker_info.variant.type = input_variant.type;
   }

    void F360_Tracker::Fill_ROT_Object_Output(
       const rspp_variant_A::RSPP_Detection_List_T& det_list,
       const F360_Host_T& host_info,
       const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
       const Functional_Safety_Faults_Log_T& functional_safety_faults_log,
       ROT_Object_List_Info_T& rot_object_list_info)
    {
       (void)memset(&rot_object_list_info, 0, sizeof(ROT_Object_List_Info_T));
       rot_object_list_info.rot_object_list_timestamp = tracker_info.object_list_timestamp;
       rot_object_list_info.tracker_index = tracker_info.cnt_loops;
       rot_object_list_info.number_of_objects = static_cast<uint16_t>(tracker_info.reduced_num_active_objs);
       rot_object_list_info.tracker_start_timestamp = tracker_info.time_us;
       rot_object_list_info.tracker_elapsed_time = tracker_info.elapsed_time_s;
       Set_ROT_Object_Output(host_info, sensors, det_list, object_tracks, rot_object_list_info.rot_object_list);
       Set_ROT_Scl_Faults(functional_safety_faults_log, rot_object_list_info.all_scl_faults);

    }

   void F360_Tracker::Log_Detections(F360_Detection_Log_Output_T* const log, const rspp_variant_A::RSPP_Detection_List_T& det_list)
   {
      (void)memset(log, 0, sizeof(F360_Detection_Log_Output_T));
      log->f360header.version = f360_header_version;
      log->f360header.num_elements = det_list.number_of_valid_detections;
      log->f360header.data_timstamp_us = tracker_info.time_us;
      Populate_Detections_Log(log, det_list, detections);
   }

   void F360_Tracker::Log_Objects(F360_Object_Log_Output_T& log) const
   {
      (void)memset(&log, 0, sizeof(F360_Object_Log_Output_T));
      log.f360header.version = f360_header_version;
      log.f360header.num_elements = static_cast<uint32_t>(tracker_info.num_active_objs);
      log.f360header.data_timstamp_us = tracker_info.time_us;
      Populate_Objects_Log_Data(log.object, object_tracks,
         tracker_info.num_active_objs, tracker_info.active_obj_ids);
   }

   void F360_Tracker::Log_Internal_Clusters(F360_Internal_Cluster_Log_Output_T& log)
   {
      log.f360header.version = f360_header_version;
      log.f360header.num_elements = static_cast<uint32_t>(tracker_info.num_active_clusters);
      log.f360header.data_timstamp_us = tracker_info.time_us;
      Populate_Internal_Clusters_Log_Data(log.cluster, clusters,
         tracker_info.num_active_clusters, tracker_info.active_cluster_ids);
   }

   void F360_Tracker::Log_Internal_Objects(F360_Internal_Object_Log_Output_T& log)
   {
      log.f360header.version = f360_header_version;
      log.f360header.num_elements = static_cast<uint32_t>(tracker_info.num_active_objs);
      log.f360header.data_timstamp_us = tracker_info.time_us;
      Populate_Internal_Objects_Log_Data(log.object, object_tracks,
         tracker_info.num_active_objs, tracker_info.active_obj_ids);
   }

   void F360_Tracker::Log_Internal_Detection_History(F360_Internal_Detection_History_Log_Output_T& log)
   {
      log.f360header.version = f360_header_version;
      log.f360header.data_timstamp_us = tracker_info.time_us;
      log.f360header.num_elements = static_cast<uint32_t>(det_hist.n_occupied);
      Populate_Internal_Detection_History_Log_Data(log.det_hist, det_hist);
   }

   void F360_Tracker::Log_Internal_CWD(F360_Internal_CWD_Log_Output_T& log)
   {
      log.f360header.version = f360_header_version;
      log.f360header.data_timstamp_us = tracker_info.time_us;
      log.f360header.num_elements = static_environment.Get_CWD().Get_CWD_Sensors_Count();
      Populate_Internal_CWD_Log_Data(log.cwd, static_environment);
   }

   void F360_Tracker::Log_Internal_Trailer_Detector(F360_Internal_Trailer_Detector_T& log)
   {
      Populate_Internal_Trailer_Detector_Log_Data(log, trailer_detector_core);
   }

   void F360_Tracker::Log_Internal_Reflection_Buffer(F360_Internal_Reflection_Buffer_Log_Output_T& log)
   {
      log.f360header.version = f360_header_version;
      log.f360header.num_elements = static_cast<uint32_t>(MAX_NUMBER_OF_SENSORS);
      log.f360header.data_timstamp_us = tracker_info.time_us;
      Populate_Internal_Reflection_Buffer_Log_Data(log.reflection_buffer, sensor_props);
   }

   void F360_Tracker::Log_Host_Calibs(Host_Calibs_Log_T& log, const F360_Host_Calib_T& host_calib) const
   {
      //VSE vehicle params
      log.dist_rear_axle_to_vcs_m = host_calib.dist_rear_axle_to_vcs_m;
      log.rear_cornering_compliance = host_calib.rear_cornering_compliance;
      log.steer_gear_ratio = host_calib.steer_gear_ratio;
      log.wheelbase_m = host_calib.wheelbase_m;
      log.understeer_coefficient = host_calib.understeer_coefficient;
      log.vehicle_width_m = host_calib.vehicle_width_m;
      log.vehicle_length_m = host_calib.vehicle_length_m;
      log.cog_x = host_calib.cog_x;
      log.cog_y = host_calib.cog_y;
      log.front_wheel_radius_m = host_calib.front_wheel_radius_m;
      log.front_track_width_m = host_calib.front_track_width_m;

      // VSE Tracker params
      log.raw_host_signal_latency = host_calib.raw_host_signal_latency_ms;

      // Vehicle configuration params 
      log.f_enable_internal_reflections_func = host_calib.f_enable_internal_reflections_func;
      log.f_enable_internal_reflections_func_trailer = host_calib.f_enable_internal_reflections_func_trailer;
   }

   void F360_Tracker::Log_Sensor_Calibs(F360_Sensor_Calib_Log_Output_T& log, const F360_Radar_Sensor_Tag(&sensor)[MAX_NUMBER_OF_SENSORS])
   {
      log.f360header.version = f360_header_version;
      log.f360header.data_timstamp_us = tracker_info.time_us;
      log.f360header.num_elements = 0U;

      for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         if (sensor[i].variable.is_valid)
         {
            const F360_Radar_Sensor_T* const sensors = &sensor[log.f360header.num_elements];
            F360_Sensor_Calib_Log_T* const sensor_log = &log.sensor[log.f360header.num_elements];
            log.f360header.num_elements++;

            sensor_log->id = static_cast<uint8_t>(sensors->constant.id);
            sensor_log->mounting_location = static_cast<int8_t>(sensors->constant.mounting_location);
            sensor_log->sensor_type = static_cast<int8_t>(sensors->constant.sensor_type);
            sensor_log->polarity = static_cast<int8_t>(sensors->constant.polarity);
            sensor_log->vcs_position_longitudinal = sensors->constant.mounting_position.vcs_position.longitudinal;
            sensor_log->vcs_position_lateral = sensors->constant.mounting_position.vcs_position.lateral;
            sensor_log->vcs_position_height = sensors->constant.mounting_position.vcs_position.height;
            sensor_log->vcs_boresight_azimuth_angle = sensors->constant.mounting_position.vcs_boresight_azimuth_angle;
            sensor_log->vcs_boresight_elevation_angle = sensors->constant.mounting_position.vcs_boresight_elevation_angle;

            for (int32_t j = 0; j < 4; j++)
            {
               sensor_log->range_limits[j] = sensors->constant.range_limits[j];
               sensor_log->fov_min_az_rad[j] = sensors->constant.fov_min_az_rad[j];
               sensor_log->fov_max_az_rad[j] = sensors->constant.fov_max_az_rad[j];
               sensor_log->fov_min_el_rad[j] = sensors->constant.fov_min_el_rad[j];
               sensor_log->fov_max_el_rad[j] = sensors->constant.fov_max_el_rad[j];
               sensor_log->min_aliaised_range_rate[j] = sensors->constant.min_aliaised_range_rate[j];
               sensor_log->v_wrapping[j] = sensors->constant.v_wrapping[j];
               sensor_log->r_wrapping[j] = sensors->constant.r_wrapping[j];
            }
         }
      }
   }

   void F360_Tracker::Log_Vehicle_Info(Vehicle_Info_Log_T& log,
      const F360_Host_T& host,
      const F360_Host_Raw_T& host_raw, 
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]) const
   {
      // Time
      log.vehicle_index = static_cast<int32_t>(host.vehicle_index);
      log.global_time_sync_s = host_raw.global_time_sync_s;
      log.timestamp_s = host_raw.timestamp_s;

      // Correction parameters
      for (uint8_t isens = 0U; isens < MAX_NUMBER_OF_SENSORS; isens++)
      {
         log.vacs_boresight_az_estimated[isens] = sensors[isens].variable.vacs_boresight_az_estimated;
         log.vacs_boresight_el_estimated[isens] = sensors[isens].variable.vacs_boresight_el_estimated;
      }
      log.speed_correction_factor = host.speed_correction_factor;

      // Host props
      log.world_x = host_props.position.x;
      log.world_y = host_props.position.y;
      log.heading = host_props.heading_angle;
      log.delta_pointing = host_props.delta_pointing;
      log.delta_position_x = host_props.delta_position.x;
      log.delta_position_y = host_props.delta_position.y;

      // Host
      log.speed = host.speed;
      log.acceleration = host.acceleration;
      log.curvature_rear = host.curvature_rear;
      log.vcs_speed = host.vcs_speed;
      log.vcs_lat_acceleration = host.vcs_lat_acceleration;
      log.vcs_long_acceleration = host.vcs_long_acceleration;
      log.vcs_sideslip = host.vcs_sideslip;
      log.rear_cornering_compliance = host.rear_cornering_compliance;
      log.filtered_yaw_rate_rad = host.yaw_rate_rad;

      // Host raw
      log.raw_speed = host_raw.raw_speed;
      log.steering_angle_rad = host_raw.steering_wheel_angle_rad;
      log.raw_yaw_rate_rad = host_raw.raw_yaw_rate_rad;
      log.dist_rear_axle_to_vcs_m = host.dist_rear_axle_to_vcs_m;
      log.prndl = host_raw.prndl;
      log.f_reverse_gear = host_raw.reverse_gear;
      log.f_trailer_presence_hardware = host.f_trailer_presence_hardware;
   }

   void F360_Tracker::Log_Sync_Info(Sync_Info_Log_T& log,
      const F360_Host_T& host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]) const
   {
      log.core.tracker_timestamp_us = tracker_info.time_us;
      log.core.elapsed_time_s = tracker_info.elapsed_time_s;
      log.core.tracker_index = tracker_info.cnt_loops;
      log.core.vehicle_index = host.vehicle_index;
      log.align.speed_correction_factor = host.speed_correction_factor;
      for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         log.core.sensor_timestamp_us[i] = sensors[i].variable.timestamp_us;
         log.core.sensor_look_index[i] = sensors[i].variable.look_index;
         log.align.vacs_boresight_az_estimated[i] = sensors[i].variable.vacs_boresight_az_estimated;
         log.align.vacs_boresight_el_estimated[i] = sensors[i].variable.vacs_boresight_el_estimated;
      }
   }
}
