/*===================================================================================*\
IMPORTANT DISCLAIMER:
This file is only intended as an example showing how to integrate the tracker, it is not to be used as is for production, 
it has not been properly tested by the tracker team.
All values used/assigned below are example values from previous programs, they are not to be reused.
It is the project integration teams responsibility to make sure that the below code is properly tested and functional.
\*===================================================================================*/


/*===================================================================================*\
 * FILE: Example_Wrapper_F360Tracker.cpp
 *====================================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *------------------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains definitions of F360 ISO Object List Output Adaptation functionalities
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*===================================================================================*/

// Inbuilt header inclusion
#include <cmath>
#include <cstring>

// User defined header inclusion
namespace f360_variant_A {}
using namespace f360_variant_A;

#include "Example_Wrapper_F360Tracker.h"
#include "Tracker_OAL.h"


#include "f360_reuse.h"
#include "f360_constants.h"
#include "T360_Types.h"

#include "f360_tracker.h"
#include "f360_input_diagnostics.h"
#include "f360_output_diagnostics.h"
#include "f360_safety_control_logic.h"
#include "State_Manager.h"

#include "f360_core_info.h"
#include "f360_host.h"
#include "f360_host_calib.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_rot_object_log.h"

#include "ocg_occupancy_grid.h"
#include "rspp_inputs_preprocessing.h"

// Tracker
static F360_Tracker g_f360_tracker{};
extern Tracker_OAL_T g_tracker_OAL{};

// Input Diagnostics
static Input_Diagnostics g_input_diagnostics{};

// Output Diagnostics
static Output_Diagnostics g_output_diagnostics{};

// Safety Control Logic
static SafetyControlLogic g_safety_control_logic(g_input_diagnostics, g_output_diagnostics);
static State_Manager g_tracker_state_manager(g_safety_control_logic, g_f360_tracker);

// Tracker data types
static F360_Core_Info_T g_core_info{};

// Host data types
static F360_Host_T g_host{};
static F360_Host_Raw_T g_host_raw{};
static F360_Host_Calib_T g_host_calib{};

// Sensor data types

static F360_Radar_Sensor_T g_sensors[MAX_NUMBER_OF_SENSORS]{};

//RSPP detection list
static rspp_variant_A::RSPP_Detection_List_T g_det_list{};
static rspp_variant_A::RSPP_Calibrations_T rspp_calibs;



// F360 Tracker Internals
static F360_Internal_Cluster_Log_Output_T internal_cluster_log;
static F360_Internal_Object_Log_Output_T internal_object_log;
static F360_Internal_Detection_History_Log_Output_T internal_det_hist_log;
static F360_Internal_Reflection_Buffer_Log_Output_T internal_reflection_log;
static F360_Internal_CWD_Log_Output_T internal_cwd_log;

// F360 Tracker Output Object list
static ROT_Object_List_Info_T rot_object_list{};

// OCG
static ocg::OCG_Inputs_T OCG_inputs{};
static ocg::Occupancy_Grid OCG_grid;

// Alignment Parameters
float g_align_angle_az_rad[MAX_NUMBER_OF_SENSORS]{}; // [rad] azimuth correction angle used to correct the azimuth angle (az = az_raw - align_angle_az_rad) (Only for logging)
float g_align_angle_el_rad[MAX_NUMBER_OF_SENSORS]{}; // [rad] elevation correction angle used to correct the elevation angle (el = el_raw - align_angle_el_rad) (Only for logging)
float g_speed_correction_factor = 1.0F;

uint8_t g_Tracker_initialized = 0;

// Antenna sensitivity maps (TODO move to calibrations later)
#define NUM_VTX_ANT_SENS_POLY 18
const float g_ant_sens_SCS_azim_deg[] = { -75.0F, -70.0F, -65.0F, -55.0F, -45.0F, -35.0F, -25.0F, -15.0F,  -5.0F,   5.0F,  15.0F,  25.0F,  35.0F,  45.0F,  55.0F,  65.0F,  70.0F,  75.0F }; // defined for FL sensor not flipped (polarity = 1) ???
const float g_ant_sens_SCS_rng_90[] = { 31.93F, 35.46F, 36.20F, 36.20F, 36.71F, 38.08F, 39.74F, 41.95F, 44.80F, 46.75F, 48.50F, 50.32F, 52.22F, 53.85F, 54.30F, 52.52F, 43.49F, 33.96F }; // border of antenna sensitivity region for a given probability of detection
const float g_ant_sens_SCS_rng_50[] = { 43.85F, 48.67F, 49.57F, 49.69F, 50.38F, 52.27F, 54.54F, 57.58F, 61.49F, 64.16F, 66.57F, 69.07F, 71.66F, 73.93F, 74.53F, 72.08F, 59.70F, 46.62F };

// Sensor motion
void F360_Update_Sensor_Motion(F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS], const F360_Host_Calib_T& r_host_calib, const F360_Host_T& r_host);

// Fill internal data
static void Set_SRR_Sensors_Cals(F360_Radar_Sensor_T(&r_sensor)[MAX_NUMBER_OF_SENSORS],
   const CAF_Param_T& CAF_Parameter, const F360_Host_Calib_T& r_host_calib);
static void Fill_Core_Info(F360_Core_Info_T& r_core_info, const uint64_t timestamp_usec);
static void Fill_Host_Info(const Ego_Vehicle_Info_T& vehicle_info, const F360_Host_Calib_T& host_calib, F360_Host_T& host, F360_Host_Raw_T& host_raw);
static void Fill_Host_Raw_Info(const Ego_Vehicle_Info_T& veh_input, F360_Host_Raw_T& host_raw);
static void Map_Host_Raw_To_Tracker_Host(const F360_Host_Raw_T& host_raw, const F360_Host_Calib_T& host_calib, F360_Host_T& host);
static void Fill_SRR_Sensors_Data(const Tracker_IAL_T& tracker_IAL, rspp_variant_A::RSPP_Detection_List_T& r_det_list, F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS]);

// Internal reflections prototypes
void F360_Check_If_Internal_Reflections_Function_Shall_Be_Enabled(const f360_variant_A::F360_Host_Calib_T& r_host_calib,
   const bool f_trailer_present, f360_variant_A::F360_Radar_Sensor_T(&r_sensor)[MAX_NUMBER_OF_SENSORS]);
static bool Is_Sensor_Facing_Host_Rear(const F360_Mounting_Location_T mounting_location);
void F360_Initialize_Internal_Reflections_Function_Tuning(const f360_variant_A::F360_Host_Calib_T& r_host_calib,
   f360_variant_A::F360_Radar_Sensor_T& r_sensor);

