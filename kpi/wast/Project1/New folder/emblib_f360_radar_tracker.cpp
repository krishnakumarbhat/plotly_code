
/*===========================================================================*\
 * File: f360_radar_tracker.cpp
 *===========================================================================
 * Copyright 2018 Aptiv, Inc., All Rights Reserved.
 * Aptiv Confidential
 *---------------------------------------------------------------------------
 * Created on: August 24, 2018
 * Author: Eric Schrock (qj91h7)
 *---------------------------------------------------------------------------
 *
 * Description:
 *    Embedded wrapper for the F360 radar tracker.
 *
\*===========================================================================*/
#include "f360_radar_sensor.h"

// #include "f360_types.h"
#include "sil_udp_iface.h"

namespace f360_variant_C
{
};
using namespace f360_variant_C;

/*===========================================================================*\
 * Standard Header Includes
\*===========================================================================*/
#include <string.h>

/*===========================================================================*\
 * Project Header Includes
\*===========================================================================*/
#include "emblib_ROT_cals.h"
#include "T360_Types.h"
#include "emblib_VSE_Core.h"
#include "emblib_f360_data_interface.h"
#include "f360_get_wall_time.h"
#include "f360_input_diagnostics.h"
// #include "f360_iso_object_list_output_adaptation.h"
// #include "f360_math_func.h"
#include "State_Manager.h"
#include "f360_object_log.h"
// #include "ProcessDetectionLog.h"
// #include "ReducedObjectsLog.h"
// #include "f360_detection_list.h"
#include "emblib_f360_radar_tracker.h"
#include "f360_output_diagnostics.h"
#include "f360_safety_control_logic.h"
#include "f360_tracker.h"
// #include "iso_object_list_output_log.h"
#include "reuse.h"
#include "rspp_calibrations.h"
#include "rspp_core_info.h"
#include "rspp_detection_list.h"
#include "rspp_host.h"
#include "rspp_inputs_preprocessing.h"
#include "smc_cal.h"
/*===========================================================================*\
 * Local Preprocessor Constants
\*===========================================================================*/

#ifndef US_2_S
   #define US_2_S (1e-6f)
#endif

#ifndef MM_2_M
   #define MM_2_M (0.001)
#endif
#ifndef S_2_US
   #define S_2_US (1e+6f)
#endif

#ifndef DEG2RAD
   #define DEG2RAD ((double)0.0174532925199433)
   #define RAD2DEG ((double)1 / DEG2RAD)
#endif

#ifndef DEG2RADF
   #define DEG2RADF ((float)0.0174532925199433)
   #define RAD2DEGF ((float)1 / DEG2RAD)
#endif

#ifndef CENTIMETER2METER
   #define CENTIMETER2METER (0.01f)
#endif

#define MASTER_INDEX (0U)
#define MASTER_ID    (1U)
#define SLAVE_INDEX  (1U)
#define SLAVE_ID     (2U)

#ifndef NEG_FLOAT
   #define NEG_FLOAT (-1.0f)
#endif

#ifndef unsigned8_T
typedef unsigned char unsigned8_T; // Sandeep
#endif

/*===========================================================================*\
 * Local Type Declarations
\*===========================================================================*/
#if F360_HIL_STUB
static int amp_inc        = 0;
static int stub_look_type = 0;
#endif

/*===========================================================================*\
 * Local Object Definitions
\*===========================================================================*/

using namespace f360_variant_C;
using vse_core::VSE_CORE;

/* F360 Object */
static F360_Tracker f360_tracker;
static volatile uint8_t enable_master_det = 1;

static VSE_CORE core_vse;

/* Alignment */
static float Align_Angle_Az_Rad[MAX_NUMBER_OF_SENSORS] = {0.0f};
static float Align_Angle_El_Rad[MAX_NUMBER_OF_SENSORS] = {0.0f};

/* VSE Input */
#pragma DATA_SECTION(".l3ram")
static F360_Host_Calib_T F360_Host_Calib;
static F360_Host_Raw_T *F360_Host_Raw;
#pragma DATA_SECTION(".l3ram")
static F360_Core_Info_T F360_Core_Info;

/* VSE Output */
#pragma DATA_SECTION(".l3ram")
static F360_Host_T F360_Host;

/* Tracker Input */
#pragma DATA_SECTION(".l3ram")
static F360_Radar_Sensor_T(F360_Sensors)[MAX_NUMBER_OF_SENSORS];
// #pragma DATA_SECTION(".l3ram")
// static F360_Radar_Sensor_Calib_T(F360_Sensors_Calib)[MAX_NUMBER_OF_SENSORS];

static F360_Detection_List_T *F360_Detection_List;

/* Tracker Output */
static Reduced_Objects_Log_T *F360_Reduced_Objects_Log;
static Process_Detections_Log_T *F360_Processed_Detections_Log;
static F360_Detection_Log_Output_T F360_Detection_Output;
static Functional_Safety_Faults_Log_T *F360_Functional_Safety_Faults_Log;
static Vehicle_Info_Log_T *F360_Vehicle_Info_Log;
static Tracker_Info_Log_T *F360_Tracker_Info_Log;
static F360_Host_Props_Log_T *F360_Host_Props;

#if F360_DEBUG
static F360_Object_Log_Output_T *Object_Log;
static All_Objects_Log_T *F360_All_Objects_Log;
static Timing_Info_Log_T *F360_Timing_Info_Log;
#endif

#if ISO_OUTPUT
static F360_ISO_Object_Stream_T *ISO_ObjectList_Log;
#endif

/* Safety Control Logic */
static SafetyControlLogic *safety_logic;

/* Diagnostics */
static Input_Diagnostics input_diagnostics;

static Output_Diagnostics output_diagnostics;

static SafetyControlLogic safety_control_logic(input_diagnostics, output_diagnostics);

static f360_variant_C::State_Manager SM_C(safety_control_logic, f360_tracker);

// RSPP
static rspp_variant_C::RSPP_Calibrations_T rspp_calibs_C;
static rspp_variant_C::RSPP_Detection_List_T detection_list_C{};

static RSPP_Core_Info_T rspp_core_info_C{};
static RSPP_Host_T rspp_host_C{};
/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/
#if F360_HIL_STUB
static void Update_Host_Raw_Stub();
static void Update_Detections_Stub();
#else
static void Update_Host_Raw(Veh_Info_T *p_raw_veh_data);
   #if defined(ROT_STANDALONE)
static void Update_Detections(D2M_Payload_T *p_dss_to_mss_l3);
   #elif defined(ROT_FUSION_MASTER)
static void Update_Detections(D2M_Payload_T *p_dss_to_mss_l3, AF_Det_Slave_T *p_det_slave_data,
                              GLOB_TS_STRUCT_TYPE *p_slave_det_ts, GLOB_TS_STRUCT_TYPE *p_master_det_ts);
   #endif
#endif
static void Update_Core_Info(void);

static void Init_Sensors_Calibrations(uint8_t radar_position);
static void F360_Update_Sensor_Motion(void);
static bool Update_Look_Index_Available_Flag(void);

static void Populate_Functional_Safety_Faults_Log_Data(void);
static void Populate_Vehicle_Info_Log_Data(void);
static void Populate_Sensor_Info_Log_Data(void);

