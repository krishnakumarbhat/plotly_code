#ifndef ISO_OUTPUT_ADAPTION_LOG_H
#define ISO_OUTPUT_ADAPTION_LOG_H
/*===================================================================================*\
* FILE: iso_object_list_output_log.h
*====================================================================================
* Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains the ISO Object List Output Log definition
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===================================================================================*/

//#include "f360_reuse.h"
#include <stdint.h> //Avinash
#define MAX_ISO_OBJECTS 64
#define MAX_ISO_CLASSES 6

#define ISO_OBJECTLIST_STREAM_NUM     45
#define ISO_OBJECTLIST_STREAM_VERSION 3

typedef enum
{
   TRKR_DATA_QUALIFIER_UNKNOWN_                          = 0,
   TRKR_DATA_QUALIFIER_OTHER_                            = 1,
   TRKR_DATA_QUALIFIER_AVAILABLE_                        = 2,
   TRKR_DATA_QUALIFIER_AVAILABLE_REDUCED_                = 3,
   TRKR_DATA_QUALIFIER_NOT_AVAILABLE_                    = 4,
   TRKR_DATA_QUALIFIER_AVAILABLE_REDUCED_IN_COVERAGE_    = 5,
   TRKR_DATA_QUALIFIER_AVAILABLE_REDUCED_IN_PERFORMANCE_ = 6,
   TRKR_DATA_QUALIFIER_INVALID_                          = 7,
} TRKR_DATA_QUALIFIER;

typedef enum
{
   TRKR_VEH_COORD_TYPE_REARAXLE_  = 0,
   TRKR_VEH_COORD_TYPE_ROADLEVEL_ = 1,
} TRKR_VEH_COORD_TYPE;

typedef enum
{
   ISO_OBJ_CLS_UNKNOWN_MOVING_OBJ_  = 0,
   ISO_OBJ_CLS_CAR_                 = 1,
   ISO_OBJ_CLS_TRUCK_               = 2,
   ISO_OBJ_CLS_MOTOR_BIKE_          = 3,
   ISO_OBJ_CLS_BICYCLE_             = 4,
   ISO_OBJ_CLS_PEDESTRIAN_          = 5,
   ISO_OBJ_CLS_ROAD_BOUNDRY_        = 6,
   ISO_OBJ_CLS_STATIC_OBJECT_       = 7,
   ISO_OBJ_CLS_TRAFFIC_LIGHT_       = 8,
   ISO_OBJ_CLS_TRAFFIC_SIGN_        = 9,
   ISO_OBJ_CLS_TWO_WHEELERS_        = 10,
   ISO_OBJ_CLS_OBJECT_UNDETERMIND_  = 14,
   ISO_OBJ_CLS_VEHICLE_UNDETERMIND_ = 15,
} ISO_OBJ_CLS_TYPE;

typedef enum
{
   ISO_OBJ_MEASUREMENT_STATUS_INVALID_        = 0,
   ISO_OBJ_MEASUREMENT_STATUS_NEW_            = 1,
   ISO_OBJ_MEASUREMENT_STATUS_PREDICTED_      = 2,
   ISO_OBJ_MEASUREMENT_STATUS_PARTLYMEASURED_ = 3,
   ISO_OBJ_MEASUREMENT_STATUS_MEASURED_       = 4,
   ISO_OBJ_MEASUREMENT_STATUS_OCCLUDED_       = 5,
} ISO_OBJ_MEASUREMENT_STATUS;

typedef enum
{
   ISO_OBJ_MOVEMENT_STATUS_UNKNOWN_     = 0,
   ISO_OBJ_MOVEMENT_STATUS_MOVING_      = 1,
   ISO_OBJ_MOVEMENT_STATUS_SLOW_MOVING_ = 2,
   ISO_OBJ_MOVEMENT_STATUS_FAST_MOVING_ = 3,
   ISO_OBJ_MOVEMENT_STATUS_STATIONARY_  = 4,
   ISO_OBJ_MOVEMENT_STATUS_STOPPED_     = 5,
   ISO_OBJ_MOVEMENT_STATUS_RESERVED1_   = 6,
   ISO_OBJ_MOVEMENT_STATUS_RESERVED2_   = 7,
} ISO_OBJ_MOVEMENT_STATUS;

typedef enum
{
   ISO_OBJ_TRACK_MOTION_MODEL_INVALID_ = 0,
   ISO_OBJ_TRACK_MOTION_MODEL_CCV_     = 1,
   ISO_OBJ_TRACK_MOTION_MODEL_CTCA_    = 2,
   ISO_OBJ_TRACK_MOTION_MODEL_CCA_     = 3,
} ISO_OBJ_TRACK_MOTION_MODEL;

