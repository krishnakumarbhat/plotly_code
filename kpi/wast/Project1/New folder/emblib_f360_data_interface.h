#ifndef F360_DATA_INTERFACE_H
#define F360_DATA_INTERFACE_H
/*===========================================================================*\
 * File: F360_data_interface.h
 *===========================================================================
 * Copyright 2018 Aptiv, Inc., All Rights Reserved.
 * Aptiv Confidential
 *---------------------------------------------------------------------------
 * Created on: October 10, 2023
 * Author: Dikshant Patel
 *---------------------------------------------------------------------------
 *
 * Description:
 *    Interface between .cpp and .c files required to populate F360 inputs and
 *    outputs
 *
\*===========================================================================*/

/*===========================================================================*\
 * Standard Header Includes
\*===========================================================================*/
#include <stdbool.h>

/*===========================================================================*\
 * Project Header Includes
\*===========================================================================*/
#include "T360_Types.h"
#include "f360_rot_object_log.h"
#include "reuse.h"

/*===========================================================================*\
 * Exported Preprocessor Constants
\*===========================================================================*/

/*===========================================================================*\
 * Types specific for tracker Sandeep
\*===========================================================================*/
#ifndef signed8_T
typedef signed char signed8_T;
#endif

#ifndef signed16_T
typedef signed char signed16_T;
#endif

#ifndef unsigned16_T
typedef unsigned char unsigned16_T;
#endif

/*#ifndef bool
typedef boolean_T bool; // Sandeep
#endif*/

/*Downselected, CAN Detections*/
//#define MAX_TARGET_REPORTS        256
#define NUMBER_OF_F360_DETECTIONS 64
/*===========================================================================*\
 * Exported Type Declarations
\*===========================================================================*/
typedef enum
{
   INPUT_DET_CONFD_ELEVATION_HIGH_    = 0,
   INPUT_DET_CONFD_ELEVATION_MIDHIGH_ = 1,
   INPUT_DET_CONFD_ELEVATION_MIDLOW_  = 2,
   INPUT_DET_CONFD_ELEVATION_LOW_     = 3,
} INPUT_DET_CONFD_ELEVATION;

typedef enum
{
   INPUT_DET_CONFD_AZIMUTH_HIGH_    = 0,
   INPUT_DET_CONFD_AZIMUTH_MIDHIGH_ = 1,
   INPUT_DET_CONFD_AZIMUTH_MIDLOW_  = 2,
   INPUT_DET_CONFD_AZIMUTH_LOW_     = 3,
} INPUT_DET_CONFD_AZIMUTH;

typedef enum
{
   INPUT_SSTS_RIGHT_BLINK_OFF_ = 0,
   INPUT_SSTS_RIGHT_BLINK_ON_  = 1,
} INPUT_SSTS_RIGHT_BLINK;

typedef enum
{
   INPUT_SSTS_LEFT_BLINK_OFF_ = 0,
   INPUT_SSTS_LEFT_BLINK_ON_  = 1,
} INPUT_SSTS_LEFT_BLINK;

typedef enum
{
   INPUT_SSTS_LROS_DRIVE_REVERSE_NOT_REVERSE_ = 0,
   INPUT_SSTS_LROS_DRIVE_REVERSE_REVERSE_     = 1,
} INPUT_SSTS_LROS_DRIVE_REVERSE;

typedef enum
{
   INPUT_SSTS_LROS_DRIVE_DIR_UNKNOWN_  = 0,
   INPUT_SSTS_LROS_DRIVE_DIR_BACKWARD_ = 1,
   INPUT_SSTS_LROS_DRIVE_DIR_FORWARD_  = 2,
   INPUT_SSTS_LROS_DRIVE_DIR__         = 4,
} INPUT_SSTS_LROS_DRIVE_DIR;

typedef enum
{
   INPUT_SSTS_ELPW_D_STAT_NOT_SUPPORTED_          = 0,
   INPUT_SSTS_ELPW_D_STAT_SUPPORTED_              = 1,
   INPUT_SSTS_ELPW_D_STAT_NOT_SUPPORTED_IMMINENT_ = 2,
   INPUT_SSTS_ELPW_D_STAT_LV_EVENT_IN_PROGRESS_   = 3,
   INPUT_SSTS_ELPW_D_STAT_FAULT_LIMITED_          = 4,
   INPUT_SSTS_ELPW_D_STAT_NOTUSED_1_              = 5,
   INPUT_SSTS_ELPW_D_STAT_NOTUSED_2_              = 6,
   INPUT_SSTS_ELPW_D_STAT_NOTUSED_3_              = 7,
} INPUT_SSTS_ELPW_D_STAT;

