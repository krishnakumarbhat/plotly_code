/** \file
    This file contains unit tests for content of f360_sensor_capability.cpp file
*/

#include "f360_sensor_capability.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <cmath>
#include <cstdio>

using namespace f360_variant_A;

/** \defgroup f360_sensor_capability_Get_Uncertainty_Of_Compensated_Range_Rate
 *  @{
 */

 /** \brief
  *  This test group tests the functionality of the function Get_Uncertainty_Of_Compensated_Range_Rate()
  */
TEST_GROUP(f360_sensor_capability_Get_Uncertainty_Of_Compensated_Range_Rate)
{
   float32_t threshold = 0.001F;
   float32_t sens_vel[2] = {18.0F , 0.5F};
   float32_t var_det_rng_rate=0.8F;
   float32_t var_det_az=1.0F;
   float32_t cov_sens_vel[2][2]={ {1.0005e-05F, 2.60488e-06F} ,{2.60488e-06F , 0.00138266F}};
   float32_t var_comp_rng_rate;
};
/**
*\purpose Test Get_Uncertainty_Of_Compensated_Range_Rate Function when both cos and sin of azimuth are inside allowed range
*\req NA
*/   
TEST(f360_sensor_capability_Get_Uncertainty_Of_Compensated_Range_Rate, Test_Cos_Sin_Az_Inside_Range)
{
   /** \precond
    * Setting up cos_det_az to 0.5 and sin_det_az to 0.8
    **/
   float32_t cos_det_az=0.5F;
   float32_t sin_det_az=0.866025F;
   /** \action
    * Call Get_Uncertainty_Of_Compensated_Range_Rate Function
    **/
   Get_Uncertainty_Of_Compensated_Range_Rate(cos_det_az,sin_det_az,sens_vel,var_det_rng_rate,var_det_az,cov_sens_vel,var_comp_rng_rate);
   /** \result
    * Check the expected value for var_comp_rng_rate
    **/
   DOUBLES_EQUAL(var_comp_rng_rate,236.0691F,threshold)
}

/**
*\purpose  Test Get_Uncertainty_Of_Compensated_Range_Rate Function when cos or sin of azimuth is on boundary of allowed range
*\req NA
*/
TEST(f360_sensor_capability_Get_Uncertainty_Of_Compensated_Range_Rate, Test_Cos_Sin_Az_On_Boundary_Of_Range)
{
   /** \precond
    * Setting up cos_det_az to 1 and sin_det_az to 0
    **/
   float32_t cos_det_az=1.0F;
   float32_t sin_det_az=0.0F;
   /** \action
    * Call Get_Uncertainty_Of_Compensated_Range_Rate Function
    **/
   Get_Uncertainty_Of_Compensated_Range_Rate(cos_det_az,sin_det_az,sens_vel,var_det_rng_rate,var_det_az,cov_sens_vel,var_comp_rng_rate);
   /** \result
    * Check the expected value for var_comp_rng_rate
    **/
   DOUBLES_EQUAL(var_comp_rng_rate,1.05001F,threshold)
}

/**
*\purpose  Test Get_Uncertainty_Of_Compensated_Range_Rate Function when cos and sin of azimuth is beyond allowed range
*\req NA
*/
TEST(f360_sensor_capability_Get_Uncertainty_Of_Compensated_Range_Rate, Test_Cos_Sin_Az_Out_Of_Range)
{
   /** \precond
    * Setting up input to function and expected output
    **/
   /* Test with cos and sin out of range */
   float32_t cos_det_az=2.0F;
   float32_t sin_det_az=2.0F;

   /** \action
    * Call Get_Uncertainty_Of_Compensated_Range_Rate Function
    **/
    Get_Uncertainty_Of_Compensated_Range_Rate(cos_det_az,sin_det_az,sens_vel,var_det_rng_rate,var_det_az,cov_sens_vel,var_comp_rng_rate);
   /** \result
    * Check the expected value for var_comp_rng_rate
    **/
   DOUBLES_EQUAL(var_comp_rng_rate,1225.806F,threshold)
}
/** @}*/