typedef struct ISO_RDR_HEADER_Tag
{
   float tracker_time_peak; /* [ms] : Max time of single tracker instance execution (difference between tracker execution time
                               start and end) */
   float tracker_time_inst; /* [ms] : Time of single tracker instance execution (difference between tracker execution time start
                               and end) */
   uint32_t tracker_timestamp_ns;  /* [ns] : Object list timestamp related to measurement timestamp (nanoseconds component of
                                      "broken down" time) */
   uint32_t tracker_timestamp_sc;  /* [s] : Object list timestamp related to measurement timestamp (seconds component of "broken
                                      down" time ) */
   uint16_t vehicle_idx;           /* Index of the VSE iteration */
   uint16_t tracker_idx;           /* Index of the current tracker iteration */
   uint8_t tracker_veh_coord_type; /* [enum] : Vehicle coordination type: REARAXLE, ROADLEVEL */
   uint8_t tracker_data_qualifier; /* [enum] : Describes the quality level of the data provided by this interface: AVAILABLE,
                                      AVAILABLE_REDUCED, NOT_AVAILABLE, INVALID */
   uint8_t tracker_num_of_active_objs; /* Number of valid potentially moving objects */
   uint8_t reserved;
} ISO_RDR_HEADER_T;

typedef struct ISO_RDR_STATUS_Tag
{
   uint16_t sts_perception_module_id;  /* Sum of each sensor ID + 100 */
   uint8_t sts_tracker_patch_version;  /* Tracker software patch version */
   uint8_t sts_tracker_minor_version;  /* Tracker software minor version */
   uint8_t sts_tracker_major_version;  /* Tracker software major version */
   uint8_t sts_obj_dbc_base_ver_minor; /* Object tracker base dbc version minor */
   uint8_t sts_obj_dbc_base_ver_major; /* Object tracker base dbc version major */
   uint8_t reserved;
} ISO_RDR_STATUS_T;

