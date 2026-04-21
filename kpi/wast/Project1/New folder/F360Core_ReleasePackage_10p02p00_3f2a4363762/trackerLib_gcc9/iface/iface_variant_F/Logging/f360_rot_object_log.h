#ifndef F360_ROT_OBJECT_LOG_H
#define F360_ROT_OBJECT_LOG_H

/*===================================================================================*\
 * FILE:  f360_rot_object_log.h
 *====================================================================================
 * File contains common object output struct declaration
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 */

// Add pragmas to throw error if struct is padded
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(error : 4820)
#elif defined __GNUC__ && !defined(__TASKING__)
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wpadded"
#endif

#include "f360_reuse.h"
#include "f360_constants.h"

typedef struct ROT_Object_Output_Tag
{           
   /* Object Properties at Reference Point in VCS Coordinates */
   float vcs_x_posn;                           // object longitudinal position under VCS coordinate, unit meter
   float vcs_y_posn;                           // object lateral position under VCS coordinate,unit meter
   float vcs_x_vel;                            // object longitudinal velocity under VCS coordinate, unit m/s
   float vcs_y_vel;                            // object lateral velocity under VCS coordinate, unit m/s
   float vcs_x_acc;                            // object longitudinal acceleration under VCS coordinate, unit m/s^2
   float vcs_y_acc;                            // object lateral acceleration under VCS coordinate, unit m/s^2
   float vcs_heading;                          // object heading in vcs coordinates, unit rad
   float vcs_pointing;                         // object pointing angle in vcs coordinates, unit rad
   float vcs_state_variance[6];                // variance of the state vector consisting of [vcs_xposn, vcs_yposn, vcs_heading, curvature, speed and tangential acceleration] when track filter is CTCA
                                               // and [vcs_xposn, vcs_xvel, vcs_yposn, vcs_yvel, vcs_xacc, vcs_yacc] when tracker filter is CCA
   float vcs_supplemental_state_covariance[3]; // covariance of [vcs_xposn & vcs_yposn, speed & vcs_heading, tangential acceleration & curvature] when track filter is CTCA 
                                               // and [vcs_xposn & vcs_yposn, vcs_xvel & vcs_yvel, vcs_xacc & vcs_yacc] when track filter is CCA
   float vcs_curvature;                        // trajectory curvature of object track's reference point in vcs coordinates, unit rad/m 
   
   /* Object Properties at Reference Point in ISO Coordinates */
   float iso_x_posn;                           // object longitudinal position under ISO coordinate, unit meter
   float iso_y_posn;                           // object lateral position under ISO coordinate,unit meter
   float iso_x_vel;                            // object longitudinal velocity under ISO coordinate, unit m/s
   float iso_y_vel;                            // object lateral velocity under ISO coordinate, unit m/s
   float iso_x_acc;                            // object longitudinal acceleration under ISO coordinate, unit m/s^2
   float iso_y_acc;                            // object lateral acceleration under ISO coordinate, unit m/s^2
   float iso_orientation;                      // object bounding box pointing angle under ISO coordinate, unit in radians
   float iso_orientation_var;                  // object orientation variance
   float iso_orientation_rate;                 // object orientation (heading) rate [rad/s]
   float iso_orientation_rate_var;             // object orientation (heading) rate variance
   float iso_x_posn_var;                       // object position x variance
   float iso_y_posn_var;                       // object position y variance
   float iso_xy_posn_cov;                      // object x-y position covariance
   float iso_x_vel_var;                        // object velocity x variance
   float iso_y_vel_var;                        // object velocity y variance
   float iso_xy_vel_cov;                       // object velocity x-y covariance
   float iso_x_acc_var;                        // object acceleration x variance
   float iso_y_acc_var;                        // object acceleration y variance
   float iso_xy_acc_cov;                       // object acceleration x-y covariance
   
   /*Other Object Properties*/
   float speed;                                // object speed. Speed remains the same in both vcs and iso coordinates.
                                               // speed is also reported negative when the object is reversing.
   float tang_accel;                           // object tang_accel. Remains same in both vcs and iso coordinates. 
                                               // tang_accel is also reported negative when the object is reversing
   float length;                               // object length in meters
   float length_var;                           // object length variance
   float width;                                // object width in meters
   float width_var;                            // object width variance
   float time_since_created;                   // time since object created in seconds
   float time_since_published;                 // time since object published to interface in seconds
   float time_since_stage_start;               // time since start of current track status, unit seconds 
   float existence_probability;                // object existence probability [0.0F - 1.0F]
   float mirror_prob;                          // object Probability [0.0F - 1.0F] that this track is a mirror of multi path detections (where the mirror is guardrail or other object) [-]
   float radar_cross_section;                  // object rcs value is not available on the object tracks interface yet. Default value 0 is filled
   float otg_height;                           // object over the ground height, accumulated over the objects lifetime [m]
   float confidence_level;                     // confidence level of current track, calculated based on there being at least one detection matching the object in position and range rate. Values range between [0-1]
   float probability_pedestrian;               // probability the object is a pedestrian
   float probability_car;                      // probability the object is a car
   float probability_motorcycle;               // probability the object is a motorcycle
   float probability_bicycle;                  // probability the object is a bicycle
   float probability_truck;                    // probability the object is a truck
   float probability_undet;                    // probability the object type cannot be determined
   float probability_underdrivable;            // probability that the object is underdrivable  [0.0F - 1.0F]
   int32_t id;                                 // object track identifier (id = index_in_array + 1; ) [-]
   uint32_t unique_id;                         // unique object track identifier [0 - 2^32-1] 
   uint32_t ndets;                             // number of sensor detections associated to current object track
   uint32_t num_dets_used_in_rr_msmt_update;   // number of range rate inlier detections used in the measurement update of the range-rate.
   uint16_t sensor_src;                        // object measurement radar sensor source in bitfield, starting from front mid and counter clockwise. 
                                               // bit1-front foward,bit2-front left, bit3-left side, bit4-rear left, bit5-rear center, bit6-rear right, bit7-right side, bit8-front right
   uint8_t reference_point;                    // 0 object centroid, 1 front-left, 2 front-mid, 3 front-right,  4 right-mid, 5 rear-right, 6 rear-center,7 rear-left,8 left-mid
   uint8_t object_status;                      // object status. 0 measured, 1 newly created, 2 predicited, 255 default value
                                               
   uint8_t object_class;                       // determines the type of Object detected. 0=Unknown, 1=Car, 2=Motorbike , 3=Truck, 4=Bicycle, 5=Pedestrian. [0 - 255] 
   uint8_t movement_status;                    // object movement status. 0 is default value for invalid object, 1 stationary--until now no movement has been detected, 
                                               // 2 Stopped--has been detected as moving in tracking history, 3 moving
   uint8_t occlusion_status;                   // Indicates if an object is occluded by another object. 3=Visible, 2=On edge, 1=Occluded, 0=Undefined.
   uint8_t underdrivable_status;               // enum identifying whether host can pass under track 0=can not pass, 1=likely to pass, 2=can pass, 3=invalid/not in area of interest
   uint8_t f_onguardrail;                      // indicates if the current track is on guard rail.
   uint8_t trk_fltr_type;                      // track filter type, 0-INVALID, 1-CCV, 2-CTCA, 3-CCA.
   uint8_t padding[2];
} ROT_Object_Output_T;