/*===========================================================================*\
 * Function Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Function: get_wall_time()
 *===========================================================================
 * Return Value:
 *  - float
 *
 * Description:
 *   Returns current system time in seconds
 *
\*===========================================================================*/
float get_wall_time()
{
   uint32_t time_us = Get_System_Time();
   return static_cast<float>(time_us) / 1e6F;
}
/*===========================================================================*\
 * Function: Initialize_F360_Radar_Tracker
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  - static F360_Radar_Sensor_T F360_Sensors[MAX_NUMBER_OF_SENSORS]
 *  - static F360_Host_Raw_T F360_Host_Raw
 *  - static F360_Detection_List_T F360_Detection_List
 *  - static F360_Host_T F360_Host
 *  - static F360_Core_Info_T F360_Core_Info
 *  - static Tracker_Info_Log_T F360_Tracker_Info_Log
 *  - static Vehicle_Info_Log_T F360_Vehicle_Info_Log
 *  - static Process_Detections_Log_T F360_Processed_Detections_Log
 *  - static Reduced_Objects_Log_T F360_Reduced_Objects_Log
 *  - static All_Objects_Log_T F360_All_Objects_Log
 *  - static Guard_Rails_Log_T F360_Guardrails_Log
 *  - static Timing_Info_Log_T F360_Timing_Info_Log
 *  - static int32_t Radar_Idx_Map[MAX_NUMBER_OF_SENSORS]
 *
 * Description:
 *    Initialize the F360 radar tracker.
 *
\*===========================================================================*/
void Initialize_F360_Radar_Tracker(uint8_t radar_position)
{
   /* Initialize I/O Structures */
   Initialize_host_calib();

   /* Initialize VSE */
   core_vse.Initialize(F360_Host_Calib);

   rspp_variant_C::Initialize_RSPP_Calibrations(rspp_calibs_C);

   /* Initialize F360 Tracker */
   f360_tracker.Initialize();

   /* Set SCL */
   safety_logic = static_cast<SafetyControlLogic *>(&safety_control_logic);

   /* Init sensor cals from smc*/
   Init_Sensors_Calibrations(radar_position);
}

/*===========================================================================*\
 * Function: Initialize_host_calib
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - void
 *
 * External References:
 *
 * Description:
 *    Initialize the F360 Host Calib.
 *
\*===========================================================================*/
void Initialize_host_calib(void)
{
   /* Init Host Calib */
   F360_Host_Calib.dist_rear_axle_to_vcs_m   = DIST_FRONT_2_REAR_AXLE;
   F360_Host_Calib.rear_cornering_compliance = REAR_CORNERING_COMPLIANCE;
   F360_Host_Calib.steer_gear_ratio =
      14.8F; // TODO: Where to get this value? The ratio between the turn of the steering wheel and the turn of the wheels
   F360_Host_Calib.wheelbase_m            = WHEELBASE;
   F360_Host_Calib.understeer_coefficient = REAR_CORNERING_COMPLIANCE; // According to VSE team, this can be used as default value
   F360_Host_Calib.vehicle_width_m        = VEHICLE_WIDTH;
   F360_Host_Calib.vehicle_length_m       = VEHICLE_LENGTH;
   F360_Host_Calib.cog_x                  = -1.0F; // Currently not used
   F360_Host_Calib.cog_y                  = -1.0F; // Currently not used
   F360_Host_Calib.front_wheel_radius_m   = -1.0F; // Currently not used
   F360_Host_Calib.front_track_width_m    = -1.0F; // Currently not used

   F360_Host_Calib.raw_host_signal_latency_ms                 = 30;
   F360_Host_Calib.f_enable_internal_reflections_func         = 0; // CAF_Parameter.C_Internal_Reflections_Enable_Function;
   F360_Host_Calib.f_enable_internal_reflections_func_trailer = 0; // CAF_Parameter.C_LCW_trailer_mode ;
}

template <typename RSPP_Core_T, typename Core_Info_T>
static void Set_RSPP_Core_Info(RSPP_Core_T &rspp_core_info, Core_Info_T &core_info)
{
   rspp_core_info.cnt_loops      = core_info.cnt_loops;
   rspp_core_info.elapsed_time_s = core_info.elapsed_time_s;
   rspp_core_info.prev_time_us   = core_info.prev_time_us;
   rspp_core_info.time_us        = core_info.time_us;
}

template <typename RSPP_T, typename Host_T> static void Set_RSPP_Host_Info(RSPP_T &rspp_host, Host_T &host)
{
   rspp_host.vehicle_index             = host.vehicle_index;
   rspp_host.speed                     = host.speed;
   rspp_host.vcs_speed                 = host.vcs_speed;
   rspp_host.acceleration              = host.acceleration;
   rspp_host.vcs_lat_acceleration      = host.vcs_lat_acceleration;
   rspp_host.vcs_long_acceleration     = host.vcs_long_acceleration;
   rspp_host.yaw_rate_rad              = host.yaw_rate_rad;
   rspp_host.vcs_sideslip              = host.vcs_sideslip;
   rspp_host.curvature_rear            = host.curvature_rear;
   rspp_host.dist_rear_axle_to_vcs_m   = host.dist_rear_axle_to_vcs_m;
   rspp_host.rear_cornering_compliance = host.rear_cornering_compliance;
   rspp_host.speed_correction_factor   = host.speed_correction_factor;
   rspp_host.speed_qf                  = host.speed_qf;
   rspp_host.yaw_rate_qf               = host.yaw_rate_qf;
   rspp_host.lat_accel_qf              = host.lat_accel_qf;
   rspp_host.long_accel_qf             = host.long_accel_qf;
}
/*===========================================================================*\
 * Function: Run_F360_Radar_Tracker
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - Detection_Stream_T *p_raw_det_data  -> Pointer to raw detection data from angle finding output
 *  - Veh_Info_T *p_raw_veh_data          -> Pointer to raw vehicle info from mss ipc buffer
 *
 * External References:
 *  - static F360_Radar_Sensor_T F360_Sensors[MAX_NUMBER_OF_SENSORS]
 *  - static F360_Host_Raw_T F360_Host_Raw
 *  - static F360_Detection_List_T F360_Detection_List
 *  - static F360_Host_T F360_Host
 *  - static Process_Detections_Log_T F360_Processed_Detections_Log
 *  - static Reduced_Objects_Log_T F360_Reduced_Objects_Log
 *  - static Guard_Rails_Log_T F360_Guardrails_Log
 *  - static Timing_Info_Log_T F360_Timing_Info_Log
 *
 * Description:
 *    Run the F360 radar tracker.
 *
\*===========================================================================*/

void Run_F360_Radar_Tracker(Emblib_M2D_Msg_T *p_ipc_m2d, Emblib_D2M_Msg_T *p_dss_to_mss_l3, GLOB_TS_STRUCT_TYPE *det_master_ts)
{
   (void)memset((void *)F360_Detection_List, (int)0, sizeof(F360_Detection_List));
   (void)memset((void *)F360_Host_Raw, (int)0, sizeof(F360_Host_Raw));

#if F360_HIL_STUB
   Update_Host_Raw_Stub();
   Update_Detections_Stub();
#else
   Emblib_D2M_Payload_T *p_det_master_data = &p_dss_to_mss_l3->payload;
   Veh_Info_T *p_raw_veh_data = &p_ipc_m2d->payload.veh_data.veh_params; // Pointer to raw vehicle info from mss ipc buffer

   /* Set F360 inputs */
   Update_Host_Raw(p_raw_veh_data);

   #if defined(ROT_STANDALONE)
   Update_Detections(p_det_master_data);
   #elif defined(ROT_FUSION_MASTER)
   AF_Det_Slave_T *p_slave_det_data    = &(p_ipc_m2d->payload.slaves_detections[0]); /* assuming number of slaves=1 */
   GLOB_TS_STRUCT_TYPE *p_det_slave_ts = &(p_ipc_m2d->payload.af_slaves_ts[0]);      /* assuming number of slaves=1 */
   Update_Detections(p_det_master_data, p_slave_det_data, p_det_slave_ts, det_master_ts);
   #endif
#endif

   /* Update core info data */
   Update_Core_Info();

   /* Run VSE Core */
   core_vse.Step((uint64_t)(Get_System_Time()), F360_Host.speed_correction_factor, *F360_Host_Raw);
   core_vse.Get_Output(F360_Host);

   /* Update F360 Sensor Motion */
   F360_Update_Sensor_Motion();

   Set_RSPP_Host_Info(rspp_host_C, F360_Host);
   Set_RSPP_Core_Info(rspp_core_info_C, F360_Core_Info);

   rspp_variant_C::Inputs_Preprocessing(rspp_core_info_C, rspp_host_C,
                                        *reinterpret_cast<rspp_variant_C::F360_Radar_Sensor_T(*)[4]>(&F360_Sensors), rspp_calibs_C,
                                        detection_list_C);

   /* Check for successful initialization before running tracker*/
   if (Update_Look_Index_Available_Flag() == true)
   // run tracker only if all look data for all sensors available, running tacker withpout it makes it reset
   {
      /* Run F360 radar tracker */
      SM_C.execute(F360_Core_Info, F360_Host, detection_list_C, F360_Sensors, *Object_Log);
   }

   /* Update F360 logging */
   f360_tracker.Log_Objects(*Object_Log);
   f360_tracker.Log_Detections(&F360_Detection_Output, detection_list_C);
   f360_tracker.Log_Tracker_Info(F360_Tracker_Info_Log);
   f360_tracker.Log_F360_Host_Props(*F360_Host_Props);

   /* tracker fault status */
   safety_logic->evaluate_cycle(F360_Core_Info, F360_Host, F360_Sensors, *Object_Log);

   /* Populate tracker output to IPC*/
   Populate_Functional_Safety_Faults_Log_Data();
   Populate_Vehicle_Info_Log_Data();
   Populate_Sensor_Info_Log_Data();

#if ISO_OUTPUT
   /* Run f360_iso_object_list_output_adaptation */
   ISO_ObjectList_Log->object_list_timestamp = F360_Reduced_Objects_Log->object_list_timestamp;
   ISO_ObjectList_Log->tracker_index         = F360_Reduced_Objects_Log->tracker_index;
   f360_tracker.Fill_ROT_Object_Output(F360_Host, *F360_Processed_Detections_Log, ISO_ObjectList_Log->obj);
#endif
}