/** \defgroup  f360_sensor_capability_Compute_Raw_Detection_Uncertainty
 *  @{
 */

 /** \brief
  *  This test group tests the functionality of the function Compute_Raw_Detection_Uncertainty()
  */
TEST_GROUP(f360_sensor_capability_Compute_Raw_Detection_Uncertainty)
{
   float32_t threshold = 0.001F;
   rspp_variant_A::RSPP_Detection_T det={};
   F360_Radar_Sensor_T sensor={};
   RSPP_Calibrations_T rspp_calibrations={};
   
   TEST_SETUP()
   {
      Initialize_RSPP_Calibrations(rspp_calibrations);
      sensor.constant.fov_min_az_rad[0]=-0.785398185F;
      sensor.constant.fov_max_az_rad[0]=0.785398185F;
      sensor.constant.interior_fov[F360_DET_LOOK_ID_0]=-0.785398185F;
      sensor.constant.interior_fov[F360_DET_LOOK_ID_1]=0.785398185F;
      sensor.constant.interior_fov[F360_DET_LOOK_ID_2]=-0.785398185F;
      sensor.constant.interior_fov[F360_DET_LOOK_ID_3]=0.785398185F;
      rspp_calibrations.k_range_rate_std = 0.06F;
      rspp_calibrations.k_range_std = 0.1F;
      det.raw.azimuth=-0.515454471F;
   }
};
/**
*\purpose  Test Compute_Raw_Detection_Uncertainty Function with MRR360 radar type
*\req   NA
*/
TEST(f360_sensor_capability_Compute_Raw_Detection_Uncertainty, Test_Compute_Raw_Detection_Uncertainty)
{
   /** \precond
   * Setting up input to function and expected output
   **/
   float32_t rng_var;
   float32_t az_var;
   float32_t rng_rate_var;
   sensor.constant.sensor_type=F360_SENSOR_TYPE_MRR360_RADAR;
   sensor.variable.look_id=F360_DET_LOOK_ID_0;
   

   /** \action
   * Call Compute_Raw_Detection_Uncertainty Function
   **/
   Compute_Raw_Detection_Uncertainty(det,sensor, rspp_calibrations,rng_var,az_var,rng_rate_var);
   /** \result
    * Check the expected value for rng_var, az_var and rng_rate_var
    **/
   DOUBLES_EQUAL_TEXT(rng_var,0.01F,threshold, "Range differs from expected value")
   DOUBLES_EQUAL_TEXT(az_var,0.000304617F,threshold, "Azimuth differs from expected value")
   DOUBLES_EQUAL_TEXT(rng_rate_var,0.0036F,threshold, "Range rate differs from expected value")
}

/**
*\purpose  Test Compute_Raw_Detection_Uncertainty Function with MRR3 radar type
*\req   NA
*/
TEST(f360_sensor_capability_Compute_Raw_Detection_Uncertainty, Test_Compute_Raw_Detection_Uncertainty_2)  
{
   /** \precond
   * Setting up input to function and expected output
   **/
   float32_t rng_var;
   float32_t az_var;
   float32_t rng_rate_var;
   sensor.constant.sensor_type=F360_SENSOR_TYPE_MRR3_RADAR;
   sensor.variable.look_id=F360_DET_LOOK_ID_2;

   /** \action
   * Call Compute_Raw_Detection_Uncertainty Function
   **/
   Compute_Raw_Detection_Uncertainty(det,sensor, rspp_calibrations,rng_var,az_var,rng_rate_var);
   /** \result
    * Check the expected value for rng_var, az_var and rng_rate_var
    **/
   DOUBLES_EQUAL_TEXT(rng_var,0.01F,threshold, "Range differs from expected value")
   DOUBLES_EQUAL_TEXT(az_var,7.61544e-05F,threshold, "Azimuth differs from expected value")
   DOUBLES_EQUAL_TEXT(rng_rate_var,0.0036F,threshold, "Range rate differs from expected value")
}
/** @}*/

/** \defgroup f360_sensor_capability_Get_Host_Velocity_Uncertainty
 *  @{
 */

 /** \brief
  *  This test group tests the functionality of the function Get_Host_Velocity_Uncertainty()
  */
