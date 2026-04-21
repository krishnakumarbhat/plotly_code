<!---
Reading and editing the guideline is easy with vscode https://code.visualstudio.com/download
Use the markdown preview to see the formatted output as you edit "Ctrl + Shift + V"

Export the guideline to HTML with the "Markdown PDF" extension in vscode (yzane.markdown-pdf)
-->
# Integration Guide
Updated 2023-10-20
## Table of Contents
1. [Preface](#Preface)
1. [Radar Sensor Preprocessing Library Overview](#radar-sensor-preprocessing-library-overview)
1. [Input Interface](#input-interface)
1. [Output Interface](#output-interface)
1. [Building the Library](#building-the-library)
1. [Integrating the RSPP Library](#integrating-the-rspp-library)
1. [Appendix](#appendix)

# Preface
The Radar Sensor Preprocessing (RSPP) component performs pre-processing of raw radar detections. 
It is is mandatory to integrate and execute this component before running the Radar Object Tracker (ROT) and the Occupancy Grid (OCG).
The RSPP should be executed over all sensor data per cycle at once (not per sensor) and it should be executed immediately before the OCG (if applicable) and the ROT in the same thread.

The Radar Sensor Pre-processing(**RSPP**) is built in C++ and measured for compliance against MISRA C++ 2008.

The release package contains test specifications and reports for Unit-Testing. Additionally, it contains a static code analysis report summary of any MISRA C++ 2008 rule violations, a software complexity report, as well as a code coverage summary.

The RSPP library has been successfully compiled with the following:
- MSVC 2019
- GCC 7.5.0
- Tasking Tricore v6.3
- Windriver diab-5.9.6.4
- QNX 7.1.0

The RSPP is wrapped in a namespace which corresponds to the variant that it built as: `rspp_variant_A`, `rspp_variant_B` or `rspp_variant_C`, etc. <span style="color:red"> **Note:** </span> that RSPP and ROT variants must be the same.

# Radar Sensor Preprocessing Library Overview
RSPP is a library that provides the methods to perform pre-processing of the raw radar detections. 
There are mainly two function calls needed to populate the pre-processed detections. 
## 1. Initialize the RSPP calibrations.
   `Initialize_RSPP_Calibrations()` An initialization method that will set up calibration values and the correct starting values of various internal variables. 
## 2. Call the main function `Inputs_Preprocessing()`
   `Inputs_Preprocessing()` Call this function to perform preprocessing of the raw detections. Before calling this function the input structs should be populated with consistent and valid data. Check [Input Interface](#input-interface) below to understand the inputs required.


# Input Interface
The `Inputs_Preprocessing()` function requires 5 structs as arguments to the function,
`RSPP_Core_Tnfo_T`, `RSPP_Host_T`, `RSPP_Detection_List_T`, `F360_Radar_Sensor_T` and `RSPP_Calibrations_T`. The definition of these required inputs are described below.

<span style="color:red">**All detection properties shall be reported according to the SAE J670 z-down coordinate system (x-axis pointing in the direction of the antenna, y-axis to the right of the antenna, z-axis pointing down).**</span>

Project integration teams need to ensure that all the input data is available and updated for each cycle. In case an input struct is not able to be populated it should be cleared rather than sending old/invalid data.
In general, structs are expected to be cleared before populating them.

For definitions with respect to coordinate systems, please refer to Chapter 5 in the following requirements document: https://polarionprod1.aptiv.com/polarion/#/project/F360TrackerCoreProject/wiki/SYS_Requirements/Abbreviations_Definitions_TrackerCore

## `RSPP_Core_Info_T`
| signal | description |
| :--- | :--- |
| `cnt_loops` | Counter that increases every cycle iteration |
| `time_us`   | Time in microseconds |
| `prev_time_us` | Previous time in microseconds |
| `elapsed_time_s` | Time elapsed since the previous tracker execution |

## `RSPP_Host_T`
The vehicle coordinate system (VCS) is defined in the Appendix of this document.

| signal | description |
| :--- | :--- |
| `vehicle_index` | index of VSE iteration |
| `speed` | m/s, total speed at rear axle |
| `vcs_speed` | m/s, total speed at VCS origin |
| `acceleration` | m/s^2, total acceleration at rear axle |
| `vcs_lat_acceleration` | m/s^2, at VCS origin |
| `vcs_long_acceleration` | m/s^2, at VCS origin |
| `yaw_rate_rad` | rad/s, in VCS, compensated for bias |
| `vcs_sideslip` | rad, signal range (-pi/2, pi/2) at VCS, check definition from VSE - might need to be converted from range -pi/pi |
| `curvature_rear` | 1/m, at rear axle |
| `dist_rear_axle_to_vcs_m` | distance between the rear axle and the VCS origin (front bumper) in meters, always a positive value |
| `rear_cornering_compliance` | calibration value |
| `speed_correction_factor` | used to correct the host speed in VSE |
| `speed_qf` | quality flag (used only for debug) |
| `yaw_rate_qf` | quality flag (used only for debug) |
| `lat_accel_qf` | quality flag (used only for debug) |
| `long_accel_qf` | quality flag (used only for debug) |
| `f_trailer_presence_hardware` | trailer presence detected by the host vehicle electronically  |

## `RSPP_Detection_List_T`
`RSPP_Detection_List_T` contains the following:
 * all detections from all sensors in a packed list with `RSPP_Detection_T` type. All detections in the list are expected to be valid.
 * `number_of_valid_detections` member in the struct should correspond to the total number of detections in the array that the RSPP needs to process.
 * vcslong_det_idx_min; Sorted vcs-long index of detection with most negative vcs-long position, this member is updated by RSPP not the wrapper
 * vcslong_det_idx_max; Sorted vcs-long index of detection with most positive vcs-long position, this member is updated by RSPP not the wrapper
 * vcslong_sorted_ref_det_idx; this member is updated by RSPP not the wrapper 

`RSPP_Detection_T` struct contains two substructs, 
 * `Raw_Detection_T` raw; 
 * `Processed_Detection_T` processed; 

 The `Raw_Detection_T` should be populated with raw detections information from the sensors and is used as input to the function `Inputs_Preprocessing()`. 
 
 The `Processed_Detection_T` struct is the output provided by the function call `Inputs_Preprocessing()`. It is populated by the RSPP component - it should be cleared before each RSPP iteration.

The `sensor_id` and `det_id` members in `Raw_Detection_T` need to be populated manually before being sent to the RSPP for further processing. 
The `det_id` should be populated from 1 to the number of detections received for each sensor.
The `sensor_id` shall correspond to the id of the sensor in the `F360_Radar_Sensor_T` structure under `ConstantProps_T` structs as sensor_id = index + 1.
Simplified pseudo code example:
```C++
for (uint8_t iSens=0; iSens < n_sensors; iSens++){
   for (uint8_t iDet=0; iDet < n_dets; iDet++){
      sensor_id = iSens + 1;
      det_id = iDet + 1;
   }
}
```
This will result in the below data representation:

| sensor_id | det_id |
| :--- | :--- |
| 1 | 1 |
| 1 | 2 |
| 1 | 3 |
| 2 | 1 |
| 2 | 2 |
| etc. | etc. |


Description of signals in  `Raw_Detection_T`

| signal | description |
| :--- | :--- |
| `sensor_id` | id of the sensor that the detection came from |
| `det_id` | id from the sensor that the detection came from (unique only to that sensor) |
| `range` | [m], distance to detection |
| `std_range` | set if available from sensor, otherwise zero |
| `range_rate` | m/s |
| `std_range_rate` | set if available from sensor, otherwise zero |
| `azimuth` | [rad] Raw Azimuth angle. **Not compensated for alignment or polarity** |
| `std_azimuth` | set if available from sensor, otherwise zero |
| `elevation` | [rad] Raw Elevation angle. **Not compensated for alignment or polarity** |
| `std_elevation` | set if available from sensor, otherwise zero |
| `snr` | Signal to noise ratio |
| `rcs` | radar cross section |
| `confid_azimuth` | [0-3] 0 = best, 3 = worst |
| `confid_elevation` | [0-1] 0 = best, 1 = worst |
| `f_super_res` | [0-1] 0 = not super resolution, 1 = is super resolution |
| `f_host_veh_clutter` | [0-1] flag indicating that detection is host vehicle clutter |
| `f_nd_target` | [0-1] flag indicating that this detection seems to stem from a small target close to a larger target |
| `f_bistatic` | [0-1] Flag indicating that this detection is a bistatic detection |

**A note on det_id**: When analyzing the output of the radar object tracker it's important to be able to determine which detections eg. are associated to an object. 
The objects contain a `detectionID` field which comes as an index from `RSPP_Detection_List.T`. The corresponding index in the `RSPP_Detection_Log_T` contains a `sensorID` and a `raw_det_id`. 



This information can then be used to trace back to the sensor inputs.

## `F360_Radar_Sensor_T`
Sensor data contains two struct `ConstantProps_T` and `VariableProps_T`.

`ConstantProps_T` contains all the sensor calibration parameters which is set during initialization. The data should not be updated during runtime:
Sensor Coordinate System is abbreviated as SCS below.
All data is per sensor.

| signal | description | Updated by RSPP/Wrapper |
| :--- | :--- | :--- |
| `ant_sens_SCS_azim` | common azimuth angle knots for all antenna sensitivity maps of the sensor, to be populated using input from radar team | Wrapper |
| `ant_sens_SCS_sq_rng_90` | border of antenna sensitivity region for a given probability of detection, to be populated using input from radar team | Wrapper |
| `ant_sens_SCS_sq_rng_50` | border of antenna sensitivity region for a given probability of detection, to be populated using input from radar team | Wrapper |
| `internal_reflections` | Contains enable/disable & tuning parameters for the internal reflections functionality, only applicable if a trailer is connected. Unless otherwise specified/discussed set the subsignal f_enable to false and all other signals to 0 | Wrapper |
| `interior_fov` | interior fov  - per look id | RSPP |
| `left_fov_normal`  | sensor left edge field of view normal vector towards inside [-sinf(theta), cosf(theta)] - per look id | RSPP |
| `right_fov_normal` | sensor right edge field of view normal vector towards inside [sinf(theta), -cosf(theta)] - per look id | RSPP |
| `mounting_position.vcs_position.longitudinal` | [m] longitudinal mounting position of the sensor in VCS | Wrapper |
| `mounting_position.vcs_position.lateral` | [m] lateral mounting position of the sensor in VCS | Wrapper |
| `mounting_position.vcs_position.height` | [m] height above ground mounting position of the sensor | Wrapper |
| `mounting_position.vcs_boresight_azimuth_angle` | [rad] nominal boresight of the sensor in VCS, not compensated for alignment error | Wrapper |
| `mounting_position.vcs_boresight_elevation_angle` | [rad] nominal elevation of the sensor in VCS, not compensated for alignment error | Wrapper |
| `range_limits` | instrumented range of the radar - per look id | Wrapper |
| `fov_min_az_rad` | instrumented azimuth in the negative direction - per look id | Wrapper |
| `fov_max_az_rad` | instrumented azimuth in the positive direction - per look id | Wrapper |
| `fov_min_el_rad` | instrumented elevation in the negative direction - per look id | Wrapper |
| `fov_max_el_rad` | instrumented elevation in the positive direction - per look id | Wrapper |
| `min_aliaised_range_rate` | most negative measurable range-rate before folding,  - per look id | Wrapper |
| `v_wrapping` | Absolute value of SMC same-titled parameter; range rate dealiasing interval - per look id | Wrapper |
| `r_wrapping` | Absolute value of SMC same-titled parameter; range offset after doppler unfolding; zero for FMCW and nonzero for SFW - per look id | Wrapper |
| `id` | sensor id, manually populated, id = index + 1 | Wrapper |
| `polarity` | 1 (regular mounting) or -1 (flipped mounting) depending on mounting orientation | Wrapper |
| `sensor_sw_version` | sw version of the radar sensor | Wrapper |
| `f_read_cdc_data` | not used, set false | Wrapper |
| `mounting_location` |  Check headerfile enumeration | Wrapper |
| `sensor_type` | Check headerfile enumeration | Wrapper |


`VariableProps_T` contains the sensor data that should be updated before tracker iteration

| signal | description |  Updated by RSPP/Wrapper |
| :--- | :--- | :--- |
| `timestamp_us` | microseconds, time when the sensor scanned the environment. Usually >100ms before   tracker execution, and this should be visible when comparing this timestamps and the timestamp in   `F360_Core_Info_T` | Wrapper |
| `vcs_velocity.longitudinal` | m/s | Wrapper |
| `vcs_velocity.lateral` | m/s | Wrapper |
| `vacs_boresight_az_estimated ` | true boresight azimuth angle compensated by sensor misalignment, output from alignment algorithm, [rad] | Wrapper |
| `vacs_boresight_el_estimated` | true boresight elevation angle compensated by sensor misalignment, output from alignment algorithm, [rad] | Wrapper |
| `number_of_valid_detections` | number of detections from this sensor | Wrapper |
| `yaw_rate_calc_dps` | deg/s in VCS, (used only for debug) | Wrapper |
| `vehicle_speed_calc_mps` | m/s in VCS, (used only for debug) | Wrapper |
| `time_since_measurement_s` | delta time between RSPP excution and sensor measurement timestamp. Unit second | RSPP |
| `first_detection_list_idx`| Index of first detection from this sensor in the full tracker detection list | RSPP |
| `look_index` | look_index reported by sensor | Wrapper |
| `is_valid` | set true | Wrapper |
| `f_ant_sens_available` | set true if `ant_sens_SCS`* arrays are populated  | Wrapper |
| `f_ant_sens_degraded` | set true if `ant_sens_SCS`* arrays are populated  | Wrapper |
| `look_id` | [0-3] look_id reported by sensor | Wrapper |

## `Dynamic Radar Alignment`
<span style="color:red">**Important note on the Dynamic Radar Alignment interface from v14.0.1 and onwards.**</span>
In DRA v14.0.1 the output interface changes from providing a misalignment relative a nominal mounting angle of the sensor, to providing a complete estimated mounting angle relative to the vehicle.

This interface change has a hard dependency to the above structure `VariableProps_T` and specifically the new signals `vacs_boresight_az_estimated` and `vacs_boresight_el_estimated` which are a part of the RSPP components input interface since RSPP v2.0.0.

For this reason, if you are using a version of the DRA which is earlier than v14.0.1 together with RSPP v2.0.0 (and ROT v10.0.0) you still need to provide the new signals `vacs_boresight_az_estimated` and `vacs_boresight_el_estimated` and they need to be calculated using the nominal mounting angles and the misalignment angles as in the example below.

```C++
// vacs_boresight_az_estimated from F360_Radar_Sensor_T -> VariableProps_T.
// vacs_boresight_el_estimated from F360_Radar_Sensor_T -> VariableProps_T.

// sensor_misalign_azimuth from legacy alignment algorithm
// sensor_misalign_elevation from legacy alignment algorithm

// vcs_boresight_azimuth_angle from F360_Radar_Sensor_T -> ConstantProps_T -> RSPP_Sensor_Mounting_Position_T
// vcs_boresight_elevation_angle from F360_Radar_Sensor_T -> ConstantProps_T -> RSPP_Sensor_Mounting_Position_T

vacs_boresight_az_estimated = vcs_boresight_azimuth_angle - sensor_misalign_azimuth;
vacs_boresight_el_estimated = vcs_boresight_elevation_angle - sensor_misalign_elevation;
```

In case of the opposite scenario, DRA v14.0.1 or later used together with RSPP 1.x.x and ROT 9.x.x you will need to calculate the legacy misalignment angles using the nominal mounting angles and the corrected boresight angles as in the example below.

```C++
// vacs_boresight_az_estimated from Dynamic_Alignment_Log_Stream_T -> DRA_Core_Log_Data_T.
// vacs_boresight_el_estimated from Dynamic_Alignment_Log_Stream_T -> DRA_Core_Log_Data_T.

// sensor_misalign_azimuth from F360_Radar_Sensor_T -> VariableProps_T (RSPP v1.x.x)
// sensor_misalign_elevation from F360_Radar_Sensor_T -> VariableProps_T (RSPP v1.x.x)

// vcs_boresight_azimuth_angle from F360_Radar_Sensor_T -> ConstantProps_T -> RSPP_Sensor_Mounting_Position_T
// vcs_boresight_elevation_angle from F360_Radar_Sensor_T -> ConstantProps_T -> RSPP_Sensor_Mounting_Position_T

sensor_misalign_azimuth = vcs_boresight_azimuth_angle - vacs_boresight_az_estimated;
sensor_misalign_elevation = vcs_boresight_elevation_angle - vacs_boresight_el_estimated;
```
It is highly recommended to use DRA v14.0.1 or later if you are using ROT 10.0.0 and RSPP 2.0.0 since in this case a simple assignment from the DRA output struct to the F360 Sensor struct is all that is required.


## `RSPP_Calibrations_T`
This struct contains the RSPP calibration parameters and they can be populated by simply calling the function `Initialize_RSPP_Calibrations()`.

# Output Interface
RSPP primary output interface to downstream users

| signal | description |
| :--- | :--- |
| `RSPP_Detection_List_T` | Raw and processed detection data |
| `F360_Radar_Sensor_T` | Sensor calibration data |

After executing RSPP `Inputs_Preprocessing()` detections[MAX_NUMBER_OF_DETECTIONS].processed properties will be updated.
F360_Radar_Sensor_T sensor info data have been converted to VCS coordinate. 

# Building the Library
RSPP is expected to be delivered as a prebuilt library. However if for some reason RSPP is being integrated as source code, no modifications to the code should be made without consulting with the development team.

The default CMake files are used to configure build environments for RSPP variant_A which is used for F360tracker resim and trackerPC applications. A toolchain with limited functionality for Tasking TriCore exists but since linking requires a program specific .lsl file, it cannot be simply dropped into projects.

Part of the CMake configuration allows for the selection of a variant to build. The effect of this is that different variants from the [variants folder](../sw/rspp/include/variants) replaces the `rspp_variant_definition.h`. There are different variants corresponding to different customer programs and integrators need to select the appropriate one in order to ensure that the tracker runs as expected. Additionally, the cmake system will generate a folder with variant specific interfaces which shall be used. The folder will be located in your specified build folder and be named "`iface_variant_*`". If unsure of which variant should be used for a given project, reference in `index.txt` file in the variants folder.

Observe that different variant definitions have different namespaces. In order to also build everything else with the same namespace, the C Preprocessor is used to specify the default namespace name as a symbol and thereby overwriting it with the appropriate namespace name. Exactly how this is done can be seen in the `variant_config.cmake` file for the RSPP.


# Integrating the RSPP Library
## Preface
We will assume that at this phase the rspp has been compiled and archived into a library. Then it can be imported by customized downstream user projects.

## Running a test program
Below is an example of a simple wrapper that calls the RSPP lib main function.

The sample code below below assumes two instances of different variants of both RSPP and ROT, for example (rspp_variant_A + f360_variant_A & rspp_variant_B + f360_variant_A). The integrator must change the namespace name to match the correct variant, and import the RSPP libaray to the projects. If only a single instance is used, the other should be removed.

If the below code compiles successfully, then the minimum requirements for integration are met. If the code can be executed with a connected debugger and function returns zero, then the bare minimum of qualification is met.

```C++
#include "iface_variant_A/rspp_inputs_preprocessing.h"
#include "iface_variant_B/rspp_inputs_preprocessing.h"

/* Support functions */
template<typename T>
static void Advance_Core_Info(T &core_info)
{
   core_info.cnt_loops++;
   core_info.elapsed_time_s = 0.05f;
   core_info.prev_time_us = core_info.time_us;
   core_info.time_us += 50000;
}

template<typename T1,typename T2>
static void Advance_Sensor( const uint64_t time_us, T1 &host, T2 &sensor)
{
   sensor.variable.look_index++;
   sensor.variable.look_id = (sensor.variable.look_id == RSPP_DET_LOOK_ID_0) ? RSPP_DET_LOOK_ID_1 : RSPP_DET_LOOK_ID_0;
   sensor.variable.timestamp_us = time_us;
   sensor.variable.vcs_velocity.longitudinal = host.vcs_speed;
   sensor.variable.vacs_boresight_az_estimated = sensor.constant.mounting_position.vcs_boresight_azimuth_angle; // considering no misalignment
   sensor.variable.vacs_boresight_el_estimated = sensor.constant.mounting_position.vcs_boresight_elevation_angle; // considering no misalignment

}

template<typename T>
static void Init_Sensor_Calibs(T &sensor)
{
   sensor.constant.id = 1;
   sensor.constant.v_wrapping[0] = 70.0f;
   sensor.constant.v_wrapping[1] = 60.0f;
   sensor.constant.r_wrapping[0] = 0.0f;
   sensor.constant.r_wrapping[1] = 0.0f;
   sensor.constant.polarity = 1;
   sensor.constant.mounting_location = RSPP_MOUNTING_LOCATION_CENTER_REAR;
   sensor.constant.mounting_position.vcs_position.lateral = 0.0f;
   sensor.constant.mounting_position.vcs_position.longitudinal = -4.5f;
   sensor.constant.mounting_position.vcs_boresight_azimuth_angle = 3.1415926f;
   sensor.constant.mounting_position.vcs_boresight_elevation_angle = 0.0F;

}

template<typename T>
static void Init_Single_Detection(T &raw_det_list)
{
   raw_det_list.number_of_valid_detections = 1;
   raw_det_list.detections[0].raw.range = 14.5f;
   raw_det_list.detections[0].raw.range_rate = 0.0f;
   raw_det_list.detections[0].raw.sensor_id = 1;
}

template<typename T>
static bool check_processed_det(T &processed_det)
{
   bool pass = (processed_det.range_rate_compensated > -10.01F)
            && (processed_det.range_rate_compensated < -9.99F)
            && (processed_det.vcs_position_x < -18.99F )
            && (processed_det.vcs_position_x > -19.01F )
            && (processed_det.cos_vcs_az > -1.01F)
            && (processed_det.cos_vcs_az < -0.99F);
   
   return pass;
}

template<typename T1, typename T2>
static void Sensor_Motion_Update(T1& host_info, T2& sensor)
{
   // here only considered a simple version of host move straight forward for the RSPP integration test
   float xsens = 0.0f;
   float ysens = 0.0f;
   xsens = host_info.dist_rear_axle_to_vcs_m + sensor.constant.mounting_position.vcs_position.longitudinal;
   ysens = sensor.constant.mounting_position.vcs_position.lateral;
   sensor.variable.vcs_velocity.longitudinal = host_info.vcs_speed;
   sensor.variable.vcs_velocity.lateral = 0;

}


/* Test description
   Scenario:
      - The host moves straight with 10 m/s speed.
      - Single target (as single detection) is behind the host

   Expected output (test pass criteria):
      - Processed detection has correct compensated position, range rate, vcs azimuth angle .
*/

int main()
{
   static rspp_variant_A::RSPP_Calibrations_T rspp_calibs_a;
   static rspp_variant_A::RSPP_Detection_List_T detection_list_a{};
   static RSPP_Core_Info_T rspp_core_info_a{};
   static rspp_variant_A::F360_Radar_Sensor_T sensors_a[rspp_variant_A::MAX_NUMBER_OF_SENSORS]{};

   static rspp_variant_B::RSPP_Calibrations_T rspp_calibs_b;
   static rspp_variant_B::RSPP_Detection_List_T detection_list_b{};
   static RSPP_Core_Info_T rspp_core_info_b{};
   static rspp_variant_B::F360_Radar_Sensor_T sensors_b[rspp_variant_B::MAX_NUMBER_OF_SENSORS]{};
   
   static RSPP_Host_T host_info{};

   Init_Single_Detection(detection_list_a);
   Init_Single_Detection(detection_list_b);
   host_info.vcs_speed = 10.0f;

   Advance_Core_Info(rspp_core_info_a);
   Advance_Core_Info(rspp_core_info_b);

   Init_Sensor_Calibs(sensors_a[0]);
   Init_Sensor_Calibs(sensors_b[0]);
  
   Advance_Sensor(rspp_core_info_a.time_us, host_info, sensors_a[0]);
   Advance_Sensor(rspp_core_info_b.time_us, host_info, sensors_b[0]);

   Sensor_Motion_Update(host_info, sensors_a[0]);
   Sensor_Motion_Update(host_info, sensors_b[0]);
   
   rspp_variant_A::Initialize_RSPP_Calibrations(rspp_calibs_a);
   rspp_variant_B::Initialize_RSPP_Calibrations(rspp_calibs_b);

   rspp_variant_A::Inputs_Preprocessing(rspp_core_info_a,host_info,sensors_a, rspp_calibs_a,rspp_calibs_a.max_otg_speed,detection_list_a);
   rspp_variant_B::Inputs_Preprocessing(rspp_core_info_b, host_info, sensors_b, rspp_calibs_b, rspp_calibs_b.max_otg_speed, detection_list_b);

   uint8_t test_result;

   if(check_processed_det(detection_list_a.detections[0].processed) && check_processed_det(detection_list_b.detections[0].processed))
   {
      test_result = 1U;
   }
   else
   {
      test_result = 0U;
   }
   return test_result;
}
```

# Appendix

Collection of definitions and frequently asked questions.

## Definition of vehicle coordinate system (VCS)

The vehicle coordinate system is a right-handed coordinate system in which the origin is defined at the center of the front bumper. 
The x-axis is defined as positive in the forward direction of the host vehicles driving path. The y-axis is defined as positive to the right and the z-axis is defined as positive towards the ground with the origin at ground level.
All angles in this coordinate system shall be defined in mathematically negative direction of rotation (i.e., clockwise). The standard range for angles is [-pi, pi] in radians (i.e., -180° to 180°).
This coordinate system is rigidly linked to the vehicle.

![vcs_definition](./integration_guideline_img/vcs_definition.png)
