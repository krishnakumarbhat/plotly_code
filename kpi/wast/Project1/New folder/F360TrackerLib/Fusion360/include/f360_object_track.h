/*===================================================================================*\
* FILE: f360_object_track.h
*====================================================================================
* Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains F360_Object_Track_T structure declaration
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/
#ifndef F360_OBJECT_TRACK_H
#define F360_OBJECT_TRACK_H

#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_velocity.h"
#include "f360_point.h"
#include "f360_accel.h"
#include "f360_object_status.h"
#include "f360_object_occlusion_status.h"
#include "f360_look_type.h"
#include "f360_track_init.h"
#include "f360_trk_fltr_type.h"
#include "f360_object_class.h"
#include "f360_nees_cfmi_vel_hyp_source.h"
#include "f360_conf.h"
#include "f360_math.h"
#include "f360_dead_zone_status.h"
#include "f360_angle.h"
#include "f360_bounding_box.h"
#include "f360_current_msmt_type.h"
#include "f360_reference_point.h"

#include "ocg_underdrivability_enum.h"

namespace f360_variant_A
{
   struct F360_Object_Track_T
   {
      // Longitudinal Static Curves (LSC)
      F360_Object_Track_T* lsc_next_in_cluster; // Pointer to the next VCS-longitudinal sorted object in a certain LSC cluster
      F360_Object_Track_T* lsc_prev_in_cluster; // Pointer to the previous VCS-longitudinal sorted object in a certain LSC cluster

      F360_Object_Track_T* p_higher_priority_track;      // pointer to the track with next greater priority [-]
      F360_Object_Track_T* p_lower_priority_track;       // pointer to the track with next lower priority [-]

      Point pseudo_vcs_position;  // position with pseudo-measurement for measurement update in VCS [m]
      float32_t speed;                   // object Over-The-Ground (OTG) [m/s]; estimation point at object rear center in CTCA model
      float32_t predicted_speed;         // predicted object Over-The-Ground (OTG) [m/s]
      float32_t hdg_ptng_disagmt;        // disagreement between heading angle and pointing angle [rad]
      float32_t curvature;               // object movement curvature [1/m]; estimation point at object rear center in CTCA model
      float32_t heading_rate;               // object heading rate [rad/s]; 
      float32_t tang_accel;              // object tangential acceleration [m/s^2]; estimation point at object rear center in CTCA model
      float32_t predicted_tang_accel;    // predicted object tangential acceleration [m/s^2]
      Point vcs_position;   // object position in VCS [m]
      Point predicted_vcs_position; // predicted object position in VCS [m]
      float32_t otg_height; // over the ground height mean of the object, accumulated over object's lifetime [m]
      F360_VCS_Velocity_T vcs_velocity;   // object velocity in VCS [m/s]; estimation point at object rear center in CTCA model
      F360_VCS_Velocity_T predicted_vcs_velocity; // predicted object velocity in VCS [m/s]
      F360_VCS_Accel_T vcs_accel;         // object acceleration in VCS [m/s^2]
      Angle vcs_heading;             // object velocity heading in VCS [rad]; estimation point at object rear center in CTCA model
      float32_t predicted_vcs_heading;   // predicted object velocity heading in VCS [rad]
      BoundingBox bbox;                   // bounding box - stores position of center, pointing angle, length and width
      float32_t predicted_vcs_pointing;  // predicted object pointing angle in VCS [rad]
      float32_t time_since_cluster_created;  // time since creation of a cluster, from which the track is derived (based on tracker execution time) [s]
      float32_t time_since_track_updated;    // time since latest detection associated to the object (based on tracker execution time) [s]
      float32_t time_since_downselected;    // time since downselected [s]
      float32_t time_since_split;            // time since object was involved in a split [s]
      float32_t errcov[STATE_DIMENSION][STATE_DIMENSION]; // error covariance matrix for current state representation [multiple]
      float32_t cca_pnt_filter_cov[2][2]; // covariance matrix used in object cca pointing filter
      uint32_t ndets;                 // number of detections associated to object [-]
      uint32_t detids[MAX_DETS_IN_OBJ_TRK]; // list of detections identifiers associated to the object [-]
      uint32_t num_rr_inlier_dets;              // number of associated detections which range-rate closely matches the predicted range-rate for the object [-]
      uint32_t num_dets_used_in_rr_msmt_update; // number of reduced detections which was used in the measurement rr update [-]
      
      // Underdrivability for moving objects
      float32_t  ud_mov_historic_ndets;  // total number of detections used to calculate historic height mean, associated to the object over time [-]
      uint32_t ud_mov_cnt_underdrivable;  // number of scans during which object historic height mean was above the threshold determining ud status [-]
      float32_t probability_underdrivable; // Probability (0-1) that the object is underdrivable [-]

      float32_t mirror_prob;            // Probability (0-1) that this track is a mirror of multi path detections (where mirror is guardrail or other object) [-]
      float32_t length_uncertainty;      // Uncertainty scalar for updating object size in length direction [m]
      float32_t width_uncertainty;       // Uncertainty scalar for updating object size in width direction [m]
      int32_t id;                    // object track identifier (id = index_in_array + 1; ) [-]
      uint32_t unique_id;             // unique object track identifier [-]
      int32_t reduced_id;            // down-selected tracked object identifier (0 = invalid object) [-]
      int32_t cntConsecutiveAmbiguous; // counter of consecutive tracked object update by ambiguous object motion status (incremented once per tracker cycle) [tracker execution cycles]
      int32_t cntConsecutiveMoving;    // counter of consecutive tracked object update by moving object motion status (incremented once per tracker cycle) [tracker execution cycles]
      int32_t cntConsecutiveStopped;   // counter of consecutive tracked object update by stopped object motion status (incremented once per tracker cycle) [tracker execution cycles]
      float32_t raw_confidence_level;    // instantaneous object confidence level (based on current measurement) [-], range: <0, 1>
      float32_t confidenceLevel;         // overall confidence that object state is valid [-], range: <0, 1>
      float32_t prev_avrg_conf_level;    // average confidence level calculated based on previous and current confidence  [-]
      float32_t time_since_stage_start;        // tracker time since last change in track status [s]
      int32_t num_types_of_dets[2];  // number of detections associated to the track (split by detection motion status: 0 - moving, 1 - other) [-]
      float32_t meascov[F360_PSEUDO_MEAS_DIM][F360_PSEUDO_MEAS_DIM]; // track measurement covariance for x, y pseudo position [m^2]
      int32_t cnt_error_in_predicted_speed; // counter of updates with  speed inaccuracy estimation detection counter [tracker execution cycles]
      float32_t long_buffer_zone_len1;    // length dimension added to len1 of the bounding box for detection association [m]
      float32_t long_buffer_zone_len2;    // length dimension added to the len2 of the bounding box for detection association [m]
      float32_t lat_buffer_zone_wid1;     // lateral dimension added to wid1 bounding box for detection association [m]
      float32_t lat_buffer_zone_wid2;     // lateral dimension added to wid2 bounding box for detection association [m]
      float32_t time_since_initialization;     // time since object initialization (based on tracker execution time) [s]
      float32_t time_since_vehicle_init; // time since f_vehicular_trk flag set (based on tracker execution time) [s]
      float32_t time_since_last_stop;    // time since last time the object comes to a stop from moving motion classification [s]
      uint32_t total_reduced_dets;    // number of down-selected detection associated to the track through its lifespan [-]
      float32_t filtered_dets;           // filtered over the time number of detection (can be fraction) [-]
      float32_t time_since_measurement;  // Time since object was updated [s]
      float32_t innovation_length;       // length innovation [m] [m]
      float32_t innovation_width;        // width innovation [m] [m]
      float32_t accuracy_length;         // accuracy of filtered length [m]
      float32_t accuracy_width;          // accuracy of filtered width [m]
      float32_t priority;                // priority used for object vs clusters prioritization during new object initialization when object list is saturated, range <0, 1> (higher priority means that object is more important) [-]

      // existance probability filter
      float32_t exist_prob;              // object existence probability [-], range: <0, 1>
      float32_t p_track_state;           // probability of precise estimation of object state, range: <0, 1>
      float32_t p_det_sensor;            // probability that sensor is able to perform a valid measurement at a location in FoV [-]], range: <0, 1>
      float32_t p_measurement;           // probability of true positive measurement, range: <0, 1>
      float32_t p_birth;                 // (not assigned) always 0; probability of birth of the track, range: <0, 1>
      float32_t p_persist;               // probability that track is persistent [-], range: <0, 1>

      // Object class probability vectors
      float32_t probability_pedestrian;  // probability that object is a pedestrian [-]
      float32_t probability_car;         // probability that object is a car [-]
      float32_t probability_bicycle;     // probability that object is a bicycle [-]
      float32_t probability_motorcycle;   // probability that object is a motorcycle [-]
      float32_t probability_truck;       // probability that object is a track [-]
      float32_t probability_undet;       // probability that object is a grounded UFO [-]

      // Reserved for future use for 3D object detection reporting
      float32_t reserved_value_1;
      float32_t reserved_value_2;
      float32_t reserved_value_3;
      float32_t reserved_value_4;
      float32_t reserved_value_5;

      // To Select Detections To Mark As Inliers
      float32_t filtered_hist_assoc_det_rr_err_mean; // adjusted mean difference between speeds of a detection and and object it is assigned to, accumulated over object's lifetime.
      float32_t filtered_hist_assoc_det_rr_err_var; // adjusted variance difference between speeds of a detection and and object it is assigned to, accumulated over object's lifetime.
      float32_t filtered_hist_assoc_n_dets; // adjusted total number of detections associated to the object over time

      float32_t average_rcs; // low-pass filtered RCS value based on average rcs of associated detections [dB/m^2] 

      // Split logic signals
      float32_t orth_delta_filtered; // Low pass filtered orthogonal delta distance between detections on object (maximum orth distance between all detections) [m]
      float32_t orth_gap_filtered; // Low pass filtered orthogonal distance gap between detections on object (maximum orth gap between two detections). Note that gap <= delta based on its definition [m]
      Point prev_vcs_center_pos; // VCS position of object center position in previous tracker iteratation. Only used for CTCA objects. Transformed to current VCS in time update module [m]
      float32_t filtered_pos_diff_heading; // Low pass filtered VCS heading solely based on object centroid position delta between tracker iterations [rad]. Only used for CTCA objects. 

      CONF9_T conf_longitudinal_position; // Internal state confidence of vcs_position longitudinal used for determining overall confidence
      CONF9_T conf_lateral_position;      // Internal state confidence of vcs_position lateral used for determining overall confidence
      CONF9_T conf_longitudinal_velocity; // Internal state confidence of vcs_velocity longitudinal used for determining overall confidence
      CONF9_T conf_lateral_velocity;      // Internal state confidence of vcs_velocity lateral used for determining overall confidence
      CONF9_T conf_speed;                 // Internal state confidence of speed used for determining overall confidence
      CONF3_T conf_overall;               // Overall state confidence based on "smoothness" of all states

      F360_Object_Status_T status;              // object lifespan status [-]
      Object_Occlusion_Status occlusion_status; // occlusion status of various points on the object
      F360_Track_Init_T init_scheme;            // object initialization method [-]
      F360_NEES_CFMI_Vel_Hyp_Source_T init_vel_source; // NEES Cost-Function-Min-Initialization velocity source [-]
      F360_Object_Status_T reduced_status; // down-selected tracked object lifespan status [-]
      F360_Trk_Fltr_Type_T trk_fltr_type;  // track motion filter method [-]
      F360_Reference_Point_T reference_point; // Enumeration of which object point (corners, side midpoints or center) that is most likely seen from center of host.
      F360_Reference_Point_T min_projection_reference_point; // Enumeration of which object point (corners, side midpoints or center) that is most likely seen from center of host regardless of visiblity
      F360_Object_Class_T object_class; // object class [-]
      F360_Dead_Zone_Status_T dead_zone_status; // Enumeration identifying whether object is in left or right dead zone
      F360_Current_Msmt_Type_T current_msmt_type; // current measurement type, intermediate unification of interfaces, to be removed after DEX-1436 [-]
      
      // Underdrivibility class
      ocg::OCG_Underdrivable_Status_T underdrivable_status; // enum identifying whether host can pass under track

      bool f_crossing;            // not used - obsolete or not implemented [-]
      bool f_moving;              // flag indicating that object is moving [-]
      bool f_stopped;             // flag indicating that object has stopped [-]
      bool f_moveable;            // flag indicating that object is capable to move (e.g. object was moving but stopped) [-]
      bool f_oncoming;            // flag indicating that object is oncoming to the host [-]
      bool f_low_confidence_level;// flag indicating that object has low confidence level [-]
      bool f_vehicular_trk;       // flag indicating that object is a vehicle [-]
      bool f_veh_trk_near_stat_host; // flag indicating that host is stationary and moving track is in a short distance [-]
      bool f_valid_for_liberal_tracking;  // Process this track with more liberal conditions within association and earlier downselection
      bool f_fast_moving;         // flag indicating fast moving objects [-]
      bool f_need_to_hide_trk;    // (obsolete) always 0
      bool f_ghost_NU_2_C;        // flag indicating change of object status from NEW to COASTED [-]
      bool f_overlapping_with_object; // flag indicating that object bounding box overlaps with different bounding box [-]
      bool f_used_by_occlusion;   // Flag indicating that track was used by occlusion detection algorithm. [-]

      // existance probability filter
      bool f_track_born;          // (obsolete) always 0

      // Static Environment Polynomials
      bool f_behind_sep_ambiguous; // Flag indicating that the object is partially behind an SEP
      uint8_t behind_sep_id; // Id of highest prioritized SEP that this object is behind. Tracks that are "on" will never be flagged as "behind". Only CTCA and fast moving CCA trare flagged as behind whereas slow moving CCA tracks have behind_lsc_id = F360_INVALID_UNSIGNED_ID by default.
      uint8_t on_sep_id;  // Id of highest prioritized SEP which this object is "on".
      Point sep_intersection_point; // Point where a straight line from VCS origin to the object centroid intersects the closest SEP 
      
      uint8_t num_updates_since_init; // number of times an upbject has been KF measurement updated since it was first born
      uint8_t low_rcs_dets_cnt; // Counter for identifying suspicious low rcs tracks is close proximity to host that should not be downselected. Since the track will be initialied close to host we start collecting information already in the cluster stages. The counter can only be set while the cluster and subsequent merged clusters have only a single associated detection per tracker iteration.

      uint8_t padding[3];

      void Update_Bbox_Size(const float32_t &length, const float32_t &width); // Update bbox to be aligned with size related signals
      void Set_Bbox_Orientation(const Angle& new_orientation);    // Set new orientation of bbox
      void Update_Bbox_Center();    // Recalculate bbox center to be aligned with current state of track

   };

   using Object_Tracks_Array = F360_Object_Track_T[NUMBER_OF_OBJECT_TRACKS];

   /*                                                                            // Todo DFD -1555: Size asserts
   #ifdef __TASKING__
      // 64-bit
      static_assert((4 == sizeof(void*)) || (((
         + sizeof(F360_Object_Track_T::detids)
         + sizeof(F360_Object_Track_T::errcov)
         + sizeof(F360_Object_Track_T::meascov)
         + 484))
         == sizeof(F360_Object_Track_T)), "sizeof(F360_Object_Track_T) not as expected. Remember to align padding if needed");

      // 32-bit
      static_assert((8 == sizeof(void*)) || (((
         + sizeof(F360_Object_Track_T::detids)
         + sizeof(F360_Object_Track_T::errcov)
         + sizeof(F360_Object_Track_T::meascov)
         + 464)) 
         == sizeof(F360_Object_Track_T)), "sizeof(F360_Object_Track_T) not as expected. Remember to align padding if needed");
   #else
      // 64-bit
      static_assert((4 == sizeof(void*)) || (((
         + sizeof(F360_Object_Track_T::detids)
         + sizeof(F360_Object_Track_T::errcov)
         + sizeof(F360_Object_Track_T::meascov)
         + 516))
         == sizeof(F360_Object_Track_T)), "sizeof(F360_Object_Track_T) not as expected. Remember to align padding if needed");
       
      // 32-bit
      static_assert((8 == sizeof(void*)) || (((
         + sizeof(F360_Object_Track_T::detids)
         + sizeof(F360_Object_Track_T::errcov)
         + sizeof(F360_Object_Track_T::meascov)
         + 492)) 
         == sizeof(F360_Object_Track_T)), "sizeof(F360_Object_Track_T) not as expected. Remember to align padding if needed");
   #endif
   */
}
#endif