TEST_GROUP(f360_sensor_capability_Get_Host_Velocity_Uncertainty)
{
   float32_t threshold = 0.001F;
   F360_Host_T host={};
   float32_t host_speed_var={};
   float32_t host_yaw_rate_var=3.0F;
   float32_t translation_vec[2]={20.0F,1.0F};
   float32_t velocity_cov[2][2]={};
   
   TEST_SETUP()
   {
      host.yaw_rate_rad=-0.00629700907F;
      host.dist_rear_axle_to_vcs_m=3.45000005F;
      host.rear_cornering_compliance=0.00529999984F;
   }
};
/**
*\purpose  Test Get_Host_Velocity_Uncertainty function
*\req   NA
*/
TEST(f360_sensor_capability_Get_Host_Velocity_Uncertainty, Test_with_Forward_speed)
{
   /** \precond
    * Setting up input to function and expected output
    **/
    host_speed_var=40.0F;
    host.speed =34.3173103F;
   /** \action
    * Call Get_Host_Velocity_Uncertainty function
    **/
   Get_Host_Velocity_Uncertainty(host,host_speed_var,host_yaw_rate_var,translation_vec,velocity_cov);
   
   /** \result
    * Check the expected value for velocity_cov
    **/
   DOUBLES_EQUAL(velocity_cov[0][0], 42.9571F,threshold)
   DOUBLES_EQUAL(velocity_cov[0][1],-51.16426F,threshold)
   DOUBLES_EQUAL(velocity_cov[1][0],-51.16426F,threshold)
   DOUBLES_EQUAL(velocity_cov[1][1], 888.3797F,threshold)
}

/**
*\purpose  Test Get_Host_Velocity_Uncertainty function
*\req   NA
*/
TEST(f360_sensor_capability_Get_Host_Velocity_Uncertainty, Test_with_backward_speed)
{   
   /** \precond
    * Setting up the Host speed to negative value
    **/
   host_speed_var=10.0F;
   host.speed =-34.3173103F;
   /** \action
    * Call Get_Host_Velocity_Uncertainty function
    **/
   Get_Host_Velocity_Uncertainty(host,host_speed_var,host_yaw_rate_var,translation_vec,velocity_cov);

   /** \result
    * Check the expected value for velocity_cov
    **/
   DOUBLES_EQUAL(velocity_cov[0][0],13.043F,threshold)
   DOUBLES_EQUAL(velocity_cov[0][1],-52.01689F,threshold)
   DOUBLES_EQUAL(velocity_cov[1][0],-52.01689F,threshold)
   DOUBLES_EQUAL(velocity_cov[1][1],888.3796F,threshold)
}
/** @}*/

/** \defgroup  f360_sensor_capability_Sensor_Capability_Detections
 *  @{
 */

 /** \brief
  *  This test group tests the functionality of the function Sensor_Capability_Detections()
  */
