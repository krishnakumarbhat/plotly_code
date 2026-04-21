#include "State_Manager.h"
#include "f360_input_diagnostics.h"
#include "f360_output_diagnostics.h"
#include "f360_safety_control_logic.h"
#include "f360_tracker.h"

#include "rspp_core_info.h"
#include "rspp_host.h"
#include "rspp_radar_sensor.h"
#include "rspp_detection_list.h"
#include "rspp_calibrations.h"
#include "rspp_inputs_preprocessing.h"

#include <iostream>



/* Support functions */
template<typename T>
static void Advance_Core_Info(T &core_info)
{
   core_info.cnt_loops++;
   core_info.elapsed_time_s = 0.05f;
   core_info.prev_time_us = core_info.time_us;
   core_info.time_us += 50000;
}

template<typename T>
static void Advance_Sensor(T &sensor, const  uint64_t time_us)
{
   sensor.variable.look_index++;
   sensor.variable.look_id = (sensor.variable.look_id == RSPP_DET_LOOK_ID_0) ? RSPP_DET_LOOK_ID_1 : RSPP_DET_LOOK_ID_0;
   sensor.variable.timestamp_us = time_us;
   // sensor.variable.vacs_boresight_azimuth_angle = sensor.constant.mounting_position.vcs_boresight_azimuth_angle; // considering no misalignment
   sensor.variable.vacs_boresight_az_estimated = sensor.constant.mounting_position.vcs_boresight_azimuth_angle;

   // sensor.variable.vacs_boresight_elevation_angle = sensor.constant.mounting_position.vcs_boresight_elevation_angle; // considering no misalignment
   sensor.variable.vacs_boresight_el_estimated = sensor.constant.mounting_position.vcs_boresight_elevation_angle;


}

template<typename T>
static void Init_Single_Detection(T &raw_det_list)
{
   raw_det_list.number_of_valid_detections = 1;
   raw_det_list.detections[0].raw.range = 14.5f;
   raw_det_list.detections[0].raw.range_rate = 0.0f;
   raw_det_list.detections[0].raw.sensor_id = 1;
   raw_det_list.detections[0].raw.det_id = 1;
   

}

template<typename RSPP_T, typename Host_T>
static void Set_RSPP_Host_Info(RSPP_T &rspp_host, Host_T &host)
{
   rspp_host.vehicle_index = host.vehicle_index;
   rspp_host.speed = host.speed;
   rspp_host.vcs_speed = host.vcs_speed;
   rspp_host.acceleration = host.acceleration;
   rspp_host.vcs_lat_acceleration = host.vcs_lat_acceleration;
   rspp_host.vcs_long_acceleration = host.vcs_long_acceleration;
   rspp_host.yaw_rate_rad = host.yaw_rate_rad;
   rspp_host.vcs_sideslip = host.vcs_sideslip;
   rspp_host.curvature_rear = host.curvature_rear;
   rspp_host.dist_rear_axle_to_vcs_m = host.dist_rear_axle_to_vcs_m;
   rspp_host.rear_cornering_compliance = host.rear_cornering_compliance;
   rspp_host.speed_correction_factor = host.speed_correction_factor;
   rspp_host.speed_qf = host.speed_qf;
   rspp_host.yaw_rate_qf = host.yaw_rate_qf;
   rspp_host.lat_accel_qf = host.lat_accel_qf;
   rspp_host.long_accel_qf = host.long_accel_qf;
   
}

template<typename RSPP_Core_T, typename Core_Info_T>
static void Set_RSPP_Core_Info(RSPP_Core_T &rspp_core_info, Core_Info_T &core_info)
{
   rspp_core_info.cnt_loops = core_info.cnt_loops;
   rspp_core_info.elapsed_time_s = core_info.elapsed_time_s;
   rspp_core_info.prev_time_us = core_info.prev_time_us;
   rspp_core_info.time_us = core_info.time_us;

}

template<typename T>
static void Init_Sensor_Calibs(T &sensor)
{
   sensor.variable.f_ant_sens_available = false;
   sensor.constant.id = 1;
   sensor.variable.is_valid = true;
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
      - An object created.
*/

int main()
{


   std::cout<< "hi"<<std::endl;
   static f360_variant_C::F360_Tracker f360tracker_C{};
   static f360_variant_C::Input_Diagnostics input_diag_C;
   static f360_variant_C::Output_Diagnostics output_diag_C;
   static f360_variant_C::SafetyControlLogic SCL_C(input_diag_C, output_diag_C);
   static f360_variant_C::State_Manager SM_C(SCL_C, f360tracker_C);
   static f360_variant_C::F360_Host_T host_C{};
   static rspp_variant_C::RSPP_Calibrations_T rspp_calibs_C;
   static rspp_variant_C::RSPP_Detection_List_T detection_list_C{};
   static f360_variant_C::F360_Core_Info_T core_info_C{};
   static rspp_variant_C::F360_Radar_Sensor_T sensors_C[rspp_variant_C::MAX_NUMBER_OF_SENSORS]{};
   static f360_variant_C::F360_Object_Log_Output_T obj_log_C;
   static Tracker_Info_Log_T tracker_info_log_C{};

   //RSPP
   
   static RSPP_Core_Info_T rspp_core_info_C{};
   static RSPP_Host_T rspp_host_C{};


   f360tracker_C.Initialize();
   // host_B.vcs_speed = 10.0f;
   host_C.vcs_speed = 10.0f;

   const uint32_t max_nb_loops = 20U;
   while((0 == tracker_info_log_C.reduced_num_active_objs)
      && (max_nb_loops > core_info_C.cnt_loops))
   {
      std::cout<< "Loop: "<< core_info_C.cnt_loops << std::endl;
      Init_Single_Detection(detection_list_C);

      Advance_Core_Info(core_info_C);

      
      Init_Sensor_Calibs(sensors_C[0]);

      Advance_Sensor(sensors_C[0], core_info_C.time_us);

      Sensor_Motion_Update(rspp_host_C, sensors_C[0]);

      Set_RSPP_Host_Info(rspp_host_C,host_C);
      Set_RSPP_Core_Info(rspp_core_info_C,core_info_C);


      rspp_variant_C::Initialize_RSPP_Calibrations(rspp_calibs_C);

      // rspp_variant_C::Inputs_Preprocessing(rspp_core_info_C,rspp_host_C,sensors_C,rspp_calibs_C,detection_list_C);
      rspp_variant_C::Inputs_Preprocessing(rspp_core_info_C, rspp_host_C, sensors_C, rspp_calibs_C, detection_list_C);





      SM_C.execute(core_info_C, host_C, detection_list_C, reinterpret_cast<f360_variant_C::F360_Radar_Sensor_T(&)[rspp_variant_C::MAX_NUMBER_OF_SENSORS]>(sensors_C), obj_log_C);

   }

   int test_result;
   if (tracker_info_log_C.reduced_num_active_objs > 0)
   {
      test_result = 0;
   }
   else
   {
      test_result = -1;
   }

   std::cout << "test result: " << test_result << std::endl;
   std::cout << "The number of created objects: " << tracker_info_log_C.reduced_num_active_objs << std::endl;
   std::cout << "The number of called loops: " << core_info_C.cnt_loops << std::endl;
   std::cout << "The max number of callled loops: " << max_nb_loops << std::endl;
   return test_result;
}