/*===========================================================================*\
 * Function: Update_Host_Raw
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - Veh_Info_T *p_raw_veh_data  -> Pointer to raw vehicle info from mss ipc buffer
 *
 * External References:
 *  - static F360_Host_Raw_T F360_Host_Raw
 *
 * Description:
 *    Updates the F360_Host_Raw members before running the F360 radar tracker.
 *
\*===========================================================================*/
#if F360_HIL_STUB
void Update_Host_Raw_Stub()
{
   // F360_Host_Raw->global_time_sync_s = 0.0;   Nolan: Do we need this?

   F360_Host_Raw->timestamp_s = (uint64_t)(Get_System_Time()) * US_2_S;

   F360_Host_Raw->raw_speed                = 0;
   F360_Host_Raw->speed_qf                 = 3;
   F360_Host_Raw->raw_yaw_rate_rad         = 0;
   F360_Host_Raw->yaw_rate_qf              = 3;
   F360_Host_Raw->steering_wheel_angle_rad = 0;
   F360_Host_Raw->steering_wheel_angle_qf  = 3;
   F360_Host_Raw->lat_accel_qf             = 3;
   F360_Host_Raw->long_accel_qf            = 3;

   F360_Host_Raw->prndl        = Get_f360_prndl_state(3);
   F360_Host_Raw->reverse_gear = 0;

   F360_Host.speed_correction_factor = 1.03;
}
#else

void Update_Host_Raw(Veh_Info_T *p_raw_veh_data)
{
   // F360_Host_Raw->global_time_sync_s = 0.0;   Nolan: Do we need this?

   F360_Host_Raw->timestamp_s = (Get_System_Time()) * US_2_S;

   F360_Host_Raw->raw_speed                = p_raw_veh_data->veh_speed;
   F360_Host_Raw->speed_qf                 = p_raw_veh_data->veh_speed_qf;
   F360_Host_Raw->raw_yaw_rate_rad         = p_raw_veh_data->veh_yaw;
   F360_Host_Raw->yaw_rate_qf              = p_raw_veh_data->veh_yaw_qf;
   F360_Host_Raw->steering_wheel_angle_rad = p_raw_veh_data->veh_steering_angle;
   F360_Host_Raw->steering_wheel_angle_qf  = 3;
   F360_Host_Raw->lat_accel_qf             = 3;
   F360_Host_Raw->long_accel_qf            = 3;

   if (F360_Host_Raw->raw_speed < 0.0f)
   {
      F360_Host_Raw->prndl        = Get_f360_prndl_state(2);
      F360_Host_Raw->reverse_gear = 1;
   }
   else
   {
      F360_Host_Raw->prndl        = Get_f360_prndl_state(3);
      F360_Host_Raw->reverse_gear = 0;
   }

   F360_Host.speed_correction_factor = 1.0;
}
#endif

/*===========================================================================*\
 * Function: Update_Core_Info
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  - static F360_Core_Info_T F360_Core_Info
 *
 * Description:
 *    Updates the F360_Core_Info members before running the F360 radar tracker.
 *
\*===========================================================================*/
static void Update_Core_Info(void)
{
   uint64_t time_us = (uint64_t)(Get_System_Time());

   F360_Core_Info.cnt_loops++;
   F360_Core_Info.prev_time_us   = (1 == F360_Core_Info.cnt_loops) ? time_us : F360_Core_Info.time_us;
   F360_Core_Info.time_us        = time_us;
   F360_Core_Info.elapsed_time_s = (F360_Core_Info.time_us - F360_Core_Info.prev_time_us) * US_2_S;
}