TEST_GROUP(f360_sensor_capability_Sensor_Capability_Detections)
{
   F360_Host_T host={};
   F360_Host_Props_T host_props = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   RSPP_Calibrations_T rspp_calibrations = {};
   F360_TRKR_TIMING_INFO_T timing_info = {}; 
   float32_t threshold = 0.001F;
   
   TEST_SETUP()
   {   
      Initialize_RSPP_Calibrations(rspp_calibrations);
      host.speed =34.3173103F;
      host.yaw_rate_rad=-0.00629700907F;
      host.dist_rear_axle_to_vcs_m=3.45000005F;
      host.rear_cornering_compliance=0.00529999984F;
      for(int i=0;i<2;i++)
      {
         sensors[i].variable.vcs_velocity.longitudinal= 22.1280212402F;
         sensors[i].variable.vcs_velocity.lateral = -13.2757692337F;
         sensors[i].constant.mounting_position.vcs_position.longitudinal=-0.0599999987F;
         sensors[i].constant.mounting_position.vcs_position.lateral=0.100000001F;
         sensors[i].constant.fov_min_az_rad[0]=-0.785398185F;
         sensors[i].constant.fov_max_az_rad[0]=0.785398185F;
         sensors[i].constant.interior_fov[F360_DET_LOOK_ID_0]=-0.785398185F;
         sensors[i].constant.interior_fov[F360_DET_LOOK_ID_1]=0.785398185F;
         sensors[i].constant.interior_fov[F360_DET_LOOK_ID_2]=-0.785398185F;
         sensors[i].constant.interior_fov[F360_DET_LOOK_ID_3]=0.785398185F;
      }
      rspp_calibrations.k_range_rate_std = 0.06F;
      rspp_calibrations.k_range_std = 0.1F;
      host_props.std_speed_scm=0.109195359F;
      host_props.std_yaw_rate_scm=0.00100000005F;
      for (int i=0 ; i<3;i++)
      {
         raw_detection_list.detections[i].processed.cos_vcs_az=0.930880785F;
         raw_detection_list.detections[i].processed.sin_vcs_az=-0.365323097F;
         raw_detection_list.detections[i].raw.range=125.379997F;
         raw_detection_list.detections[i].raw.azimuth=-0.515454471F;
      }
   }
};
/**
*\purpose  Test Sensor_Capability_Detections function
*\req   NA
*/
TEST(f360_sensor_capability_Sensor_Capability_Detections, Test_Sensor_Capability_Detections)
{
   /** \precond
    * Setting up input to function and expected output
    **/
   sensors[0].variable.is_valid =1;
   sensors[1].variable.is_valid =1;
   sensors[2].variable.is_valid =1;
   raw_detection_list.number_of_valid_detections =3;
   raw_detection_list.detections[0].raw.sensor_id =1;
   raw_detection_list.detections[1].raw.sensor_id =1;
   raw_detection_list.detections[2].raw.sensor_id =2;
   /** \action
    * Call Sensor_Capability_Detections function
    **/
   Calculate_Detections_Uncertainties(host, host_props, sensors, rspp_calibrations, raw_detection_list);
   /** \result
    * Check the expected value for std_vcs_az_scm, std_range_rate_compensated_scm ,vcs_position_cov_scm and vcs_cross_covariances_scm
    **/
  for (int i=0 ; i<3;i++)
  {   
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.std_vcs_az_scm,0.0176485,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.std_range_rate_compensated_scm,0.140675,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[0][0],0.66214,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[0][1],1.66172,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[1][0],1.66172,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[1][1],4.24423,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[0],0.0142667,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[1],-0.0609799,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[2],0.036353,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[3],-0.155383,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[4],-0.00133132,threshold)
   }
  for (int i=3 ; i<MAX_NUMBER_OF_DETECTIONS;i++)
  {     
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.std_vcs_az_scm,0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.std_range_rate_compensated_scm,0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[0][0],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[0][1],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[1][0],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_position_cov_scm[1][1],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[0],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[1],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[2],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[3],0,threshold)
      DOUBLES_EQUAL(raw_detection_list.detections[i].processed.vcs_cross_covariances_scm[4],0,threshold)
  }
}
/** @}*/

/** \defgroup  f360_sensor_capability_Sensor_Capability_Host
 *  @{
 */

 /** \brief
  *  This test group tests the functionality of the function Sensor_Capability_Host()
  */
TEST_GROUP(f360_sensor_capability_Sensor_Capability_Host)
{
   F360_Host_T host={};
   F360_Host_Props_T host_props = {};
   float32_t elapsed_time = 0.05F;
   F360_Globals_T globals = {};
   float32_t threshold = 0.001F;
   
   TEST_SETUP()
   {
      host.vehicle_index =6;
      host.speed =34.3173103;
      host.vcs_speed=34.3172951;
      host.acceleration=-6.14509583;
      host.vcs_lat_acceleration=0.0328746177;
      host.vcs_long_acceleration=-6.14509583;
      host.yaw_rate_rad=-0.00629700907;
      host.vcs_sideslip=0.000460191397;
      host.curvature_rear=-0.000164843179;
      host.dist_rear_axle_to_vcs_m=3.45000005;
      host.rear_cornering_compliance=0.00529999984;
      host_props.position.x=6.86346102;
      host_props.position.y=-1.000131496054;
      host_props.heading_angle=-0.000752687454;
      host_props.cos_heading=0.999999702;
      host_props.sin_heading=-0.000752687396;
      host_props.delta_pointing=-0.000326087233;
      host_props.cos_delta_pointing=0.99999994;
      host_props.sin_delta_pointing=-0.000326087233;
      host_props.delta_position.x=1.71586502;
      host_props.delta_position.y=1.000257372303;
      globals.obj_mov_stat_spd_thresh=1.5;
      globals.rear_cornering_compliance=0.00529999984;
      globals.oncoming_speed_thresh=-1.0;
      globals.obj_vehicular_spd_thresh=3.0;
      globals.default_half_length=3.0;
      globals.default_half_width=1.0;
      globals.default_length=6.0;
      globals.default_width=2.0;
      globals.max_otg_speed=1.0;
   }
};