typedef struct ISO_RDR_TRACKER_OBJ_Tag
{
   float acceleration_long_err; /* [m/s^2] : Standard deviation of acceleration of the object along the x-axis of the ego-vehicle
                                   coordinate system */
   float acceleration_lat_err;  /* [m/s^2] : Standard deviation of acceleration of the object along the y-axis of the ego-vehicle
                                   coordinate system */
   float velocity_long_err; /* [m/s] : Standard deviation of velocity of the object along the x-axis of the ego-vehicle coordinate
                               system */
   float velocity_lat_err;  /* [m/s] : Standard deviation of velocity of the object along the y-axis of the ego-vehicle coordinate
                               system */
   float
      pos_long_err; /* [m] : Standard deviation of position of the object along the x-axis of the ego-vehicle coordinate system */
   float
      pos_lat_err; /* [m] : Standard deviation of position of the object along the y-axis of the ego-vehicle coordinate system */
   float dimension_width;     /* [m] : Width of the object */
   float dimension_length;    /* [m] : Length of the object */
   float existence_prob;      /* [0..1] Existence probability of the object */
   float yaw_angle;           /* [rad] : Orientation of the object bounding box in the ego-vehicle coordinate system.
                                        The coordinate system is defined with 0 defined along the x-axis of the ego-vehicle with positive to
                                 the left           and negative to the right with range from @f$(\pi, -\pi)@f$ (counter-clock wise). This is
                                 equivalent to the           angular difference between the ego-vehicle's x-axis and the object's x-axis.
                                 Note that the           orientation correspond to the orientation of the bounding box and not the
                                 orientation based on           the           object's velocity vector  */
   float yaw_angle_err;       /* [rad] : Standard deviation of orientation of the object bounding box in the ego-vehicle coordinate
                                 system */
   float pos_long;            /* [m] : Position of the object along the x-axis of the ego-vehicle coordinate system */
   float pos_lat;             /* [m] : Position of the object along the y-axis of the ego-vehicle coordinate system */
   float velocity_long;       /* [m/s] : Velocity of the object along the x-axis of the ego-vehicle coordinate system */
   float velocity_lat;        /* [m/s] : Velocity of the object along the y-axis of the ego-vehicle coordinate system */
   float acceleration_long;   /* [m/s^2] : Acceleration of the object along the x-axis of the ego-vehicle coordinate system */
   float acceleration_lat;    /* [m/s^2] : Acceleration of the object along the y-axis of the ego-vehicle coordinate system */
   float dimension_width_err; /* [m] : Standard deviation of the width */
   float dimension_length_err; /* [m] : Standard deviation of the length */
   float ref_pt_long_offset;   /* [0..1] X-axis offset ratio of ref point to rear right corner */
   float ref_pt_lat_offset;    /* [0..1] Y-axis offset ratio of ref point to rear right corner */
   float
      yaw_rate; /* [rad/s] : Orientation rate of the object. Sign of the orientation rate is defined with respect to the y-axis */
   float yaw_rate_err; /* [rad/s] : Standard deviation of orientation rate of the object. Sign of the orientation rate is defined
                          with respect to the y-axis */
   float pos_err_crr_coef; /* [m] : Pearson correlation coefficient between the x and y position components (covariance divided by
                              product of standard deviations for x and y) */
   float velocity_err_crr_coef; /* [m/s] : Pearson correlation coefficient between the x and y velocity components (covariance
                                   divided by product of standard deviations for x and y) */
   float accel_err_crr_coef;   /* [m/s^2] : Pearson correlation coefficient between the x and y acceleration components (covariance
                                  divided by product of standard deviations for x and y) */
   float cls_type1_prob;       /* [0..1] : Classification Type 1 probability (highest) */
   float cls_type2_prob;       /* [0..1] : Classification Type 2 probability */
   float cls_type3_prob;       /* [0..1] : Classification Type 3 probability */
   float cls_type4_prob;       /* [0..1] : Classification Type 4 probability */
   float cls_type5_prob;       /* [0..1] : Classification Type 5 probability */
   float cls_type6_prob;       /* [0..1] : Classification Type 6 probability (lowest) */
   uint16_t alive_time;        /* [ms] : Time since the creation of current track (saturated at 65535ms) */
   int16_t timestamp_offset;   /* [ms] : not implemented */
   uint8_t group_id;           /* not implemented */
   uint8_t id;                 /* [0-250] : ID of object track, zero-ID represents an inactive track */
   uint8_t track_motion_model; /* [enum] : Track motion model : INVALID, CCV, CTCA */
   uint8_t movement_status;    /* [enum] : Information about a possible movement of the object during tracking : MOVING, STOPPED,
                                  STATIONARY */
   uint8_t measurement_status; /* [enum] : Measurement status - object level : INVALID, NEW, PREDICTED, MEASURED */
   uint8_t
      cls_type1; /* [enum] : Classification Type 1 (highest) : PEDESTRIAN, CAR, MOTOR_BIKE, BICYCLE, TRUCK, OBJECT_UNDETERMIND */
   uint8_t cls_type2; /* [enum] : Classification Type 2 : PEDESTRIAN, CAR, MOTOR_BIKE, BICYCLE, TRUCK, OBJECT_UNDETERMIND */
   uint8_t cls_type3; /* [enum] : Classification Type 3 : PEDESTRIAN, CAR, MOTOR_BIKE, BICYCLE, TRUCK, OBJECT_UNDETERMIND */
   uint8_t cls_type4; /* [enum] : Classification Type 4 : PEDESTRIAN, CAR, MOTOR_BIKE, BICYCLE, TRUCK, OBJECT_UNDETERMIND */
   uint8_t cls_type5; /* [enum] : Classification Type 5 : PEDESTRIAN, CAR, MOTOR_BIKE, BICYCLE, TRUCK, OBJECT_UNDETERMIND */
   uint8_t
      cls_type6; /* [enum] : Classification Type 6 (lowest) : PEDESTRIAN, CAR, MOTOR_BIKE, BICYCLE, TRUCK, OBJECT_UNDETERMIND */
   int8_t rcs;   /* [dB/m^2] : not implemented */
   int8_t veh_track_flag; /* [bool] : indicates if current track is a vehicle */
   int8_t reserved[3];
} ISO_RDR_TRACKER_OBJ_T;

typedef struct ISO_Object_List_Output_Log_Tag
{
   ISO_RDR_HEADER_T tracker_header;
   ISO_RDR_STATUS_T tracker_status;
   ISO_RDR_TRACKER_OBJ_T tracker_object_list[MAX_ISO_OBJECTS];
} ISO_Object_List_Output_Log_T;

#define ISO_OBJECT_LIST_OUTPUT_ADAPTAION_ASSERT(expn) typedef char __ISO_OBJECT_LIST_OUTPUT_ADAPTAION_ASSERT__[(expn) ? 1 : -1]
// ISO_OBJECT_LIST_OUTPUT_ADAPTAION_ASSERT(sizeof(ISO_Object_List_Output_Log_T) == 9504);
// ISO_OBJECT_LIST_OUTPUT_ADAPTAION_ASSERT((sizeof(ISO_Object_List_Output_Log_T) / ISO_OBJECT_LIST_LOG_NUM_CHUNKS) *
// ISO_OBJECT_LIST_LOG_NUM_CHUNKS == sizeof(ISO_Object_List_Output_Log_T));

#endif