/*===========================================================================*\
 * Function: Init_Sensors_Calibrations
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - uint8_t radar_position
 *
 * External References:
 *  - static F360_Radar_Sensor_Calib_T F360_Sensors_Calib
 *
 * Description:
 *    Updates the F360_Sensors_Calib members before running the F360 radar tracker.
 *
\*===========================================================================*/
void Init_Sensors_Calibrations(uint8_t radar_position)
{
   uint8_t lookidx;
   F360_Sensors_Calib[MASTER_INDEX].id = MASTER_ID;

   Status_Stream_T *StatusLogData = Get_Status_Log_Data_Ptr();

   /* Use the mounting position from SMC */
   switch (radar_position)
   {
      case 1:
         F360_Sensors_Calib[MASTER_INDEX].mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;
         break;
      case 2:
         F360_Sensors_Calib[MASTER_INDEX].mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;
         break;
      case 3:
         F360_Sensors_Calib[MASTER_INDEX].mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
         break;
      case 4:
         F360_Sensors_Calib[MASTER_INDEX].mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD;
         break;
      case 5:
         F360_Sensors_Calib[MASTER_INDEX].mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
         break;
      default:
         F360_Sensors_Calib[MASTER_INDEX].mounting_location = F360_MOUNTING_LOCATION_UNKNOWN;
         break;
   }
   switch (K_PLATFORM_VARIANT)
   {
      case 72:
         F360_Sensors_Calib[MASTER_INDEX].sensor_type = F360_SENSOR_TYPE_SRR6_PLUS_RADAR;
         break;
      case 73:
         F360_Sensors_Calib[MASTER_INDEX].sensor_type = F360_SENSOR_TYPE_FLR4_RADAR;
         break;
      default:
         F360_Sensors_Calib[MASTER_INDEX].sensor_type = F360_SENSOR_TYPE_UNKNOWN;
         break;
   }

   F360_Sensors_Calib[MASTER_INDEX].mounting_position.vcs_position.longitudinal = // -0.20F;
      StatusLogData->sensor_mount_pos_x; // k_sensor_mount_pos_x_list_SMC[radar_position - 1];                     // -
                                         // DIST_FRONT_2_REAR_AXLE;  /* SMC
                                         // Position in SAE*/
   F360_Sensors_Calib[MASTER_INDEX].mounting_position.vcs_position.lateral = // 0.66F;
      StatusLogData->sensor_mount_pos_y; //  k_sensor_mount_pos_y_list_SMC[radar_position - 1];                     // *
                                         //  NEG_FLOAT;   /* SMC Position
                                         //  in SAE*/
   F360_Sensors_Calib[MASTER_INDEX].mounting_position.vcs_position.height = // 0.54F;
      StatusLogData
         ->sensor_height; // k_sensor_mount_height_list_SMC[radar_position - 1];                    /* SMC Position in SAE*/
   F360_Sensors_Calib[MASTER_INDEX].mounting_position.vcs_boresight_azimuth_angle = // 0.872665F;
      StatusLogData->sensor_mount_ornt_yaw; // NEG_FLOAT * k_sensor_mount_ornt_yaw_list_SMC[radar_position - 1];             /*
                                            // SMC Angles in ISO but
                                            // tracker input in SAE*/
   F360_Sensors_Calib[MASTER_INDEX].mounting_position.vcs_boresight_elevation_angle = 0.0F;
   // NEG_FLOAT * k_sensor_mount_ornt_pitch_list_SMC[radar_position - 1]; /* SMC Angles in ISO but tracker input
   // in SAE*/
   if (k_sensor_mount_ornt_roll_list_SMC[radar_position - 1] < 0.1F)
   {
      F360_Sensors_Calib[MASTER_INDEX].polarity = 1;
   }
   else
   {
      F360_Sensors_Calib[MASTER_INDEX].polarity = -1;
   }
   F360_Sensors_Calib[MASTER_INDEX].polarity = StatusLogData->sensor_polarity; //-1;
   F360_Sensors_Calib[MASTER_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_0] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_0]), s10p21_T);
   F360_Sensors_Calib[MASTER_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_1] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_1]), s10p21_T);
   F360_Sensors_Calib[MASTER_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_2] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_2]), s10p21_T);
   F360_Sensors_Calib[MASTER_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_3] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_3]), s10p21_T);

   for (lookidx = 0; lookidx < (uint8_t)F360_DET_NUM_LOOK_ID; lookidx++)
   {
      F360_Sensors_Calib[MASTER_INDEX].min_aliaised_range_rate[lookidx] = (float)Fix2Float(k_rdotmin_SMC, s10p21_T);
      F360_Sensors_Calib[MASTER_INDEX].fov_min_az_rad[lookidx]          = (float)Fix2Float(k_fov_hor_begin_SMC, s2p13_T);
      F360_Sensors_Calib[MASTER_INDEX].fov_max_az_rad[lookidx]          = (float)Fix2Float(k_fov_hor_end_SMC, s2p13_T);
      F360_Sensors_Calib[MASTER_INDEX].fov_min_el_rad[lookidx]          = (float)Fix2Float(k_fov_ver_begin_SMC, s2p13_T);
      F360_Sensors_Calib[MASTER_INDEX].fov_max_el_rad[lookidx]          = (float)Fix2Float(k_fov_ver_end_SMC, s2p13_T);
   }

   /* Updated to use SMC values */
   F360_Sensors_Calib[MASTER_INDEX].range_limits[F360_LOOK_ID_0] = // 248.3793;
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_0]), u9p7_T);
   F360_Sensors_Calib[MASTER_INDEX].range_limits[F360_LOOK_ID_1] = // 251.6051;
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_1]), u9p7_T);
   F360_Sensors_Calib[MASTER_INDEX].range_limits[F360_LOOK_ID_2] = // 248.3793;
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_2]), u9p7_T);
   F360_Sensors_Calib[MASTER_INDEX].range_limits[F360_LOOK_ID_3] = // 251.6051;
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_3]), u9p7_T);

   F360_Sensors_Calib[MASTER_INDEX].f_ant_sens_available = false;
   F360_Sensors_Calib[MASTER_INDEX].f_ant_sens_degraded  = false;
   F360_Sensors_Calib[MASTER_INDEX].is_valid             = true;

#if defined(ROT_FUSION_MASTER)
   uint8_t radar_position_slave;
   F360_Sensors_Calib[SLAVE_INDEX].id = SLAVE_ID;

   if (radar_position > 1)
   {
      radar_position_slave = radar_position - 1;
   }
   else
   {
      radar_position_slave = radar_position + 1;
   }

   /* Use the mounting position from SMC */
   switch (radar_position_slave)
   {
      case 1:
         F360_Sensors_Calib[SLAVE_INDEX].mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;
         break;
      case 2:
         F360_Sensors_Calib[SLAVE_INDEX].mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;
         break;
      case 3:
         F360_Sensors_Calib[SLAVE_INDEX].mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD;
         break;
      case 4:
         F360_Sensors_Calib[SLAVE_INDEX].mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD;
         break;
      case 5:
         F360_Sensors_Calib[SLAVE_INDEX].mounting_location = F360_MOUNTING_LOCATION_CENTER_FORWARD;
         break;
      default:
         F360_Sensors_Calib[SLAVE_INDEX].mounting_location = F360_MOUNTING_LOCATION_UNKNOWN;
         break;
   }
   switch (K_PLATFORM_VARIANT)
   {
      case 72:
         F360_Sensors_Calib[SLAVE_INDEX].sensor_type = F360_SENSOR_TYPE_SRR6_PLUS_RADAR;
         break;
      case 73:
         F360_Sensors_Calib[SLAVE_INDEX].sensor_type = F360_SENSOR_TYPE_FLR4_RADAR;
         break;
      default:
         F360_Sensors_Calib[SLAVE_INDEX].sensor_type = F360_SENSOR_TYPE_UNKNOWN;
         break;
   }
   F360_Sensors_Calib[SLAVE_INDEX].mounting_position.vcs_position.longitudinal = //-0.20F;
      k_sensor_mount_pos_x_list_SMC[radar_position_slave - 1];              //- DIST_FRONT_2_REAR_AXLE;  /* SMC Position in SAE*/
   F360_Sensors_Calib[SLAVE_INDEX].mounting_position.vcs_position.lateral = //-0.66F;
      k_sensor_mount_pos_y_list_SMC[radar_position_slave - 1];              /* SMC Position in SAE*/
   F360_Sensors_Calib[SLAVE_INDEX].mounting_position.vcs_position.height =  // 0.54F;
      k_sensor_mount_height_list_SMC[radar_position_slave - 1];             /* SMC Position in SAE*/
   F360_Sensors_Calib[SLAVE_INDEX].mounting_position.vcs_boresight_azimuth_angle = //-0.872665F;
      NEG_FLOAT * k_sensor_mount_ornt_yaw_list_SMC[radar_position_slave - 1];      /* SMC Angles in ISO but tracker input in SAE*/
   F360_Sensors_Calib[SLAVE_INDEX].mounting_position.vcs_boresight_elevation_angle = // 0.0F;
      NEG_FLOAT * k_sensor_mount_ornt_pitch_list_SMC[radar_position_slave - 1]; /* SMC Angles in ISO but tracker input in SAE*/

   if (k_sensor_mount_ornt_roll_list_SMC[radar_position_slave - 1] < 0.1F)
   {
      F360_Sensors_Calib[SLAVE_INDEX].polarity = 1;
   }
   else
   {
      F360_Sensors_Calib[SLAVE_INDEX].polarity = -1;
   }
   F360_Sensors_Calib[SLAVE_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_0] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_0]), s10p21_T);
   F360_Sensors_Calib[SLAVE_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_1] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_1]), s10p21_T);
   F360_Sensors_Calib[SLAVE_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_2] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_2]), s10p21_T);
   F360_Sensors_Calib[SLAVE_INDEX].rng_rate_interval_widths[F360_DET_LOOK_ID_3] =
      NEG_FLOAT * (float)Fix2Float((k_vwrapping_SMC[F360_DET_LOOK_ID_3]), s10p21_T);

   for (lookidx = 0; lookidx < (uint8_t)F360_DET_NUM_LOOK_ID; lookidx++)
   {
      F360_Sensors_Calib[SLAVE_INDEX].min_aliaised_range_rate[lookidx] = (float)Fix2Float(k_rdotmin_SMC, s10p21_T);
      F360_Sensors_Calib[SLAVE_INDEX].fov_min_az_rad[lookidx]          = (float)Fix2Float(k_fov_hor_begin_SMC, s2p13_T);
      F360_Sensors_Calib[SLAVE_INDEX].fov_max_az_rad[lookidx]          = (float)Fix2Float(k_fov_hor_end_SMC, s2p13_T);
      F360_Sensors_Calib[SLAVE_INDEX].fov_min_el_rad[lookidx]          = (float)Fix2Float(k_fov_ver_begin_SMC, s2p13_T);
      F360_Sensors_Calib[SLAVE_INDEX].fov_max_el_rad[lookidx]          = (float)Fix2Float(k_fov_ver_end_SMC, s2p13_T);
   }

   /* Updated to use SMC values */
   F360_Sensors_Calib[SLAVE_INDEX].range_limits[F360_LOOK_ID_0] =
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_0]), u9p7_T);
   F360_Sensors_Calib[SLAVE_INDEX].range_limits[F360_LOOK_ID_1] =
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_1]), u9p7_T);
   F360_Sensors_Calib[SLAVE_INDEX].range_limits[F360_LOOK_ID_2] =
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_2]), u9p7_T);
   F360_Sensors_Calib[SLAVE_INDEX].range_limits[F360_LOOK_ID_3] =
      (float)Fix2Float((k_detection_range_max_SMC[F360_LOOK_ID_3]), u9p7_T);

   F360_Sensors_Calib[SLAVE_INDEX].f_ant_sens_available = false;
   F360_Sensors_Calib[SLAVE_INDEX].f_ant_sens_degraded  = false;
   F360_Sensors_Calib[SLAVE_INDEX].is_valid             = true;