// Log function prototypes
static void Log_Output_Data(Tracker_OAL_T& r_tracker_OAL, const F360_Tracker r_f360_tracker,
   const F360_Core_Info_T& r_core_info, const F360_Host_T& r_host, const F360_Host_Calib_T& r_host_calib,
   F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS]);

static void F360_Log_Tracker_Internals(const F360_Tracker& r_f360_tracker, const F360_Core_Info_T& r_core_info);

bool Write_Single_UDP_Stream_Chunk(uint32_t streamRefIndex, const F360_Log_Data_T* log, uint8_t stream_number);
bool F360_Log_Variable_Tracker_Internals(const uint32_t stream_ref_idx);

// Stubbed Logging interface
typedef struct UDPRecord_Header_Tag
{
   // Application Layer Info
   uint16_t versionInfo;
   uint16_t sourceTxCnt;
   uint32_t sourceTxTime;
   uint8_t  sourceInfo;
   uint8_t  reservedSrc1;
   uint8_t  reservedSrc2;
   uint8_t  reservedSrc3;
   // Process Layer Info
   uint32_t streamRefIndex;
   uint16_t streamDataLen;
   uint8_t  streamTxCnt;
   uint8_t  streamNumber;
   uint8_t  streamVersion;
   uint8_t  streamChunks;
   uint8_t  streamChunkIdx;
   uint8_t  reservedStr3; 
}UDPRecord_Header_T;
typedef struct UDPRecord_Tag {
   UDPRecord_Header_T  header;
   void* payload;
}UDPRecord_T;

static const int UDP_RECORD_VERSION = 0xA1;
static const uint16_t UDP_RECORD_VERSIONINFO = ((UDP_RECORD_VERSION << 8) | sizeof(UDPRecord_Header_T));

static const float MAX_ALLOWED_UDP_BYTE_LIMIT = 32000.0F;
static const int32_t MAX_PAYLOAD_SIZE = 32000;
static const uint8_t T360_TRACKER_SRC_NUM = 35U;
static uint16_t udpSourceTxCnt = 0U;
static uint8_t streamTxCnt[200]{};
static void Write_F360_UDP_Stream_Chunk(
   const uint64_t sourceTxTime_us,
   const uint32_t stream_ref_index,
   const uint8_t stream_number,
   const uint8_t stream_version,
   const uint16_t chunk_size,
   const uint8_t num_of_chunks,
   const uint8_t chunk_idx,
   void* log_data_ptr)
{
   UDPRecord_T udp_record{};
   // Process Layer Info
   udp_record.header.streamRefIndex = stream_ref_index;
   udp_record.header.streamDataLen = chunk_size;
   udp_record.header.streamTxCnt = ++streamTxCnt[stream_number];
   udp_record.header.streamNumber = stream_number;
   udp_record.header.streamVersion = stream_version;
   udp_record.header.streamChunks = num_of_chunks;
   udp_record.header.streamChunkIdx = chunk_idx;
   udp_record.header.reservedStr3 = 0U;

   udp_record.payload = ((uint8_t*)log_data_ptr) + chunk_size * chunk_idx;

   // Application Layer Info
   udp_record.header.versionInfo = UDP_RECORD_VERSIONINFO;
   udp_record.header.sourceTxTime = (uint32_t)(sourceTxTime_us / 1000U); /* usec2msec */
   udp_record.header.sourceTxCnt = ++udpSourceTxCnt;
   udp_record.header.sourceInfo = T360_TRACKER_SRC_NUM;
   udp_record.header.reservedSrc2 = 0U;

   // Call function that logs the output udp_record
}
static void Write_F360_UDP_Stream(
   const uint64_t sourceTxTime_us,
   const uint32_t stream_ref_index,
   const uint8_t stream_number,
   const uint8_t stream_version,
   const uint16_t stream_size,
   void* log_data_ptr)
{
   uint32_t log_size = sizeof(F360_Log_Header_T);
   int32_t max_payload_size = MAX_PAYLOAD_SIZE;
   uint8_t num_of_chunks = 0;
   uint16_t chunk_size = 0;

   if((stream_number == F360_Objects_Log_Stream_Num) ||
      (stream_number == F360_Detection_Log_Stream_Num) ||
      (stream_number == F360_Sensor_Calib_Log_Stream_Num))
      {
         F360_Log_Data_T* dyn_log_data_ptr = static_cast<F360_Log_Data_T*>(log_data_ptr);
         switch (stream_number)
         {
            case F360_Objects_Log_Stream_Num:
               log_size += sizeof(F360_Object_Log_T) * dyn_log_data_ptr->f360header.num_elements;
               max_payload_size = F360_Objects_Log_Max_Payload_Size;
               break;
            case F360_Detection_Log_Stream_Num:
               log_size += sizeof(F360_Detection_Log_T) * dyn_log_data_ptr->f360header.num_elements;
               max_payload_size = F360_Detection_Log_Max_Payload_Size;
               break;
            case F360_Sensor_Calib_Log_Stream_Num:
               log_size += sizeof(F360_Sensor_Calib_Log_T) * dyn_log_data_ptr->f360header.num_elements;
               max_payload_size = F360_Sensor_Calib_Log_Max_Payload_Size;
               break;
            default:
               break;
         }
         num_of_chunks = static_cast<uint8_t>(log_size / max_payload_size);
         if ((log_size % max_payload_size) > 0)
         {
            num_of_chunks++;
         }
          for (uint8_t chunk_idx = 0; chunk_idx < num_of_chunks; chunk_idx++)
         {
            chunk_size = static_cast<uint16_t>(max_payload_size);
            if (chunk_idx == (num_of_chunks - 1))
            {
               /* Last chunk, need to calculate required size */
               chunk_size = static_cast<uint16_t>(log_size - (max_payload_size * chunk_idx));
            }
            Write_F360_UDP_Stream_Chunk(sourceTxTime_us, stream_ref_index, stream_number, stream_version, chunk_size, num_of_chunks, chunk_idx, log_data_ptr);
         }
   }
   else{
      float num_of_chunks_flt = ceilf(static_cast<float>(stream_size) / MAX_ALLOWED_UDP_BYTE_LIMIT);
      num_of_chunks = static_cast<uint8_t>(num_of_chunks_flt + 0.5f);
      chunk_size = static_cast<uint16_t>(ceilf(static_cast<float>(stream_size) / num_of_chunks_flt));
      for (uint8_t chunk_idx = 0; chunk_idx < num_of_chunks; chunk_idx++)
      {
         Write_F360_UDP_Stream_Chunk(sourceTxTime_us, stream_ref_index, stream_number, stream_version, chunk_size, num_of_chunks, chunk_idx, log_data_ptr);
      }
   }

   
}
/*===========================================================================*\
 * FUNCTION: Initialize_Tracker()
 *===========================================================================
 * RETURN VALUE:
 *   void
 *
 * PARAMETERS:
 * CAF_Param_T* p_CAF_Parameter
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
 * This function initialize tracker
 *
 * PRECONDITIONS:
 * CAF Parameters must be read before initializing the tracker
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
void Initialize_Tracker(CAF_Param_T* p_CAF_Parameter)
{
   // Initialize Tracker Globals
   (void*)memset(&g_core_info, 0, sizeof(F360_Core_Info_T));
   (void*)memset(&g_host, 0, sizeof(F360_Host_T));
   (void*)memset(&g_host_calib, 0, sizeof(F360_Host_Calib_T));
   (void*)memset(&g_det_list, 0, sizeof(rspp_variant_A::RSPP_Detection_List_T));
   (void*)memset(g_sensors, 0, sizeof(F360_Radar_Sensor_T) * MAX_NUMBER_OF_SENSORS);

   // Initialize Tracker Output
   (void*)memset(&g_tracker_OAL, 0, sizeof(Tracker_OAL_T));

   // Init F360 Tracker
   g_tracker_state_manager.Initialize();

   // Init Host Calib
   g_host_calib.dist_rear_axle_to_vcs_m = p_CAF_Parameter->host.dist_rear_axle_to_vcs_m;
   g_host_calib.rear_cornering_compliance = p_CAF_Parameter->host.rear_cornering_compliance;
   g_host_calib.steer_gear_ratio = p_CAF_Parameter->host.steer_gear_ratio;
   g_host_calib.wheelbase_m = p_CAF_Parameter->host.wheelbase_m;
   g_host_calib.understeer_coefficient = p_CAF_Parameter->host.understeer_coefficient;
   g_host_calib.vehicle_width_m = p_CAF_Parameter->host.vehicle_width_m;
   g_host_calib.vehicle_length_m = p_CAF_Parameter->host.vehicle_length_m;
   g_host_calib.cog_x = -1.0F; // Currently not used
   g_host_calib.cog_y = -1.0F; // Currently not used
   g_host_calib.front_wheel_radius_m = -1.0F; // Currently not used
   g_host_calib.front_track_width_m = -1.0F; // Currently not used
   g_host_calib.raw_host_signal_latency_ms = 50;
   g_host_calib.f_enable_internal_reflections_func = p_CAF_Parameter->host.f_enable_internal_reflections_func;
   g_host_calib.f_enable_internal_reflections_func_trailer = p_CAF_Parameter->host.f_enable_internal_reflections_func_trailer;

   // Setting sensors with default values
   Set_SRR_Sensors_Cals(g_sensors, *p_CAF_Parameter, g_host_calib);
   
   //initialization rspp
   Initialize_RSPP_Calibrations(rspp_calibs);
   // Flag to indicate Tracker is initialized
   g_Tracker_initialized = 1;

   /* Initialize OCG */
   OCG_inputs.host.dist_rear_axle_to_vcs_m = g_host_calib.dist_rear_axle_to_vcs_m;
   OCG_grid.initialize(OCG_inputs);
} // End of Initialize_Tracker()