typedef enum
{
   INPUT_SSTS_CC_STAT_D_ACTL_OFF_               = 0,
   INPUT_SSTS_CC_STAT_D_ACTL_DENIED_            = 1,
   INPUT_SSTS_CC_STAT_D_ACTL_STANDBY_DENIED_    = 2,
   INPUT_SSTS_CC_STAT_D_ACTL_STANDBY_           = 3,
   INPUT_SSTS_CC_STAT_D_ACTL_ACTIVE_QUE_ASSIST_ = 4,
   INPUT_SSTS_CC_STAT_D_ACTL_ACTIVE_            = 5,
   INPUT_SSTS_CC_STAT_D_ACTL_UNDEFINED_1_       = 6,
   INPUT_SSTS_CC_STAT_D_ACTL_UNDEFINED_2_       = 7,
} INPUT_SSTS_CC_STAT_D_ACTL;

typedef enum
{
   INPUT_SSTS_PWPCKTQ_D_STAT_PWPCKOFF_TQNOTAVAILABLE_  = 0,
   INPUT_SSTS_PWPCKTQ_D_STAT_PWPCKON_TQNOTAVAILABLE_   = 1,
   INPUT_SSTS_PWPCKTQ_D_STAT_STARTINPRGRSS_TQNOTAVAIL_ = 2,
   INPUT_SSTS_PWPCKTQ_D_STAT_PWPCKON_TQAVAILABLE_      = 3,
} INPUT_SSTS_PWPCKTQ_D_STAT;

typedef enum
{
   INPUT_SSTS_WIPER_STATUS_OFF_ = 0,
   INPUT_SSTS_WIPER_STATUS_ON_  = 1,
} INPUT_SSTS_WIPER_STATUS;

typedef enum
{
   INPUT_SSTS_GEAR_POSITION_PARK_             = 0,
   INPUT_SSTS_GEAR_POSITION_REVERSE_          = 1,
   INPUT_SSTS_GEAR_POSITION_NEUTRAL_          = 2,
   INPUT_SSTS_GEAR_POSITION_DRIVE_            = 3,
   INPUT_SSTS_GEAR_POSITION_SPORT_DRIVESPORT_ = 4,
   INPUT_SSTS_GEAR_POSITION_LOW_              = 5,
   INPUT_SSTS_GEAR_POSITION_FIRST_            = 6,
   INPUT_SSTS_GEAR_POSITION_SECOND_           = 7,
   INPUT_SSTS_GEAR_POSITION_THIRD_            = 8,
   INPUT_SSTS_GEAR_POSITION_FOURTH_           = 9,
   INPUT_SSTS_GEAR_POSITION_FIFTH_            = 10,
   INPUT_SSTS_GEAR_POSITION_SIXTH_            = 11,
   INPUT_SSTS_GEAR_POSITION_NOTUSED_1_        = 12,
   INPUT_SSTS_GEAR_POSITION_NOTUSED_2_        = 13,
   INPUT_SSTS_GEAR_POSITION_UNKNOWN_POSITION_ = 14,
   INPUT_SSTS_GEAR_POSITION_FAULT_            = 15,
} INPUT_SSTS_GEAR_POSITION;

typedef enum
{
   INPUT_SSTS_IGN_STATUS_UNKNOWN_   = 0,
   INPUT_SSTS_IGN_STATUS_OFF_       = 1,
   INPUT_SSTS_IGN_STATUS_ACCESSORY_ = 2,
   INPUT_SSTS_IGN_STATUS_RUN_       = 4,
   INPUT_SSTS_IGN_STATUS_START_     = 8,
   INPUT_SSTS_IGN_STATUS_INVALID_   = 15,
} INPUT_SSTS_IGN_STATUS;

typedef enum
{
   INPUT_SSTS_VEHICLE_SPEED_CALC_QF_UNDEFINED_0            = 0,
   INPUT_SSTS_VEHICLE_SPEED_CALC_QF_TEMPORARILY_UNDEFINED_ = 1,
   INPUT_SSTS_VEHICLE_SPEED_CALC_QF_INNACCURATE_           = 2,
   INPUT_SSTS_VEHICLE_SPEED_CALC_QF_ACCURATE_              = 3,
} INPUT_SSTS_VEHICLE_SPEED_CALC_QF;