#endif
}

/*===========================================================================*\
 * Function: Update_Detections
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - Detection_Stream_T *p_raw_det_data  -> Pointer to raw detection data from angle finding output
 *
 * External References:
 *  - static F360_Detection_List_T F360_Detection_List
 *  - static F360_Radar_Sensor_Calib_T F360_Sensors_Calib
 *  - static F360_Radar_Sensor_T F360_Sensors
 *
 * Description:
 *    Updates the F360_Detection_List members before running the F360 radar tracker.
 *
\*===========================================================================*/
#if F360_HIL_STUB
void Update_Detections_Stub()
{
   uint16_t i;
   uint32_t num_valid_dets = MAX_NUMBER_OF_DETECTIONS;

   (void)memset((void *)F360_Detection_List, (int)0, sizeof(F360_Detection_List));

   F360_Sensors[MASTER_INDEX].look_id      = (F360_Det_Look_ID_T)stub_look_type;
   F360_Sensors[MASTER_INDEX].timestamp_us = (uint64_t)((Get_System_Time()) - 50000);
   F360_Sensors[MASTER_INDEX].look_index   = 0;
   #if ROT_FUSION_MASTER
   F360_Sensors[SLAVE_INDEX].look_id      = (F360_Det_Look_ID_T)stub_look_type;
   F360_Sensors[SLAVE_INDEX].timestamp_us = (uint64_t)((Get_System_Time()) - 50000);
   F360_Sensors[SLAVE_INDEX].look_index   = 0;
   #endif

   // Update static amp_inc every cycle
   amp_inc++;
   if (amp_inc > 80)
   {
      amp_inc = 0;
   }

   for (i = 0; i < (uint16_t)num_valid_dets; i++)
   {
      int set_sensor_id = MASTER_ID;
      float set_range_rate;
      float set_azimuth;
      uint8_t det_mod = i % 4;
      switch (det_mod)
      {
         case 0:
            set_range_rate = 5.0000;
            set_azimuth    = -0.977384;
            break;
         case 1:
            set_range_rate = -10.00;
            set_azimuth    = -0.523599;
            break;
         case 2:
            set_range_rate = -12.6577;
            set_azimuth    = -0.0872665;
            break;
         case 3:
            set_range_rate = -24.1481;
            set_azimuth    = 0.698132;
      }

   #if ROT_FUSION_MASTER
      if (det_mod == 2 || det_mod == 3)
         set_sensor_id = SLAVE_ID;
   #endif

      F360_Detection_List->detections[i].sensor_id          = (F360_SI32N_T)set_sensor_id;
      F360_Detection_List->detections[i].det_id             = i + 1;
      F360_Detection_List->detections[i].range              = (i + 1) % 36;
      F360_Detection_List->detections[i].range_rate         = set_range_rate;
      F360_Detection_List->detections[i].azimuth_raw        = set_azimuth;
      F360_Detection_List->detections[i].azimuth            = set_azimuth;
      F360_Detection_List->detections[i].confid_azimuth     = 0;
      F360_Detection_List->detections[i].elevation_raw      = (float)0.0;
      F360_Detection_List->detections[i].elevation          = (float)0.0;
      F360_Detection_List->detections[i].confid_elevation   = 0;
      F360_Detection_List->detections[i].snr                = (float)0.0;
      F360_Detection_List->detections[i].amplitude          = 0.5 * amp_inc;
      F360_Detection_List->detections[i].f_super_res        = 0;
      F360_Detection_List->detections[i].f_host_veh_clutter = 0;
   }

   F360_Detection_List->number_of_valid_detections = num_valid_dets;
   #if ROT_FUSION_MASTER
   F360_Sensors[MASTER_INDEX].number_of_valid_detections = num_valid_dets - (num_valid_dets / 2);
   F360_Sensors[SLAVE_INDEX].number_of_valid_detections  = num_valid_dets / 2;
   #else
   F360_Sensors[MASTER_INDEX].number_of_valid_detections = num_valid_dets;
   #endif
   stub_look_type = stub_look_type < 3 ? stub_look_type + 1 : 0;
}

#else
   #if defined(ROT_STANDALONE)
void Update_Detections(D2M_Payload_T *p_det_master_data)
{
   uint16_t i;
   uint16_t num_valid_dets = (p_det_master_data->det_data.af_data.num_af_det < MAX_NUMBER_OF_DETECTIONS) ?
                                p_det_master_data->det_data.af_data.num_af_det :
                                MAX_NUMBER_OF_DETECTIONS;

   /* Calculate the elapsed time (in seconds) since the middle of the dwell time.
    * We will subtract this from the current time to provide the tracker with a more
    * accurate detection time stamp. */

   F360_Sensors[MASTER_INDEX].look_id = (F360_Det_Look_ID_T)p_det_master_data->det_data.look_type;
   F360_Sensors[MASTER_INDEX].timestamp_us =
      (uint64_t)((Get_System_Time()) - 40000); // Currently time stamp is not implemented // Todo: need time stamp from detection
   F360_Sensors[MASTER_INDEX].look_index             = p_det_master_data->det_data.lookindex;
   F360_Sensors[MASTER_INDEX].vehicle_speed_calc_mps = F360_Host_Raw->raw_speed;
   F360_Sensors[MASTER_INDEX].yaw_rate_calc_dps      = F360_Host_Raw->raw_yaw_rate_rad * RAD2DEGF;
   Align_Angle_Az_Rad[MASTER_INDEX]                  = 0.0f;
   Align_Angle_El_Rad[MASTER_INDEX]                  = 0.0f;

   for (i = 0; i < num_valid_dets; i++)
   {
      F360_Detection_List->detections[i].sensor_id = (F360_SI32N_T)MASTER_ID;
      F360_Detection_List->detections[i].det_id    = i + 1;

      F360_Detection_List->detections[i].range     = p_det_master_data->det_data.af_data.ran[i];
      F360_Detection_List->detections[i].std_range = p_det_master_data->det_data.af_data.std_ran[i];

      F360_Detection_List->detections[i].range_rate     = p_det_master_data->det_data.af_data.vel[i];
      F360_Detection_List->detections[i].std_range_rate = p_det_master_data->det_data.af_data.std_vel[i];

      F360_Detection_List->detections[i].azimuth_raw = p_det_master_data->det_data.af_data.theta[i];
      F360_Detection_List->detections[i].azimuth =
         F360_Detection_List->detections[i].azimuth_raw * (F360_Sensors_Calib[MASTER_INDEX].polarity);
      F360_Detection_List->detections[i].confid_azimuth = (F360_SI8N_T)p_det_master_data->det_data.af_data.az_conf[i];
      F360_Detection_List->detections[i].std_azimuth    = p_det_master_data->det_data.af_data.std_theta[i];

      F360_Detection_List->detections[i].elevation_raw = p_det_master_data->det_data.af_data.phi[i];
      F360_Detection_List->detections[i].elevation =
         F360_Detection_List->detections[i].elevation_raw * (F360_Sensors_Calib[MASTER_INDEX].polarity);
      F360_Detection_List->detections[i].confid_elevation = (F360_SI8N_T)p_det_master_data->det_data.af_data.el_conf[i];
      F360_Detection_List->detections[i].std_elevation    = p_det_master_data->det_data.af_data.std_phi[i];

      F360_Detection_List->detections[i].snr                = p_det_master_data->det_data.af_data.snr[i];
      F360_Detection_List->detections[i].amplitude          = p_det_master_data->det_data.af_data.rcs[i];
      F360_Detection_List->detections[i].f_super_res        = p_det_master_data->det_data.af_data.f_superres_target[i];
      F360_Detection_List->detections[i].f_bistatic         = p_det_master_data->det_data.af_data.f_bistatic[i];
      F360_Detection_List->detections[i].f_host_veh_clutter = 0;
   }

   F360_Detection_List->number_of_valid_detections       = num_valid_dets;
   F360_Sensors[MASTER_INDEX].number_of_valid_detections = num_valid_dets;
}

   #elif defined(ROT_FUSION_MASTER)