/*===========================================================================*\
 * FUNCTION: Run_Tracker_Wrapper()
 *===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   Tracker_IAL_T *p_tracker_IAL : Pointer to Tracker IAL
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
 * Maps the tracker IAL data to f360 internal structures and runs the tracker
 *
 * PRECONDITIONS:
 * None
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
void Run_Tracker_Wrapper(Tracker_IAL_T* p_tracker_IAL)
{
   // Update Core info data
   Fill_Core_Info(g_core_info, p_tracker_IAL->tracker_timestamp_us);

   // Map tracker IAL data to host and sensor data
   Fill_Host_Info(p_tracker_IAL->vehicle_info, g_host_calib, g_host, g_host_raw);
   Fill_SRR_Sensors_Data(*p_tracker_IAL, g_det_list, g_sensors);
   g_speed_correction_factor = p_tracker_IAL->vehicle_info.speed_correction_factor;

   // Update sensor motion parameters
   F360_Update_Sensor_Motion(g_sensors, g_host_calib, g_host);

   // Update internal reflections setting
   F360_Check_If_Internal_Reflections_Function_Shall_Be_Enabled(g_host_calib,
      p_tracker_IAL->vehicle_info.f_trailer_presence_hardware, g_sensors);

   
   RSPP_Core_Info_T rspp_core_info{};
   rspp_core_info.cnt_loops = g_core_info.cnt_loops;
   rspp_core_info.elapsed_time_s = g_core_info.elapsed_time_s;
   rspp_core_info.prev_time_us = g_core_info.prev_time_us;
   rspp_core_info.time_us = g_core_info.time_us;

   // RSPP_Host_T rspp_host{};
   RSPP_Host_T rspp_host{};
   rspp_host.vehicle_index = g_host.vehicle_index;
   rspp_host.speed = g_host.speed;
   rspp_host.vcs_speed = g_host.vcs_speed;
   rspp_host.acceleration = g_host.acceleration;
   rspp_host.vcs_lat_acceleration = g_host.vcs_lat_acceleration;
   rspp_host.vcs_long_acceleration = g_host.vcs_long_acceleration;
   rspp_host.yaw_rate_rad = g_host.yaw_rate_rad;
   rspp_host.vcs_sideslip = g_host.vcs_sideslip;
   rspp_host.curvature_rear = g_host.curvature_rear;
   rspp_host.dist_rear_axle_to_vcs_m = g_host.dist_rear_axle_to_vcs_m;
   rspp_host.rear_cornering_compliance = g_host.rear_cornering_compliance;
   rspp_host.speed_correction_factor = g_host.speed_correction_factor;
   rspp_host.speed_qf = g_host.speed_qf;
   rspp_host.yaw_rate_qf = g_host.yaw_rate_qf;
   rspp_host.lat_accel_qf = g_host.lat_accel_qf;
   rspp_host.long_accel_qf = g_host.long_accel_qf;
   rspp_variant_A::Inputs_Preprocessing(rspp_core_info, rspp_host,
      reinterpret_cast<rspp_variant_A::F360_Radar_Sensor_T(&)[MAX_NUMBER_OF_SENSORS]>(g_sensors),
      rspp_calibs, g_det_list);
   
   //Occupancy grip struct
   ocg::OCG_Outputs_T occupancy_grid_output = {};

   for (int i = 0; i < rspp_variant_A::MAX_NUMBER_OF_SENSORS; ++i)
   {
       OCG_inputs.sensors[i] = reinterpret_cast<rspp_variant_A::F360_Radar_Sensor_T(&)>(g_sensors[i]);
   }
   OCG_inputs.detection_list = g_det_list;
   OCG_inputs.host = rspp_host;
   OCG_grid.step(static_cast<double>(p_tracker_IAL->tracker_timestamp_us), OCG_inputs);
   OCG_grid.get_output(occupancy_grid_output);

   // Run tracker
   g_tracker_state_manager.execute(g_core_info, g_host, occupancy_grid_output, g_det_list, g_sensors, g_tracker_OAL.objects);
   
   // Fill Tracker output log structures
   Log_Output_Data(g_tracker_OAL, g_f360_tracker, g_core_info, g_host, g_host_calib, g_sensors);

   //Fill the ROT object list for downstream users
   g_f360_tracker.Fill_ROT_Object_Output(g_det_list, g_host, g_sensors, g_tracker_OAL.fs_faults_log, rot_object_list);
   
   // Log the tracker internals
   F360_Log_Tracker_Internals(g_f360_tracker, g_core_info);

} // End of Tracker_Main()

/*===========================================================================*\
 * FUNCTION: F360_Update_Sensor_Motion()
 *===========================================================================
 * RETURN VALUE:
 * void
 *
 * PARAMETERS:
 *   const F360_Radar_Sensor_Calib_T(&r_sensor_calibs)[MAX_NUMBER_OF_SENSORS]
 *   const F360_Host_Calib_T& r_host_calib
 *   const F360_Host_T& r_host
 *   F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS]
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
 * Compute sensor vcs velocities
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
void F360_Update_Sensor_Motion(F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS], const F360_Host_Calib_T& r_host_calib, const F360_Host_T& r_host)
{
   float xsens = 0.0f;
   float ysens = 0.0f;
   float xdotvcs = 0.0f;
   float ydotvcs = 0.0f;
   float rear_sideslip;
   float cb = 0.0f;
   float sb = 0.0f;

   rear_sideslip = -(r_host_calib.rear_cornering_compliance) * (r_host.curvature_rear * r_host.speed * r_host.speed);
   cb = cosf(rear_sideslip);
   sb = sinf(rear_sideslip);

   for (int32_t eSens = 0; eSens < MAX_NUMBER_OF_SENSORS; eSens++)
   {
      // Sensor position relative to center of rear axle (in VCS-aligned coordinates)
      xsens = r_host.dist_rear_axle_to_vcs_m + r_sensors[eSens].constant.mounting_position.vcs_position.longitudinal;
      ysens = r_sensors[eSens].constant.mounting_position.vcs_position.lateral;

      // VCS components of OTG velocity of sensor
      xdotvcs = (r_host.speed * cb) - (r_host.yaw_rate_rad * ysens);
      ydotvcs = (r_host.speed * sb) + (r_host.yaw_rate_rad * xsens);

      r_sensors[eSens].variable.vcs_velocity.longitudinal = xdotvcs;
      r_sensors[eSens].variable.vcs_velocity.lateral = ydotvcs;
   }
} // End of F360_Update_Sensor_Motion()

/*===========================================================================*\
 * FUNCTION: Set_Sensors_Cals()
 *===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   F360_Radar_Sensor_Calib_T(&r_sensor_calibs)[MAX_NUMBER_OF_SENSORS]
 *   const CAF_Param_T& CAF_Paramete
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
 * This function will update the sensor cals values
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
static void Set_SRR_Sensors_Cals(F360_Radar_Sensor_T(&r_sensor)[MAX_NUMBER_OF_SENSORS],
   const CAF_Param_T& CAF_Parameter, const F360_Host_Calib_T& r_host_calib)
{
   for (int32_t i = 0; i < NUM_SRR_SENSORS; i++)
   {
      // Sensor properties
      r_sensor[i].constant.id = i + 1;
      r_sensor[i].constant.sensor_type = F360_SENSOR_TYPE_SRR5_RADAR;
      r_sensor[i].variable.is_valid = 1;

      // Mounting properties
      r_sensor[i].constant.mounting_location = F360_MOUNTING_LOCATION_CENTER_REAR;
      r_sensor[i].constant.mounting_position.vcs_position.longitudinal = CAF_Parameter.sensor[i].Sensor_offset_long;
      r_sensor[i].constant.mounting_position.vcs_position.lateral = CAF_Parameter.sensor[i].Sensor_offset_lat;
      r_sensor[i].constant.mounting_position.vcs_position.height = CAF_Parameter.sensor[i].Sensor_offset_vert;
      r_sensor[i].constant.mounting_position.vcs_boresight_azimuth_angle = CAF_Parameter.sensor[i].Sensor_offset_az_angle;
      r_sensor[i].constant.mounting_position.vcs_boresight_elevation_angle = CAF_Parameter.sensor[i].Sensor_offset_el_angle;
      r_sensor[i].constant.polarity = CAF_Parameter.sensor[i].Sensor_polarity;

      // Detection properties
      r_sensor[i].constant.range_limits[0] = 100.400002F;
      r_sensor[i].constant.range_limits[1] = 107.099998F;
      r_sensor[i].constant.range_limits[2] = 41.2999992F;
      r_sensor[i].constant.range_limits[3] = 44.0999985F;

      r_sensor[i].constant.v_wrapping[0] = 69.8359375F;
      r_sensor[i].constant.v_wrapping[1] = 59.6132813F;
      r_sensor[i].constant.v_wrapping[2] = 69.8359375F;
      r_sensor[i].constant.v_wrapping[3] = 59.6132813F;

      r_sensor[i].constant.fov_min_az_rad[0] = -1.308997F;
      r_sensor[i].constant.fov_min_az_rad[1] = -1.308997F;
      r_sensor[i].constant.fov_min_az_rad[2] = -1.308997F;
      r_sensor[i].constant.fov_min_az_rad[3] = -1.308997F;

      r_sensor[i].constant.fov_max_az_rad[0] = 1.308997F;
      r_sensor[i].constant.fov_max_az_rad[1] = 1.308997F;
      r_sensor[i].constant.fov_max_az_rad[2] = 1.308997F;
      r_sensor[i].constant.fov_max_az_rad[3] = 1.308997F;

      // Antenna sensitivity map
      r_sensor[i].variable.f_ant_sens_available = 1; //hard coded for now
      r_sensor[i].variable.f_ant_sens_degraded = 0; //hard coded for now

      // Version info
      r_sensor[i].constant.sensor_sw_version = 0;
      r_sensor[i].constant.min_aliaised_range_rate[0] = -45.0F;
      r_sensor[i].constant.min_aliaised_range_rate[1] = -45.0F;
      r_sensor[i].constant.min_aliaised_range_rate[2] = -45.0F;
      r_sensor[i].constant.min_aliaised_range_rate[3] = -45.0F;
   }

   for (int32_t isens = 0; isens < NUM_TOTAL_RADAR_SENSORS; isens++)
   {
      for (int32_t i_az = 0; i_az < NUM_VTX_ANT_SENS_POLY; i_az++)
      {
         const int32_t i_dst = (1 == r_sensor[isens].constant.polarity) ? i_az : (NUM_VTX_ANT_SENS_POLY - 1) - i_az;
         r_sensor[isens].constant.ant_sens_SCS_sq_rng_90[i_dst] = g_ant_sens_SCS_rng_90[i_az] * g_ant_sens_SCS_rng_90[i_az];
         r_sensor[isens].constant.ant_sens_SCS_sq_rng_50[i_dst] = g_ant_sens_SCS_rng_50[i_az] * g_ant_sens_SCS_rng_50[i_az];
         r_sensor[isens].constant.ant_sens_SCS_azim[i_az] = F360_DEG2RAD(g_ant_sens_SCS_azim_deg[i_az]);
      }
      if (1U == r_sensor[isens].variable.is_valid)
      {
         F360_Initialize_Internal_Reflections_Function_Tuning(r_host_calib, r_sensor[isens]);
      }
   }
} // End of Set_Sensors_Default_values()

/*===========================================================================*\
 * FUNCTION: Fill_Core_Info()
 *===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   F360_Core_Info_T& r_core_info
 *   const uint64_t timestamp_usec
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
 * Increment tracker index. Update timestamp and compute elapsed time
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
static void Fill_Core_Info(F360_Core_Info_T& r_core_info, const uint64_t timestamp_usec)
{
   // Increment Tracker Loop Counter
   r_core_info.cnt_loops++;
   r_core_info.prev_time_us = ((1 == r_core_info.cnt_loops) ? (timestamp_usec) : r_core_info.time_us);
   r_core_info.time_us = (timestamp_usec);
   r_core_info.elapsed_time_s = (float)((double)(r_core_info.time_us - r_core_info.prev_time_us) * 1e-6);

} // End of  Fill_Tracker_Info()

/*===========================================================================*\
 * FUNCTION: Fill_Host_Info()
 *===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   F360_Host_T& r_host
 *   const Ego_Vehicle_Info_T& r_vehicle_info
 *   const F360_Host_Calib_T& r_host_calib
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
 * This function will update the host value from Tracker IAL
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
static void Fill_Host_Info(const Ego_Vehicle_Info_T& vehicle_info, const F360_Host_Calib_T& host_calib, F360_Host_T& host, F360_Host_Raw_T& host_raw)
{
   Fill_Host_Raw_Info(vehicle_info,host_raw);
   Map_Host_Raw_To_Tracker_Host(host_raw, host_calib, host); //Here is simplelified mapping from host_raw to host. Normally there is an vse output to be used to map to host.
}/*End of Fill_Host_Info() */