typedef struct Core_Info_Faults_Bit_Field_Tag
{
   uint8_t time_us_no_increase : 1;
   uint8_t cnt_loops_no_increase : 1;
   uint8_t elapsed_time_below_lower_limit : 1;
   uint8_t elapsed_time_above_upper_limit : 1;
   uint8_t ununsed : 4;
   uint8_t padding[7];
}Core_Info_Faults_Bit_Field_T;

typedef struct Host_Info_Faults_Bit_Field_Tag
{
   uint8_t vehicle_index_no_increase : 1;
   uint8_t host_speed_invalid : 1;
   uint8_t host_yawrate_invalid : 1;
   uint8_t host_longitudinal_acceleration_invalid : 1;
   uint8_t host_lateral_acceleration_invalid : 1;
   uint8_t ununsed : 3;
   uint8_t padding[7];
}Host_Info_Faults_Bit_Field_T;

typedef struct Radar_Sensor_Faults_Bit_Field_Tag
{
   uint8_t look_index_no_increase : 1;
   uint8_t sensor_vs_tracker_timestamp_divergence : 1;
   uint8_t unused : 6;
   uint8_t padding[7];
} Radar_Sensor_Faults_Bit_Fields_T;

typedef struct Radar_Sensor_Calib_Faults_Bit_Field_Tag
{
   uint8_t mounting_pos_is_invalid : 1;
   uint8_t polarity_is_invalid : 1;
   uint8_t boresight_angle_is_invalid : 1;
   uint8_t unused : 5;
   uint8_t padding[7];
} Radar_Sensor_Calib_Faults_Bit_Field_T;

typedef struct Object_Faults_Bit_Field_Tag
{
   uint8_t f_track_positions_faulty : 1;     // flag indicating that at least one track VCS position is incorrect
   uint8_t f_track_velocities_faulty : 1;    // flag indicating that at least one track OTG-velocity is incorrect
   uint8_t f_track_accelerations_faulty : 1; // flag indicating that at least one track tangential acceleration is incorrect
   uint8_t unused : 5 ;
   uint8_t padding[7];
} Object_Faults_Bit_Field_T;

typedef struct All_SCL_Faults_Tag
{
   Core_Info_Faults_Bit_Field_T core_info_faults;
   Host_Info_Faults_Bit_Field_T host_info_faults;
   Radar_Sensor_Faults_Bit_Fields_T radar_sensor_faults[f360_variant_F::MAX_NUMBER_OF_SENSORS];
   Radar_Sensor_Calib_Faults_Bit_Field_T radar_sensor_calib_faults[f360_variant_F::MAX_NUMBER_OF_SENSORS];
   Object_Faults_Bit_Field_T object_faults;
   uint8_t overall_fault_status;
   uint8_t padding[7];
}All_SCL_Faults_T;

typedef struct ROT_Object_List_Info_Tag
{
   uint64_t rot_object_list_timestamp; // it is an average timestamp of all radar sensors, computed as: rot_object_list_timestamp = min_timestamp_sensor + (max_timestamp_sensor - min_timestamp_sensor )/2.
   uint64_t tracker_start_timestamp;   // tracker started timestamp in micro seconds
   ROT_Object_Output_T rot_object_list[f360_variant_F::NUMBER_OF_REDUCED_OBJECT_TRACKS];
   float tracker_elapsed_time;     // tracker elapsed time each cycle iteration in second
   uint32_t tracker_index;
   All_SCL_Faults_T all_scl_faults;
   uint16_t number_of_objects;
   uint8_t padding[6];
}ROT_Object_List_Info_T;

// Restore MSVC and GCC warning settings
#if defined _MSC_VER
#pragma warning(pop)
#elif defined __GNUC__ && !defined(__TASKING__)
#pragma GCC diagnostic pop
#endif

#endif