void Update_Detections(D2M_Payload_T *p_det_master_data, AF_Det_Slave_T *p_det_slave_data, GLOB_TS_STRUCT_TYPE *p_slave_det_ts,
                       GLOB_TS_STRUCT_TYPE *p_master_det_ts)
{
   /* MASTER SENSOR */
   uint8_t num_valid_dets_master = 0;

   F360_Sensors[MASTER_INDEX].look_id      = (F360_Det_Look_ID_T)p_det_master_data->det_data.look_type;
   F360_Sensors[MASTER_INDEX].timestamp_us = (uint64_t)((Get_System_Time()) - 40000);
   //    (uint64_t)((p_master_det_ts->ts_secs * 1000000) + (p_master_det_ts->ts_nanosecs * 0.001));
   F360_Sensors[MASTER_INDEX].look_index             = p_det_master_data->det_data.lookindex;
   F360_Sensors[MASTER_INDEX].vehicle_speed_calc_mps = F360_Host_Raw->raw_speed;
   F360_Sensors[MASTER_INDEX].yaw_rate_calc_dps      = F360_Host_Raw->raw_yaw_rate_rad * RAD2DEGF;
   Align_Angle_Az_Rad[MASTER_INDEX]                  = 0.0f;
   Align_Angle_El_Rad[MASTER_INDEX]                  = 0.0f;

   if (enable_master_det)
   {
      num_valid_dets_master = (p_det_master_data->downselection_data.af_ds_data.num_af_ds_det < NUMBER_OF_MRR_DETECTIONS) ?
                                 p_det_master_data->downselection_data.af_ds_data.num_af_ds_det :
                                 NUMBER_OF_MRR_DETECTIONS;

      /* Calculate the elapsed time (in seconds) since the middle of the dwell time.
       * We will subtract this from the current time to provide the tracker with a more
       * accurate detection time stamp. */

      (void)memset((void *)F360_Detection_List, (int)0, sizeof(F360_Detection_List));

      for (uint8_t i = 0; i < num_valid_dets_master; i++)
      {
         F360_Detection_List->detections[i].sensor_id = (F360_SI32N_T)MASTER_ID;
         F360_Detection_List->detections[i].det_id    = i + 1;

         F360_Detection_List->detections[i].range     = p_det_master_data->downselection_data.af_ds_data.ran[i];
         F360_Detection_List->detections[i].std_range = p_det_master_data->downselection_data.af_ds_data.std_ran[i];

         F360_Detection_List->detections[i].range_rate     = p_det_master_data->downselection_data.af_ds_data.vel[i];
         F360_Detection_List->detections[i].std_range_rate = p_det_master_data->downselection_data.af_ds_data.std_vel[i];

         F360_Detection_List->detections[i].azimuth_raw = p_det_master_data->downselection_data.af_ds_data.theta[i];
         F360_Detection_List->detections[i].azimuth =
            F360_Detection_List->detections[i].azimuth_raw * (F360_Sensors_Calib[MASTER_INDEX].polarity);
         F360_Detection_List->detections[i].confid_azimuth =
            (F360_SI8N_T)p_det_master_data->downselection_data.af_ds_data.az_conf[i];
         F360_Detection_List->detections[i].std_azimuth = p_det_master_data->downselection_data.af_ds_data.std_theta[i];

         F360_Detection_List->detections[i].elevation_raw = p_det_master_data->downselection_data.af_ds_data.phi[i];
         F360_Detection_List->detections[i].elevation =
            F360_Detection_List->detections[i].elevation_raw * (F360_Sensors_Calib[MASTER_INDEX].polarity);
         F360_Detection_List->detections[i].confid_elevation =
            (F360_SI8N_T)p_det_master_data->downselection_data.af_ds_data.el_conf[i];
         F360_Detection_List->detections[i].std_elevation = p_det_master_data->downselection_data.af_ds_data.std_phi[i];

         F360_Detection_List->detections[i].snr         = p_det_master_data->downselection_data.af_ds_data.snr[i];
         F360_Detection_List->detections[i].amplitude   = p_det_master_data->downselection_data.af_ds_data.rcs[i];
         F360_Detection_List->detections[i].f_super_res = p_det_master_data->downselection_data.af_ds_data.f_superres_target[i];
         F360_Detection_List->detections[i].f_bistatic  = p_det_master_data->downselection_data.af_ds_data.f_bistatic[i];
         F360_Detection_List->detections[i].f_host_veh_clutter = 0;
      }

   } /* Debug for f360 master detections*/

   F360_Detection_List->number_of_valid_detections       = num_valid_dets_master;
   F360_Sensors[MASTER_INDEX].number_of_valid_detections = num_valid_dets_master;

   /* SLAVE SENSOR */

   uint8_t num_valid_dets_slave =
      (p_det_slave_data->num_af_det < NUMBER_OF_MRR_DETECTIONS) ? p_det_slave_data->num_af_det : NUMBER_OF_MRR_DETECTIONS;

   /* Calculate the elapsed time (in seconds) since the middle of the dwell time.
    * We will subtract this from the current time to provide the tracker with a more
    * accurate detection time stamp. */

   F360_Sensors[SLAVE_INDEX].look_id      = (F360_Det_Look_ID_T)p_det_slave_data->slave_look_type;
   F360_Sensors[SLAVE_INDEX].timestamp_us = (uint64_t)((Get_System_Time()) - 50000);
   //    (uint64_t)((p_slave_det_ts->ts_secs * 1000000) + (p_slave_det_ts->ts_nanosecs * 0.001));
   F360_Sensors[SLAVE_INDEX].look_index             = p_det_slave_data->slave_scan_index;
   F360_Sensors[SLAVE_INDEX].vehicle_speed_calc_mps = F360_Host_Raw->raw_speed;
   F360_Sensors[SLAVE_INDEX].yaw_rate_calc_dps      = F360_Host_Raw->raw_yaw_rate_rad * RAD2DEGF;
   Align_Angle_Az_Rad[SLAVE_INDEX]                  = 0.0f;
   Align_Angle_El_Rad[SLAVE_INDEX]                  = 0.0f;

   for (uint8_t j = num_valid_dets_master; j < (num_valid_dets_slave + num_valid_dets_master); j++)
   {
      uint8_t t                                    = j - num_valid_dets_master;
      F360_Detection_List->detections[j].sensor_id = (F360_SI32N_T)SLAVE_ID;
      F360_Detection_List->detections[j].det_id    = j + 1;

      F360_Detection_List->detections[j].range     = p_det_slave_data->ran[t];
      F360_Detection_List->detections[j].std_range = p_det_slave_data->std_ran[t];

      F360_Detection_List->detections[j].range_rate     = p_det_slave_data->vel[t];
      F360_Detection_List->detections[j].std_range_rate = p_det_slave_data->std_vel[t];

      F360_Detection_List->detections[j].azimuth_raw = p_det_slave_data->theta[t];
      F360_Detection_List->detections[j].azimuth =
         F360_Detection_List->detections[j].azimuth_raw * (F360_Sensors_Calib[SLAVE_INDEX].polarity);
      F360_Detection_List->detections[j].confid_azimuth = (F360_SI8N_T)p_det_slave_data->az_conf[t];
      F360_Detection_List->detections[j].std_azimuth    = p_det_slave_data->std_theta[t];

      F360_Detection_List->detections[j].elevation_raw = p_det_slave_data->phi[t];
      F360_Detection_List->detections[j].elevation =
         F360_Detection_List->detections[j].elevation_raw * (F360_Sensors_Calib[SLAVE_INDEX].polarity);
      F360_Detection_List->detections[j].confid_elevation = (F360_SI8N_T)p_det_slave_data->el_conf[t];
      F360_Detection_List->detections[j].std_elevation    = p_det_slave_data->std_phi[t];

      F360_Detection_List->detections[j].snr                = p_det_slave_data->snr[t];
      F360_Detection_List->detections[j].amplitude          = p_det_slave_data->rcs[t];
      F360_Detection_List->detections[j].f_super_res        = p_det_slave_data->f_superres_target[t];
      F360_Detection_List->detections[j].f_bistatic         = p_det_slave_data->f_bistatic[t];
      F360_Detection_List->detections[j].f_host_veh_clutter = 0;
   }

   F360_Detection_List->number_of_valid_detections += num_valid_dets_slave;
   F360_Sensors[SLAVE_INDEX].number_of_valid_detections = num_valid_dets_slave;
}
   #endif // ROT_FUSION_MASTER