static void Fill_Host_Raw_Info(const Ego_Vehicle_Info_T& veh_input, F360_Host_Raw_T& host_raw)
{
   /* Time */
   host_raw.timestamp_s = veh_input.timestamp_s;

   /* Host Info */
   host_raw.raw_speed = veh_input.speed;
   host_raw.raw_yaw_rate_rad = (2.0F > std::abs(veh_input.yaw_rate_rad)) ? veh_input.yaw_rate_rad : 0.0F;
   host_raw.steering_wheel_angle_rad = veh_input.steering_angle_rad;
   host_raw.road_wheel_angle_rad = veh_input.road_wheel_angle_rad;
   host_raw.lat_accel = veh_input.vcs_lat_acceleration;
   host_raw.long_accel = veh_input.vcs_long_acceleration;
   host_raw.prndl = veh_input.prndl;
   host_raw.reverse_gear = (1 == veh_input.f_reverse_gear);
   host_raw.f_trailer_presence_hardware = veh_input.f_trailer_presence_hardware;

   /* Quality flags */
   host_raw.speed_qf = veh_input.speed_qf;
   host_raw.yaw_rate_qf = veh_input.yaw_rate_qf;
   host_raw.steering_wheel_angle_qf = veh_input.steering_wheel_angle_qf;
   host_raw.road_wheel_angle_qf = veh_input.road_wheel_angle_qf;
   host_raw.lat_accel_qf = veh_input.lat_accel_qf;
   host_raw.long_accel_qf = veh_input.long_accel_qf;

   /* Safety Condition to make sure speed is positive when reverse_gear flag is off */
   if (0U == host_raw.reverse_gear)
   {
      host_raw.raw_speed = std::abs(host_raw.raw_speed);
   }
   if ((1U == host_raw.reverse_gear) && (0.0F < host_raw.raw_speed))
   {
      host_raw.raw_speed *= -1.0F;
   }
}