/**
*\purpose  Test Sensor_Capability_Host Function 
*\req   NA
*/
TEST(f360_sensor_capability_Sensor_Capability_Host, Test_Sensor_Capability_Host)
{
   /** \precond
    * Setting up input to function and expected output
    **/
   /* Test with value from resim */
   elapsed_time=1;
   /** \action
    * Call Sensor_Capability_Host function
    **/
   Sensor_Capability_Host(elapsed_time,host, globals.max_otg_speed,host_props);
   /** \result
    * Check the expected value for position_inc_cov_scm, vel_cov_scm, std_speed_scm and std_yaw_rate_scm
    **/
   DOUBLES_EQUAL(host_props.position_inc_cov_scm[0][0],1.0005e-05,threshold)
   DOUBLES_EQUAL(host_props.position_inc_cov_scm[0][1],2.60488e-06,threshold)
   DOUBLES_EQUAL(host_props.position_inc_cov_scm[1][0],2.60488e-06,threshold)
   DOUBLES_EQUAL(host_props.position_inc_cov_scm[1][1],0.001382663,threshold) 
   DOUBLES_EQUAL(host_props.vel_cov_scm[0][0],1.0005e-05,threshold)
   DOUBLES_EQUAL(host_props.vel_cov_scm[0][1],2.60488e-06,threshold)
   DOUBLES_EQUAL(host_props.vel_cov_scm[1][0],2.60488e-06,threshold)
   DOUBLES_EQUAL(host_props.vel_cov_scm[1][1],0.001382663,threshold)
   DOUBLES_EQUAL(host_props.std_speed_scm,0.00316228,threshold)
   DOUBLES_EQUAL(host_props.std_yaw_rate_scm,0.001,threshold)
}
/** @}*/

/** \defgroup  f360_sensor_capability
 *  @{
 */

 /** \brief
  *  This test group tests sensor_capability file 
  */