#endif

void F360_Update_Sensor_Motion(void)
{
   float xsens   = 0.0f;
   float ysens   = 0.0f;
   float xdotvcs = 0.0f;
   float ydotvcs = 0.0f;
   float rear_sideslip;
   float cb = 0.0f;
   float sb = 0.0f;

   rear_sideslip = -(F360_Host_Calib.rear_cornering_compliance) * (F360_Host.curvature_rear * F360_Host.speed * F360_Host.speed);
   cb            = cosf(rear_sideslip);
   sb            = sinf(rear_sideslip);

   for (int eSens = 0; eSens < MAX_NUMBER_OF_SENSORS; eSens++)
   {
      /* Sensor position relative to center of rear axle (in VCS-aligned coordinates) */
      xsens = F360_Host.dist_rear_axle_to_vcs_m + F360_Sensors_Calib[eSens].mounting_position.vcs_position.longitudinal;
      ysens = F360_Sensors_Calib[eSens].mounting_position.vcs_position.lateral;

      /* VCS components of OTG velocity of sensor */
      xdotvcs = (F360_Host.speed * cb) - (F360_Host.yaw_rate_rad * ysens);
      ydotvcs = (F360_Host.speed * sb) + (F360_Host.yaw_rate_rad * xsens);

      F360_Sensors[eSens].variable.vcs_velocity.longitudinal = xdotvcs;
      F360_Sensors[eSens].variable.vcs_velocity.lateral      = ydotvcs;
   }
}

/*===========================================================================*\
 * Function: Update_Look_Index_Available_Flag
 *===========================================================================
 * Return Value:
 *  - bool radar_calib_valid_all_look_index
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  -
 *
 * Description:
 *    Updates the radar_calib_valid_all_look_index representing all rng_rate_interval_widths
 *    values for all look indexs for radars are available and valid.
 *
\*===========================================================================*/
bool Update_Look_Index_Available_Flag(void)
{
   bool calib_valid_all_look_index = true;
   for (int i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      if ((F360_Sensors_Calib[i].rng_rate_interval_widths[0] < 0.001f) ||
          (F360_Sensors_Calib[i].rng_rate_interval_widths[1] < 0.001f) ||
          (F360_Sensors_Calib[i].rng_rate_interval_widths[2] < 0.001f) ||
          (F360_Sensors_Calib[i].rng_rate_interval_widths[3] < 0.001f))
      {
         calib_valid_all_look_index &= false;
         return calib_valid_all_look_index;
      }
      else
      {
         calib_valid_all_look_index &= true;
      }
   }
   return true;
}

/*===========================================================================*\
 * Function: Populate_Functional_Safety_Faults_Log_Data
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  - static F360_Functional_Safety_Faults_Log
 *
 * Description:
 *   Populates F360_Functional_Safety_Faults_Log
 *
\*===========================================================================*/
void Populate_Functional_Safety_Faults_Log_Data(void)
{
   // input diagnostics faults logging
   const Input_Faults_T &input_faults = safety_logic->get_input_status();
   F360_Functional_Safety_Faults_Log->input_faults.core_info.cnt_loops_no_increase =
      static_cast<uint8_t>(input_faults.core_info.cnt_loops_no_increase);
   F360_Functional_Safety_Faults_Log->input_faults.core_info.elapsed_time_above_upper_limit =
      static_cast<uint8_t>(input_faults.core_info.elapsed_time_above_upper_limit);
   F360_Functional_Safety_Faults_Log->input_faults.core_info.elapsed_time_below_lower_limit =
      static_cast<uint8_t>(input_faults.core_info.elapsed_time_below_lower_limit);
   F360_Functional_Safety_Faults_Log->input_faults.core_info.time_us_no_increase =
      static_cast<uint8_t>(input_faults.core_info.time_us_no_increase);
   F360_Functional_Safety_Faults_Log->input_faults.host_info.vehicle_index_no_increase =
      static_cast<uint8_t>(input_faults.host_info.vehicle_index_no_increase);
   F360_Functional_Safety_Faults_Log->input_faults.host_info.host_speed_invalid =
      static_cast<uint8_t>(input_faults.host_info.host_speed_invalid);
   F360_Functional_Safety_Faults_Log->input_faults.host_info.host_yawrate_invalid =
      static_cast<uint8_t>(input_faults.host_info.host_yawrate_invalid);
   F360_Functional_Safety_Faults_Log->input_faults.host_info.host_longitudinal_acceleration_invalid =
      static_cast<uint8_t>(input_faults.host_info.host_longitudinal_acceleration_invalid);
   F360_Functional_Safety_Faults_Log->input_faults.host_info.host_lateral_acceleration_invalid =
      static_cast<uint8_t>(input_faults.host_info.host_lateral_acceleration_invalid);
   for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      F360_Functional_Safety_Faults_Log->input_faults.sensors[i].look_index_no_increase =
         static_cast<uint8_t>(input_faults.sensors[i].look_index_no_increase);
      F360_Functional_Safety_Faults_Log->input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence =
         static_cast<uint8_t>(input_faults.sensors[i].sensor_vs_tracker_timestamp_divergence);
      F360_Functional_Safety_Faults_Log->input_faults.sensors_calibs[i].mounting_pos_is_invalid =
         static_cast<uint8_t>(input_faults.sensors_calibs[i].mounting_pos_is_invalid);
      F360_Functional_Safety_Faults_Log->input_faults.sensors_calibs[i].polarity_is_invalid =
         static_cast<uint8_t>(input_faults.sensors_calibs[i].polarity_is_invalid);
      F360_Functional_Safety_Faults_Log->input_faults.sensors_calibs[i].boresight_angle_is_invalid =
         static_cast<uint8_t>(input_faults.sensors_calibs[i].boresight_angle_is_invalid);
   }

   // output diagnostics faults logging
   const Output_Faults_T &output_faults = safety_logic->get_output_status();
   F360_Functional_Safety_Faults_Log->output_faults.f_track_accelerations_faulty =
      static_cast<uint8_t>(output_faults.f_track_accelerations_faulty);
   F360_Functional_Safety_Faults_Log->output_faults.f_track_positions_faulty =
      static_cast<uint8_t>(output_faults.f_track_positions_faulty);
   F360_Functional_Safety_Faults_Log->output_faults.f_track_velocities_faulty =
      static_cast<uint8_t>(output_faults.f_track_velocities_faulty);

   // Tracker fault status and tracker reset flag logging
   const SafetyControlLogic::SCL_Output_T &scl_faults = safety_logic->get_scl_status();
   F360_Functional_Safety_Faults_Log->scl_output_faults.core_info_fault_status =
      static_cast<uint8_t>(scl_faults.core_info_fault_status);
   F360_Functional_Safety_Faults_Log->scl_output_faults.host_info_fault_status =
      static_cast<uint8_t>(scl_faults.host_info_fault_status);
   for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      F360_Functional_Safety_Faults_Log->scl_output_faults.sensors_calibs_fault_status[i] =
         static_cast<uint8_t>(scl_faults.sensors_calibs_fault_status[i]);
      F360_Functional_Safety_Faults_Log->scl_output_faults.sensors_fault_status[i] =
         static_cast<uint8_t>(scl_faults.sensors_fault_status[i]);
   }
   F360_Functional_Safety_Faults_Log->scl_output_faults.object_track_fault_status =
      static_cast<uint8_t>(scl_faults.object_track_fault_status);
   F360_Functional_Safety_Faults_Log->scl_output_faults.overall_fault_status =
      static_cast<uint8_t>(scl_faults.overall_fault_status);
   F360_Functional_Safety_Faults_Log->scl_output_faults.should_reset = static_cast<uint8_t>(scl_faults.should_reset);
}