static void Map_Host_Raw_To_Tracker_Host(const F360_Host_Raw_T& host_raw, const F360_Host_Calib_T& host_calib, F360_Host_T& host)
{
   // This example wrapper does not execute VSE. Down here is a simplified rough maping from host raw
   host.vehicle_index = 0; // when vse is executed, this vehicle index should be filled by vse output
   host.speed = host_raw.raw_speed;
   host.vcs_speed = host_raw.raw_speed; 
   host.acceleration = host_raw.long_accel; 
   host.vcs_lat_acceleration = host_raw.lat_accel;
   host.vcs_long_acceleration = host_raw.long_accel;
   host.yaw_rate_rad = host_raw.raw_yaw_rate_rad;
   host.vcs_sideslip = 0.0F; 
   host.curvature_rear = 0.0F;
   host.speed_correction_factor = 1.0F;
   host.speed_qf = static_cast<F360_QUALITY_FACTOR>(host_raw.speed_qf);
   host.yaw_rate_qf = static_cast<F360_QUALITY_FACTOR>(host_raw.yaw_rate_qf);
   host.lat_accel_qf = static_cast<F360_QUALITY_FACTOR>(host_raw.lat_accel_qf);
   host.long_accel_qf = static_cast<F360_QUALITY_FACTOR>(host_raw.long_accel_qf);

   host.dist_rear_axle_to_vcs_m = host_calib.dist_rear_axle_to_vcs_m;
   host.rear_cornering_compliance = host_calib.rear_cornering_compliance;
}