TEST_GROUP(f360_sensor_capability)
{
};
/**
*\purpose  Describe purpose of test
*\req    NA
*/
TEST(f360_sensor_capability, Test_Compute_Raw_Host_Speed_Uncertainty)
{
   /** \step{1}
    * Testing that Compute_Raw_Host_Speed_Uncertainty() behaves as expected
    **/

    /** \precond
    * Setting up input to function and expected output
    **/
   F360_Host_T host = {};
   const uint32_t num_host_speeds = 22U;
   const uint32_t num_host_yaw_rates = 11U;
   float host_speed_arr[num_host_speeds];
   float host_yaw_rate_arr[num_host_yaw_rates];
   float result_speed_var[num_host_speeds][num_host_yaw_rates];
   float current_speed_var;
   uint32_t host_speed_ind, host_yaw_rate_ind, ind;
   uint32_t comp_host_speed_ind, comp_host_yaw_rate_ind;
   const F360_Globals_T globals = {};

   /* Defining host speed correction factor as it is used in Compute_Raw_Host_Speed_Uncertainty  */
   host.speed_correction_factor = 1.0F;

   for (host_speed_ind = 0U; host_speed_ind < num_host_speeds; host_speed_ind++)
   {
      // Host speeds between -30kph to 400kph
      host_speed_arr[host_speed_ind] = (20.0F*static_cast<float>(host_speed_ind) - 30.0F) / 3.6F; // [m/s]
   }
   for (host_yaw_rate_ind = 0U; host_yaw_rate_ind < num_host_yaw_rates; host_yaw_rate_ind++)
   {
      // Host yaw rates between -25deg/s to 25deg/s
      host_yaw_rate_arr[host_yaw_rate_ind] = (5.0F*static_cast<float>(host_yaw_rate_ind) - 25.0F)*3.14F / 180.0F; // [rad/s]
   }

   char textFailure[200];

   /** \action
   * Call the function
   **/
   for (host_speed_ind = 0U; host_speed_ind < num_host_speeds; host_speed_ind++)
   {
      host.speed = host_speed_arr[host_speed_ind];
      for (host_yaw_rate_ind = 0U; host_yaw_rate_ind < num_host_yaw_rates; host_yaw_rate_ind++)
      {
         host.yaw_rate_rad = host_yaw_rate_arr[host_yaw_rate_ind];

         current_speed_var = Compute_Raw_Host_Speed_Uncertainty(host, globals.max_otg_speed);
         result_speed_var[host_speed_ind][host_yaw_rate_ind] = current_speed_var;
      }
   }

   /** \result
   * Check that output corresponds to following two criteria:
   * 1) speed and yaw rate variance is always >= 0
   * 2) speed and yaw rate variance is increasing (or same but not decreasing)
   *    with abs(speed) and abs(yaw rate)
   **/
   for (host_speed_ind = 0U; host_speed_ind < num_host_speeds; host_speed_ind++)
   {
      for (host_yaw_rate_ind = 0U; host_yaw_rate_ind < num_host_yaw_rates; host_yaw_rate_ind++)
      {
         // Check that variance is >= 0
         (void)sprintf(textFailure, "Speed variance on index host_speed=%d, yaw_rate=%d is less than 0: \n", host_speed_ind, host_yaw_rate_ind);
         CHECK_TEXT(result_speed_var[host_speed_ind][host_yaw_rate_ind] >= 0.0F, textFailure);


         // Check variance is larger or the same if abs(speed) or abs(yaw_rate) is larger. Compare all outputs to each other
         ind = host_speed_ind * num_host_yaw_rates + host_yaw_rate_ind;
         for (uint32_t comp_ind = ind + 1U; comp_ind < num_host_yaw_rates*num_host_speeds; comp_ind++)
         {
            comp_host_speed_ind = static_cast<uint32_t>(static_cast<float>(comp_ind) / static_cast<float>(num_host_yaw_rates));
            comp_host_yaw_rate_ind = comp_ind - comp_host_speed_ind * num_host_yaw_rates;


            if ((fabsf(host_speed_arr[host_speed_ind]) >= fabsf(host_speed_arr[comp_host_speed_ind])) &&
               (fabsf(host_yaw_rate_arr[host_yaw_rate_ind]) >= fabsf(host_yaw_rate_arr[comp_host_yaw_rate_ind])))
            {
               // If abs(speed) and abs(yaw rate) are both larger for ind then we expect variance of ind to be larger or equal
               (void)sprintf(textFailure, "Speed variance on index host_speed=%d, yaw_rate=%d is not larger than variance on index host_speed=%d, yaw_rate=%d despite that abs(speed) and abs(yaw rate) are larger: \n", host_speed_ind, host_yaw_rate_ind, comp_host_speed_ind, comp_host_yaw_rate_ind);
               CHECK_TEXT(result_speed_var[host_speed_ind][host_yaw_rate_ind] >= result_speed_var[comp_host_speed_ind][comp_host_yaw_rate_ind], textFailure);

            }
            else if ((fabsf(host_speed_arr[comp_host_speed_ind]) >= fabsf(host_speed_arr[host_speed_ind])) &&
               (fabsf(host_yaw_rate_arr[comp_host_yaw_rate_ind]) >= fabsf(host_yaw_rate_arr[host_yaw_rate_ind])))
            {
               // If abs(speed) and abs(yaw rate) are both larger for comp_ind then we expect variance of comp_ind to be larger or equal
               (void)sprintf(textFailure, "Speed variance on index host_speed=%d, yaw_rate=%d is not larger than variance on index host_speed=%d, yaw_rate=%d despite that abs(speed) and abs(yaw rate) are larger: \n", host_speed_ind, host_yaw_rate_ind, comp_host_speed_ind, comp_host_yaw_rate_ind);
               CHECK_TEXT(result_speed_var[comp_host_speed_ind][comp_host_yaw_rate_ind] >= result_speed_var[host_speed_ind][host_yaw_rate_ind], textFailure);
            }
            else
            {
               // Output is not easy to predict since it depends on the implementation and
               // tuning of function which could be changed. Therefore no test for this case
            }
         }
      }
   }
}