typedef enum
{
   INPUT_SSTS_YAW_RATE_CALC_QF_UNDEFINED_0            = 0,
   INPUT_SSTS_YAW_RATE_CALC_QF_TEMPORARILY_UNDEFINED_ = 1,
   INPUT_SSTS_YAW_RATE_CALC_QF_INNACCURATE_           = 2,
   INPUT_SSTS_YAW_RATE_CALC_QF_ACCURATE_              = 3,
} INPUT_SSTS_YAW_RATE_CALC_QF;

typedef struct
{
   INPUT_SSTS_YAW_RATE_CALC_QF ssts_yaw_rate_calc_qf;
   INPUT_SSTS_VEHICLE_SPEED_CALC_QF ssts_vehicle_speed_calc_qf;
   INPUT_SSTS_IGN_STATUS ssts_ign_status;
   INPUT_SSTS_GEAR_POSITION ssts_gear_position;
   INPUT_SSTS_WIPER_STATUS ssts_wiper_status;
   INPUT_SSTS_PWPCKTQ_D_STAT ssts_pwpcktq_d_stat; // SED
   INPUT_SSTS_CC_STAT_D_ACTL ssts_cc_stat_d_actl; // SED
   INPUT_SSTS_ELPW_D_STAT ssts_elpw_d_stat;       // SED
   INPUT_SSTS_LROS_DRIVE_DIR ssts_lros_drive_dir;
   INPUT_SSTS_LROS_DRIVE_REVERSE ssts_lros_drive_reverse;
   INPUT_SSTS_LEFT_BLINK ssts_left_blink;
   INPUT_SSTS_RIGHT_BLINK ssts_right_blink;
   uint32_t ssts_sw_version_asdm;
   float ssts_yaw_rate_bias;       // rad/s
   float ssts_yaw_rate_calc;       // rad/s
   float ssts_veh_spd_comp_factor; // NA
   float ssts_vehicle_speed_calc;  // m/s
   float ssts_veh_batt_volt;       // V
   float ssts_vehovergnd_v_est;    // kph
   float ssts_steering_angle;      // rad
   float veh_time_stamp_s;         // sec
   uint8_t ssts_can_pcan_minor_asdm;
   uint8_t ssts_can_pcan_major_asdm;
   bool ssts_steering_angle_sign;
} input_SYS_VEHICLE_STATUS_T;

typedef struct
{
   INPUT_DET_CONFD_ELEVATION det_confd_azimuth;
   INPUT_DET_CONFD_ELEVATION det_confd_elevation;
   float det_range;              // m
   float det_range_err;          // m
   float det_range_velocity;     // m/s
   float det_range_velocity_err; // m/s
   float det_azimuth;            // rad
   float det_azimuth_err;        // rad
   float det_elevation;          // rad
   float det_elevation_err;      // rad
   float det_snr;                // dB
   float det_point_target_prob;  // %
   float det_existence_prob;     // %
   float det_ambg_azimuth_prob;  // %
   uint8_t det_azimuth_amb_id;
   int8_t det_rcs; // dBsm
   bool det_valid_flag;
   bool det_super_res_target;
   bool det_outside_sector;
   bool det_hvc_flag;
} input_RDR_DETECTION_T;

typedef struct Host_Vehicle_Params_If_Tag
{
   signed8_T polarity;
   float boresight_angle_azimuth;
   float boresight_angle_el;
   signed16_T Veh_LatPos;
   signed16_T Veh_LongPos;
   unsigned16_T Veh_VertPos;
} Host_Vehicle_Params_If_T;

typedef struct F360_ISO_Object_Stream_TAG
{
   uint64_t object_list_timestamp;
   ROT_Object_Output_T obj[MAX_F360_OBJECTS];
   uint32_t tracker_index;
} F360_ISO_Object_Stream_T;

#define FREERUNNING_TIMER_5MHZ_DEVICE 1 /*Sandeep change this*/

/*===========================================================================*\
 * Exported Function Prototypes
\*===========================================================================*/

extern uint32_t Get_System_Time(void);
extern F360_PRNDL_STATE Get_f360_prndl_state(int gear_position);

#endif /* F360_DATA_INTERFACE_H */