/*===========================================================================*\
 * FUNCTION: Fill_SRR_Sensors_Data()
 *===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   rspp_variant_A::RSPP_Detection_List_T& r_det_list
 *   F360_Radar_Sensor_T (&r_sensors)[MAX_NUMBER_OF_SENSORS]
 *   const Tracker_IAL_T& tracker_IAL
 *   const F360_Radar_Sensor_Calib_T (&r_sensor_calibs)[MAX_NUMBER_OF_SENSORS]
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
 * This function will update the det_list and sensors data from the tracker_IAL
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
\*===========================================================================*/
static void Fill_SRR_Sensors_Data(const Tracker_IAL_T& tracker_IAL, 
                                  rspp_variant_A::RSPP_Detection_List_T& r_det_list, 
                                  F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS])
{
   uint16_t raw_det_lst_idx = 0;
   uint16_t prev_raw_det_lst_idx = 0;

   // Fill Raw detections in Tracker Internal buffer
   memset(&r_det_list, 0, sizeof(rspp_variant_A::RSPP_Detection_List_T));

   for (int32_t sen_idx = 0; sen_idx < NUM_SRR_SENSORS; sen_idx++)
   {
      prev_raw_det_lst_idx = raw_det_lst_idx;
      const SRR_Sensor_Info_T& srr_sensor_info = tracker_IAL.srr_sensor_info[sen_idx];
      for (uint32_t det_idx = 0; det_idx < srr_sensor_info.sens_data.number_of_valid_detections; det_idx++)
      {
         const SRR_Det_Data_T& det_list_local = srr_sensor_info.det_data[det_idx];

         r_det_list.detections[raw_det_lst_idx].raw.sensor_id = sen_idx + 1;
         r_det_list.detections[raw_det_lst_idx].raw.det_id = det_list_local.det_id;
         r_det_list.detections[raw_det_lst_idx].raw.range = det_list_local.range;
         r_det_list.detections[raw_det_lst_idx].raw.range_rate = det_list_local.range_rate;

         r_det_list.detections[raw_det_lst_idx].raw.azimuth = det_list_local.azimuth;
         r_det_list.detections[raw_det_lst_idx].raw.elevation = det_list_local.elevation;
         r_det_list.detections[raw_det_lst_idx].raw.snr = det_list_local.snr;
         r_det_list.detections[raw_det_lst_idx].raw.confid_azimuth = (int8_t)det_list_local.confid_azimuth;
         r_det_list.detections[raw_det_lst_idx].raw.confid_elevation = (int8_t)det_list_local.confid_elevation;
         r_det_list.detections[raw_det_lst_idx].raw.f_super_res = det_list_local.f_super_res;
         r_det_list.detections[raw_det_lst_idx].raw.f_host_veh_clutter = det_list_local.f_host_veh_clutter;
         r_det_list.detections[raw_det_lst_idx].raw.f_nd_target = det_list_local.f_nd_target;
         r_det_list.detections[raw_det_lst_idx].raw.f_bistatic = det_list_local.f_bistatic;

         raw_det_lst_idx++;
      }

      // Updating sensor info
      r_sensors[sen_idx].variable.timestamp_us = srr_sensor_info.sens_data.timestamp_us;
      r_sensors[sen_idx].variable.number_of_valid_detections = raw_det_lst_idx - prev_raw_det_lst_idx;

      // Look info
      r_sensors[sen_idx].variable.look_index = srr_sensor_info.sens_data.look_index;
      r_sensors[sen_idx].variable.look_id = (F360_Det_Look_ID_T)srr_sensor_info.sens_data.look_id;

      // Clear Motion info (Filled inside F360_Update_Sensor_Motion)
      r_sensors[sen_idx].variable.vcs_velocity.longitudinal = 0;
      r_sensors[sen_idx].variable.vcs_velocity.lateral = 0;

      // Update compensated sensor borsight angle which is corrected by sensor misalignment 
      r_sensors[sen_idx].variable.vacs_boresight_az_estimated = srr_sensor_info.sens_data.vacs_boresight_az_estimated;
      r_sensors[sen_idx].variable.vacs_boresight_el_estimated = srr_sensor_info.sens_data.vacs_boresight_el_estimated;

      // Populating ego vehicle speed and yaw rate
      r_sensors[sen_idx].variable.vehicle_speed_calc_mps = tracker_IAL.vehicle_info.speed;
      r_sensors[sen_idx].variable.yaw_rate_calc_dps = tracker_IAL.vehicle_info.yaw_rate_rad;
   }

   // Updating total number of detections from all sensors
   r_det_list.number_of_valid_detections = raw_det_lst_idx;
} // End of Fill_SRR_Sensors_Data()

/*================================================================================================================*/
void F360_Initialize_Internal_Reflections_Function_Tuning(const f360_variant_A::F360_Host_Calib_T& r_host_calib, f360_variant_A::F360_Radar_Sensor_T& r_sensor)
{
   if (r_host_calib.f_enable_internal_reflections_func)
   {
      r_sensor.constant.internal_reflections.min_host_vel = 0.5F;
      r_sensor.constant.internal_reflections.age_threshold = 200U;
      r_sensor.constant.internal_reflections.occurrence_lowerlimit = 0.09F;
      r_sensor.constant.internal_reflections.occurrence_threshold = 0.1F;
      r_sensor.constant.internal_reflections.azimuth_tolerance = 0.1F;
      r_sensor.constant.internal_reflections.range_tolerance = 0.2F;
      r_sensor.constant.internal_reflections.max_abs_range_rate = 0.3F;
      r_sensor.constant.internal_reflections.range_max = 1.0F;
      r_sensor.constant.internal_reflections.f_enable = true;
   }
   else if (r_host_calib.f_enable_internal_reflections_func_trailer)
   {
      r_sensor.constant.internal_reflections.min_host_vel = 0.5F;
      r_sensor.constant.internal_reflections.age_threshold = 200U;
      r_sensor.constant.internal_reflections.occurrence_lowerlimit = 0.09F;
      r_sensor.constant.internal_reflections.occurrence_threshold = 0.1F;
      r_sensor.constant.internal_reflections.azimuth_tolerance = 0.1F;
      r_sensor.constant.internal_reflections.range_tolerance = 0.2F;
      r_sensor.constant.internal_reflections.max_abs_range_rate = 0.3F;
      r_sensor.constant.internal_reflections.range_max = 1.0F;
      r_sensor.constant.internal_reflections.f_enable = false; // Will be set to true during runtime when trailer is present
   }
   else // Function will be always be disabled, set f_enable to false, no other parameters necessary
   {
      r_sensor.constant.internal_reflections = {};
   }
}