/**
*\purpose  Test that detection range and azimuth uncertainty is correctly propagated to uncertainty in detection vcs position.
*\req    NA
*/
TEST(f360_sensor_capability, Get_Uncertainty_Of_Detection_Position)
{
   /** \step{1}
    * Testing that Get_Uncertainty_Of_Detection_Position outputs the correct value
    **/

    /** \precond
    * Setting up input to function and expected output
    **/
   const float test_pass_th = 1e-6F;

   const float MY_PI = 3.14159265359F;
   const float MY_PI_4 = MY_PI / 4.0F;
   const float MY_PI_2 = MY_PI / 2.0F;
   const float MY_3_PI_4 = 3.0F*MY_PI / 4.0F;
   const float DEG2RAD = MY_PI / 180.0F;

   const float magnitude = 50;
   const uint32_t num_ang = 9U;
   const float ang_arr[num_ang] = { -MY_PI, -MY_3_PI_4, -MY_PI_2, -MY_PI_4, 0.0F, MY_PI_4, MY_PI_2, MY_3_PI_4, MY_PI };
   const float32_t var_magnitude = 0.1F*0.1F;
   const float32_t var_angle = 1.0F*1.0F*DEG2RAD*DEG2RAD;

   float expected_var_x[num_ang];
   float expected_var_y[num_ang];
   float expected_cov_xy[num_ang];
   for (uint32_t ang_ind = 0U; ang_ind < num_ang; ang_ind++)
   {
      float this_ang = ang_arr[ang_ind];

      expected_var_x[ang_ind] = var_magnitude * cosf(this_ang)*cosf(this_ang) + var_angle * magnitude*magnitude*sinf(this_ang)*sinf(this_ang);
      expected_var_y[ang_ind] = var_magnitude * sinf(this_ang)*sinf(this_ang) + var_angle * magnitude*magnitude*cosf(this_ang)*cosf(this_ang);
      expected_cov_xy[ang_ind] = var_magnitude * cosf(this_ang)*sinf(this_ang) - var_angle * magnitude*magnitude*cosf(this_ang)*sinf(this_ang);
   }

   float cov_xy[2][2];
   float result_var_x[num_ang];
   float result_var_y[num_ang];
   float result_cov_xy[num_ang];
   float result_cov_yx[num_ang];

   /** \action
   * Call the function for some different azimuths
   **/
   for (uint32_t ang_ind = 0U; ang_ind < num_ang; ang_ind++)
   {
      float this_ang = ang_arr[ang_ind];
      float this_cos_ang = cosf(this_ang);
      float this_sin_ang = sinf(this_ang);
      Get_Uncertainty_Of_Detection_Position(
         magnitude,
         this_cos_ang,
         this_sin_ang,
         var_magnitude,
         var_angle,
         cov_xy);

      result_var_x[ang_ind] = cov_xy[0][0];
      result_var_y[ang_ind] = cov_xy[1][1];
      result_cov_xy[ang_ind] = cov_xy[0][1];
      result_cov_yx[ang_ind] = cov_xy[1][0];
   }

   /** \result
    * Check that function has output the expected values
    **/
   for (uint32_t ang_ind = 0U; ang_ind < num_ang; ang_ind++)
   {

      // Check variance of x
      DOUBLES_EQUAL_TEXT(expected_var_x[ang_ind], result_var_x[ang_ind], test_pass_th, "The longitudinal position variance did not match the expected data.")

      // Check variance of y
      DOUBLES_EQUAL_TEXT(expected_var_y[ang_ind], result_var_y[ang_ind], test_pass_th, "The lateral position variance did not match the expected data.")

      // Check covariance of x and y value
      DOUBLES_EQUAL_TEXT(expected_cov_xy[ang_ind], result_cov_xy[ang_ind], test_pass_th, "The cross covariance between x and y position did not match the expected data.")

      // Check returned covariance matrix is symmetric
      DOUBLES_EQUAL_TEXT(expected_cov_xy[ang_ind], result_cov_yx[ang_ind], test_pass_th, "The cross covariance between y and x position did not match the expected data.")
   }
}