/*===========================================================================*\
 * Function: Populate_Vehicle_Info_Log_Data
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  - static Vehicle_Info_Log_Data
 *
 * Description:
 *   Populates Vehicle_Info_Log_Data
 *
\*===========================================================================*/
void Populate_Vehicle_Info_Log_Data(void)
{
   // Time
   F360_Vehicle_Info_Log->vehicle_index      = static_cast<int32_t>(F360_Host.vehicle_index);
   F360_Vehicle_Info_Log->global_time_sync_s = F360_Host_Raw->global_time_sync_s;
   F360_Vehicle_Info_Log->timestamp_s        = F360_Host_Raw->timestamp_s;

   // (void)memcpy(&F360_Vehicle_Info_Log->align_angle_az_rad, Align_Angle_Az_Rad,
   // sizeof(F360_Vehicle_Info_Log->align_angle_az_rad)); (void)memcpy(&F360_Vehicle_Info_Log->align_angle_el_rad,
   // Align_Angle_El_Rad, sizeof(F360_Vehicle_Info_Log->align_angle_el_rad));

   F360_Vehicle_Info_Log->speed_correction_factor = F360_Host.speed_correction_factor;

   // Host props
   F360_Vehicle_Info_Log->world_x          = F360_Host_Props->position_x;
   F360_Vehicle_Info_Log->world_y          = F360_Host_Props->position_y;
   F360_Vehicle_Info_Log->heading          = F360_Host_Props->heading;
   F360_Vehicle_Info_Log->delta_pointing   = F360_Host_Props->delta_pointing;
   F360_Vehicle_Info_Log->delta_position_x = F360_Host_Props->delta_position_x;
   F360_Vehicle_Info_Log->delta_position_y = F360_Host_Props->delta_position_y;

   // Host
   F360_Vehicle_Info_Log->speed                     = F360_Host.speed;
   F360_Vehicle_Info_Log->acceleration              = F360_Host.acceleration;
   F360_Vehicle_Info_Log->curvature_rear            = F360_Host.curvature_rear;
   F360_Vehicle_Info_Log->vcs_speed                 = F360_Host.vcs_speed;
   F360_Vehicle_Info_Log->vcs_lat_acceleration      = F360_Host.vcs_lat_acceleration;
   F360_Vehicle_Info_Log->vcs_long_acceleration     = F360_Host.vcs_long_acceleration;
   F360_Vehicle_Info_Log->vcs_sideslip              = F360_Host.vcs_sideslip;
   F360_Vehicle_Info_Log->rear_cornering_compliance = F360_Host.rear_cornering_compliance;

   // Host raw
   F360_Vehicle_Info_Log->raw_speed               = F360_Host_Raw->raw_speed;
   F360_Vehicle_Info_Log->steering_angle_rad      = F360_Host_Raw->steering_wheel_angle_rad;
   F360_Vehicle_Info_Log->raw_yaw_rate_rad        = F360_Host_Raw->raw_yaw_rate_rad;
   F360_Vehicle_Info_Log->dist_rear_axle_to_vcs_m = F360_Host.dist_rear_axle_to_vcs_m;
   F360_Vehicle_Info_Log->prndl                   = static_cast<uint8_t>(F360_Host_Raw->prndl);
   F360_Vehicle_Info_Log->f_reverse_gear          = F360_Host_Raw->reverse_gear;
   // F360_Vehicle_Info_Log->f_trailer_present       = F360_Host_Raw->f_trailer_present;
}

/*===========================================================================*\
 * Function: Populate_Sensor_Info_Log_Data
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  - static Sensor_Info_Log_Data
 *
 * Description:
 *   Populates Sensor_Info_Log_Data
 *
\*===========================================================================*/
void Populate_Sensor_Info_Log_Data(void)
{
   for (uint8_t i = 0; i < MAX_NUMBER_OF_SENSORS; i++)
   {
      F360_Processed_Detections_Log->sensorInfoLog[i].timestamp_us   = F360_Sensors[i].variable.timestamp_us;
      F360_Processed_Detections_Log->sensorInfoLog[i].look_id        = (int8_t)F360_Sensors[i].variable.look_id;
      F360_Processed_Detections_Log->sensorInfoLog[i].radar_polarity = (int8_t)F360_Sensors_Calib[i].polarity;
      F360_Processed_Detections_Log->sensorInfoLog[i].mount_location = (int8_t)F360_Sensors_Calib[i].mounting_location;
      F360_Processed_Detections_Log->sensorInfoLog[i].f_sens_valid   = F360_Sensors_Calib[i].is_valid;
      F360_Processed_Detections_Log->sensorInfoLog[i].look_index     = F360_Sensors[i].variable.look_index;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_lat_posn = F360_Sensors_Calib[i].mounting_position.vcs_position.lateral;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_long_posn =
         F360_Sensors_Calib[i].mounting_position.vcs_position.longitudinal;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_lat_vel  = F360_Sensors[i].variable.vcs_velocity.lateral;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_long_vel = F360_Sensors[i].variable.vcs_velocity.longitudinal;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_height_offset_m =
         F360_Sensors_Calib[i].mounting_position.vcs_position.height;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_boresight_az_angle =
         F360_Sensors_Calib[i].mounting_position.vcs_boresight_azimuth_angle;
      F360_Processed_Detections_Log->sensorInfoLog[i].vcs_boresight_elev_angle =
         F360_Sensors_Calib[i].mounting_position.vcs_boresight_elevation_angle;
      F360_Processed_Detections_Log->sensorInfoLog[i].useful_fov[0] =
         F360_Sensors_Calib[i].fov_max_az_rad[0] - F360_Sensors_Calib[i].fov_min_az_rad[0];
      if ((int8_t)F360_Sensors[i].variable.look_id >= 0)
      {
         F360_Processed_Detections_Log->sensorInfoLog[i].range_rate_interval_width =
            F360_Sensors_Calib[i].rng_rate_interval_widths[(int8_t)F360_Sensors[i].variable.look_id];
      }
      F360_Processed_Detections_Log->sensorInfoLog[i].align_angle_az_rad = 0.0f;
      F360_Processed_Detections_Log->sensorInfoLog[i].align_angle_el_rad = 0.0f;
      F360_Processed_Detections_Log->sensorInfoLog[i].sensorID           = F360_Sensors_Calib[i].id;
      // F360_Processed_Detections_Log->sensorInfoLog[i].new_measurement_update = !F360_Sensors[i].f_no_input_received;
      F360_Processed_Detections_Log->sensorInfoLog[i].sensor_type = F360_Sensors_Calib[i].sensor_type;
   }
}