/*================================================================================================================*/
static bool Is_Sensor_Facing_Host_Rear(const F360_Mounting_Location_T mounting_location)
{
   bool f_sensor_faces_rear = false;
   if (F360_MOUNT_LOC_LEFT_REAR == mounting_location ||
      F360_MOUNT_LOC_RIGHT_REAR == mounting_location ||
      F360_MOUNT_LOC_CENTER_REAR == mounting_location ||
      F360_MOUNT_LOC_CENTER2_REAR == mounting_location ||
      F360_MOUNT_LOC_CENTER3_REAR == mounting_location)
   {
      f_sensor_faces_rear = true;
   }

   return f_sensor_faces_rear;
}

void F360_Check_If_Internal_Reflections_Function_Shall_Be_Enabled(const F360_Host_Calib_T& r_host_calib,
   const bool f_trailer_present, F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS])
{
   // Note, the signal host_raw.f_trailer_present is not very time sensitive and thus does not need to pass through the VSE

   if (r_host_calib.f_enable_internal_reflections_func)
   {
      // Do nothing, function enabled during initialization. Has priority over f_enable_internal_reflections_func_trailer
   }
   else if (r_host_calib.f_enable_internal_reflections_func_trailer && f_trailer_present)
   {
      // Trailer mode enabled and trailer is present, enable for rear sensors
      for (F360_Radar_Sensor_T& sensor : r_sensors)
      {
         if (Is_Sensor_Facing_Host_Rear(sensor.constant.mounting_location))
         {
            sensor.constant.internal_reflections.f_enable = true;
         }
      }
   }
   else if (r_host_calib.f_enable_internal_reflections_func_trailer && !f_trailer_present)
   {
      // Trailer mode enabled and trailer is not present, disable for all sensors 
      for (F360_Radar_Sensor_T& sensor : r_sensors)
      {
         sensor.constant.internal_reflections.f_enable = false;
      }
   }
   else
   {
      // Do nothing, disabled during initialization
   }
};

/*================================================================================================================*/
void Log_Output_Data(Tracker_OAL_T& r_tracker_OAL, const F360_Tracker r_f360_tracker,
   const F360_Core_Info_T& r_core_info, const F360_Host_T& r_host, const F360_Host_Calib_T& r_host_calib,
   F360_Radar_Sensor_T(&r_sensors)[MAX_NUMBER_OF_SENSORS])
{
   // Read tracker fault status for Logging
   g_tracker_state_manager.Log_Functional_Safety_Faults(r_tracker_OAL.fs_faults_log);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      F360_FUNCTIONAL_SAFETY_FAULTS_LOG_STREAM_NUM, F360_FUNCTIONAL_SAFETY_FAULTS_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.fs_faults_log), &r_tracker_OAL.fs_faults_log);
   
   // Copy Tracker Info for Logging
   r_f360_tracker.Log_Tracker_Info(&r_tracker_OAL.tracker_info);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      TRACKER_INFO_LOG_STREAM_NUM, TRACKER_INFO_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.tracker_info), &r_tracker_OAL.tracker_info);

   // Host Props Logging
   r_f360_tracker.Log_F360_Host_Props(r_tracker_OAL.f360_host_props_info);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      F360_HOST_PROPS_LOG_STREAM_NUM, F360_HOST_PROPS_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.f360_host_props_info), &r_tracker_OAL.f360_host_props_info);

   // Objects Logging
   r_f360_tracker.Log_Objects(r_tracker_OAL.objects);
   //Stream size argument is not used for this log struct
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      F360_Objects_Log_Stream_Num, F360_Objects_Log_Stream_Ver,
      0U, &r_tracker_OAL.objects);

   // Detections Logging
   r_f360_tracker.Log_Detections(&r_tracker_OAL.detections,g_det_list);
   //Stream size argument is not used for this log struct
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
       F360_Detection_Log_Stream_Num, F360_Detection_Log_Stream_Ver,
      0U, &r_tracker_OAL.objects);

   // Synchronization info Logging
   r_f360_tracker.Log_Sync_Info(r_tracker_OAL.sync_info, g_host, g_sensors);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      SYNC_INFO_LOG_STREAM_NUM, SYNC_INFO_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.sync_info), &r_tracker_OAL.sync_info);     

   // Vehicle Info Logging */
   r_f360_tracker.Log_Vehicle_Info(r_tracker_OAL.vehicle_info, r_host, 
      g_host_raw, r_sensors);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      VEHICLE_INFO_LOG_STREAM_NUM, VEHICLE_INFO_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.vehicle_info), &r_tracker_OAL.vehicle_info);

   r_f360_tracker.Log_Trailer_Detector(&r_tracker_OAL.trailer_detector_log);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      TRAILER_DETECTOR_LOG_STREAM_NUM, TRAILER_DETECTOR_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.trailer_detector_log), &r_tracker_OAL.trailer_detector_log);

     /* Log Calib Streams every 100 cycles. */
   if (0U == (r_core_info.cnt_loops % 100))
   {
      r_f360_tracker.Log_Host_Calibs(r_tracker_OAL.host_calib_log, r_host_calib);
      r_f360_tracker.Log_Sensor_Calibs(r_tracker_OAL.sensor_calibs, r_sensors);
      Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      HOST_CALIBS_LOG_STREAM_NUM, HOST_CALIBS_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.host_calib_log), &r_tracker_OAL.host_calib_log);
      Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops, 
      F360_Sensor_Calib_Log_Stream_Num, F360_Sensor_Calib_Log_Stream_Ver,
      0U, &r_tracker_OAL.sensor_calibs);
   }
   
   // Copy Tracker Timing Info for Logging
   r_f360_tracker.Log_Timing_Info(&r_tracker_OAL.timing_log);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      TIMING_INFO_LOG_STREAM_NUM, TIMING_INFO_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.timing_log), &r_tracker_OAL.timing_log);

   // Copy the static environment polynomials for Logging
   r_f360_tracker.Log_Static_Env_Polys(&r_tracker_OAL.Log_Static_Env_Polys);
   Write_F360_UDP_Stream(r_core_info.time_us, r_core_info.cnt_loops,
      STATIC_ENV_POLYS_LOG_STREAM_NUM, STATIC_ENV_POLYS_LOG_STREAM_VERSION,
      sizeof(r_tracker_OAL.Log_Static_Env_Polys), &r_tracker_OAL.Log_Static_Env_Polys);

}