/**
*\purpose
  Check if Compute_Detection_Cross_Covariances returns the expected cross variance element.
*\req    NA
*/
TEST(f360_sensor_capability, Test_Compute_Detection_Cross_Covariances)
{
   /** \step{1}
   * Testing that Compute_Detection_Cross_Covariances returns the correct value
   **/

   /** \precond
   * Setting up input to function and expected output
   **/
   rspp_variant_A::RSPP_Detection_T det = {};
   det.raw.range = 20.0F;
   det.raw.range_rate = 10.0F;
   float sens_vel_vcs[2] = { 20.0, 0.0 };
   det.processed.vcs_az = 0.1F;
   det.processed.cos_vcs_az = 0.995004165278026F; // Cos(0.1)
   det.processed.sin_vcs_az = 0.099833416646828F; // Sin(0.1)
   const float var_azimuth_vcs = 0.2F;
   char textFailure[100];
   // Expected value is computed externally in Matlab
   float expected_vcs_cross_variance[5] = {-0.3993337F, 0.7973369F, 3.9800167F, -7.9467732F, -0.3993337F};
   /** \action
   * Call the function Compute_Detection_Cross_Covariances
   **/
   Compute_Detection_Cross_Covariances(sens_vel_vcs, var_azimuth_vcs, det);

   /** \result
    * Check that function has output the expected values
    **/
   for (uint8_t i = 0U; i < 5U; i++)
   {
      (void)sprintf(textFailure, "Returned cross variance value failed on element n=%d failed: \n", i);
      DOUBLES_EQUAL_TEXT(expected_vcs_cross_variance[i], det.processed.vcs_cross_covariances_scm[i], 1e-6f, textFailure);
   }

   /** \precond
   * Setting up another host heading angle and sensor motion as input to function and expected output
   **/
   sens_vel_vcs[0] = 18.0F;
   sens_vel_vcs[1] = 0.5F;

   // Expected value is computed externally in Matlab
   float expected_vcs_cross_variance_sec[5] = {-0.3993337F, 0.5189339F, 3.9800167F, -5.1720293F, -0.2598999F};
   /** \action
   * Call the function Compute_Detection_Cross_Covariances
   **/
   Compute_Detection_Cross_Covariances(sens_vel_vcs, var_azimuth_vcs, det);

   /** \result
   * Check that function has output the expected values
   **/
   for (uint8_t i = 0U; i < 5U; i++)
   {
      (void)sprintf(textFailure, "Returned cross variance value failed on element n=%d failed: \n", i);
      DOUBLES_EQUAL_TEXT(expected_vcs_cross_variance_sec[i], det.processed.vcs_cross_covariances_scm[i], 1e-6f, textFailure);
   }

   /** \precond
   * Setting up an negative host heading angle as input to function and expected output
   **/

   // Expected value is computed externally in Matlab
   float expected_vcs_cross_variance_rd[5] = {-0.3993337F, 0.5189339F, 3.9800167F, -5.1720293F, -0.2598999F};
   /** \action
   * Call the function Compute_Detection_Cross_Covariances
   **/
   Compute_Detection_Cross_Covariances(sens_vel_vcs, var_azimuth_vcs, det);

   /** \result
   * Check that function has output the expected values
   **/
   for (uint8_t i = 0U; i < 5U; i++)
   {
      (void)sprintf(textFailure, "Returned cross variance value failed on element n=%d failed: \n", i);
      DOUBLES_EQUAL_TEXT(expected_vcs_cross_variance_rd[i], det.processed.vcs_cross_covariances_scm[i], 1e-6f, textFailure);
   }

}
/** @}*/