/*================================================================================================================*/
void F360_Log_Tracker_Internals(const F360_Tracker& r_f360_tracker, const F360_Core_Info_T& r_core_info)
{
   static uint8_t chunk_idx = 0;
   static uint32_t streamRefIndex = 0;
   static bool f_tracker_internals_done = false;

   // Read internals every N updated
   if (0 == chunk_idx)
   {
      streamRefIndex = r_core_info.cnt_loops;

      // Read Internals
      r_f360_tracker.Log_Internal_Clusters(internal_cluster_log);
      r_f360_tracker.Log_Internal_Objects(internal_object_log);
      r_f360_tracker.Log_Internal_Detection_History(internal_det_hist_log);
      r_f360_tracker.Log_Internal_Reflection_Buffer(internal_reflection_log);
      r_f360_tracker.Log_Internal_CWD(internal_cwd_log);
   }

   // Write Tracker Internals
   f_tracker_internals_done = F360_Log_Variable_Tracker_Internals(streamRefIndex);

   // Update chunk index
   if (f_tracker_internals_done)
   {
      chunk_idx = 0U;
      f_tracker_internals_done = false;
   }
   else
   {
      chunk_idx++;
   }
}

bool F360_Log_Variable_Tracker_Internals(const uint32_t stream_ref_idx)
{
   static bool all_tx_done = true;
   static bool cluster_tx_done = true;
   static bool object_tx_done = true;
   static bool hist_det_tx_done = true;
   static bool refl_tx_done = true;
   static bool cwd_tx_done = true;

   if (all_tx_done)
   {
      all_tx_done = false;
      cluster_tx_done = false;
      object_tx_done = false;
      hist_det_tx_done = false;
      refl_tx_done = false;
      cwd_tx_done = false;
   }

   if (!cluster_tx_done)
   {
      cluster_tx_done = Write_Single_UDP_Stream_Chunk(stream_ref_idx, (F360_Log_Data_T*)&internal_cluster_log, F360_Internal_Cluster_Stream_Num);
   }
   else if (!object_tx_done)
   {
      object_tx_done = Write_Single_UDP_Stream_Chunk(stream_ref_idx, (F360_Log_Data_T*)&internal_object_log, F360_Internal_Object_Stream_Num);
   }
   else if (!hist_det_tx_done)
   {
      hist_det_tx_done = Write_Single_UDP_Stream_Chunk(stream_ref_idx, (F360_Log_Data_T*)&internal_det_hist_log, F360_Internal_Detection_History_Stream_Num);
   }
   else if (!refl_tx_done)
   {
      refl_tx_done = Write_Single_UDP_Stream_Chunk(stream_ref_idx, (F360_Log_Data_T*)&internal_reflection_log, F360_Internal_Reflection_Buffer_Stream_Num);
   }
   else if (!cwd_tx_done)
   {
      cwd_tx_done = Write_Single_UDP_Stream_Chunk(stream_ref_idx, (F360_Log_Data_T*)&internal_cwd_log, F360_Internal_CWD_Stream_Num);
   }
   else
   {
      all_tx_done = true;
   }

   return all_tx_done;
}

bool Write_Single_UDP_Stream_Chunk(uint32_t streamRefIndex, const F360_Log_Data_T* log, uint8_t stream_number)
{
   bool log_complete = false;
   static uint8_t stream_chunk_idx[255] = { 0 };

   uint32_t log_size = sizeof(F360_Log_Header_T);
   uint16_t max_payload_size = MAX_PAYLOAD_SIZE;
   uint8_t stream_version = 0;
   switch (stream_number)
   {
      case F360_Internal_Cluster_Stream_Num:
         log_size += sizeof(F360_Internal_Cluster_T) * log->f360header.num_elements;
         stream_version = F360_Internal_Cluster_Stream_Ver;
         max_payload_size = F360_Internal_Cluster_Max_Payload_Size;
         break;
      case F360_Internal_Object_Stream_Num:
         log_size += sizeof(F360_Internal_Object_T) * log->f360header.num_elements;
         stream_version = F360_Internal_Object_Stream_Ver;
         max_payload_size = F360_Internal_Object_Max_Payload_Size;
         break;
      case F360_Internal_Detection_History_Stream_Num:
         log_size += sizeof(F360_Internal_Detection_Hist_T) * log->f360header.num_elements;
         stream_version = F360_Internal_Detection_History_Stream_Ver;
         max_payload_size = F360_Internal_Detection_History_Max_Payload_Size;
         break;
      case F360_Internal_Reflection_Buffer_Stream_Num:
         log_size += sizeof(F360_Internal_Reflection_Buffer_T) * log->f360header.num_elements;
         stream_version = F360_Internal_Reflection_Buffer_Stream_Ver;
         max_payload_size = F360_Internal_Reflection_Buffer_Max_Payload_Size;
         break;
      case F360_Internal_CWD_Stream_Num:
         log_size += sizeof(F360_Internal_CWD_T) * log->f360header.num_elements;
         stream_version = F360_Internal_CWD_Stream_Ver;
         max_payload_size = F360_Internal_CWD_Max_Payload_Size;
         break;
      default:
         break;
   }

   uint8_t stream_chunks = static_cast<uint8_t>(log_size / max_payload_size);
   if ((log_size % max_payload_size) > 0)
   {
      stream_chunks++;
   }

   UDPRecord_T udprecord = { 0 };
   uint16_t payload_size = max_payload_size;
   if (stream_chunk_idx[stream_number] == (stream_chunks - 1))
   {
      /* Last chunk, need to calculate required size */
      payload_size = static_cast<uint16_t>(log_size - (max_payload_size * stream_chunk_idx[stream_number]));
   }

   udprecord.header.streamRefIndex = streamRefIndex;
   udprecord.header.streamDataLen = payload_size;
   udprecord.header.streamTxCnt = ++streamTxCnt[stream_number];
   udprecord.header.streamNumber = stream_number;
   udprecord.header.streamVersion = stream_version;
   udprecord.header.streamChunks = stream_chunks;
   udprecord.header.streamChunkIdx = stream_chunk_idx[stream_number];
   udprecord.payload = (uint8_t*)log + udprecord.header.streamChunkIdx * max_payload_size;

   stream_chunk_idx[stream_number]++;
   if (static_cast<uint32_t>(stream_chunk_idx[stream_number] * max_payload_size) >= log_size)
   {
      stream_chunk_idx[stream_number] = 0;
      log_complete = true;
   }
   return log_complete;
}
