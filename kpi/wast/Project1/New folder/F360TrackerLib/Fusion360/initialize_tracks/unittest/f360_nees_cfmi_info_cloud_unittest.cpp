/** \file
 * This file contains unit tests for content of f360_nees_cfmi_info_cloud.cpp file
 */

#include "f360_nees_cfmi_info_cloud.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_Calc_NEES_CFMI_Determinants_Cloud_test_group
 *  @{
 */

/** \brief
 * Test group for Calc_NEES_CFMI_Determinants_Cloud testing.
 */
TEST_GROUP(f360_Calc_NEES_CFMI_Determinants_Cloud_test_group)
{	
   F360_NEES_CFMI_Detection_T detection = {};
   const float32_t tolerance = 1e-6F;
   
   /** \setup
    * Reset detection state.
    */
   TEST_SETUP()
   {
      detection = F360_NEES_CFMI_Detection_T();
   }

   void Calculate_Detection_Azimuth_Sin_And_Cos(float32_t det_azimuth)
   {
      detection.sin_vcs_az = F360_Sinf(det_azimuth);
      detection.cos_vcs_az = F360_Cosf(det_azimuth);
   }
};

/** \purpose  
 * Test Calc_NEES_CFMI_Determinants_Cloud() behavior when range rate compensated variance is above minimal value.
 * \req
 * NA
 */
TEST(f360_Calc_NEES_CFMI_Determinants_Cloud_test_group, Rrate_Comp_Var_Is_Valid)
{
   /** \precond
    * Set detection state.
    */
   const float32_t det_azimuth = F360_DEG2RAD(45.0F);
   const float32_t rrate_comp = 1.0F;
   const float32_t rrate_comp_var = 1.0F;

   Calculate_Detection_Azimuth_Sin_And_Cos(det_azimuth);
   detection.range_rate_comp = rrate_comp;
   detection.range_rate_comp_var = rrate_comp_var;

   /** \action
    * Call Calc_NEES_CFMI_Determinants_Cloud()
    */
   Calc_NEES_CFMI_Determinants_Cloud(detection);

   /** \result
    * Compare determinants with expected results.
    */
   
   DOUBLES_EQUAL(1.0F, detection.cloud_determinants.information, tolerance);
   DOUBLES_EQUAL(0.5F, detection.cloud_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(0.5F, detection.cloud_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(F360_Cosf(det_azimuth), detection.cloud_determinants.Sx, tolerance);
   DOUBLES_EQUAL(0.5F, detection.cloud_determinants.Syy, tolerance);
   DOUBLES_EQUAL(F360_Sinf(det_azimuth), detection.cloud_determinants.Sy, tolerance);
}

/** \purpose
 * Test Calc_NEES_CFMI_Determinants_Cloud() behavior when range rate compensated variance is below minimal value.
 * \req
 * NA
 */
TEST(f360_Calc_NEES_CFMI_Determinants_Cloud_test_group, Rrate_Comp_Var_Is_Invalid)
{
   /** \precond
    * Set detection state.
    */
   const float32_t det_azimuth = F360_DEG2RAD(45.0F);
   const float32_t rrate_comp = 1.0F;
   const float32_t rrate_comp_var = F360_MIN_DENOMINATOR * 0.9F;

   Calculate_Detection_Azimuth_Sin_And_Cos(det_azimuth);
   detection.range_rate_comp = rrate_comp;
   detection.range_rate_comp_var = rrate_comp_var;

   /** \action
    * Call Calc_NEES_CFMI_Determinants_Cloud()
    */
   Calc_NEES_CFMI_Determinants_Cloud(detection);

   /** \result
    * All determinants should be equal to default value - 0.0.
    */

   DOUBLES_EQUAL(0.0F, detection.cloud_determinants.information, tolerance);
   DOUBLES_EQUAL(0.0F, detection.cloud_determinants.Sxx, tolerance);
   DOUBLES_EQUAL(0.0F, detection.cloud_determinants.Sxy, tolerance);
   DOUBLES_EQUAL(0.0F, detection.cloud_determinants.Sx, tolerance);
   DOUBLES_EQUAL(0.0F, detection.cloud_determinants.Syy, tolerance);
   DOUBLES_EQUAL(0.0F, detection.cloud_determinants.Sy, tolerance);
}
/** @}*/

/** \defgroup  f360_Calc_NEES_CFMI_Determinants_Cloud_test_group
 *  @{
 */

 /** \brief
  * Test group for Update_NEES_Detection_Slot testing.
  */
TEST_GROUP(Update_NEES_Detection_Slot_test_group)
{
   F360_NEES_CFMI_Detection_T nees_detection_slot = {};
   F360_Calibrations_T calibrations;
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   float32_t det_time_since_measurement = 0.0F;
   Point detection_position = {};
   uint32_t det_index = 0U;
   float32_t det_rrate_comp = 0.0F;
   float32_t det_azimuth = 0.0F;
   F360_Detection_Wheelspin_Type_T det_wheelspin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_INVALID;
   bool det_is_historical = false;
   float32_t det_pos_cov[2][2] = {};

   /** \setup
    * Reset input variables state.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/** \purpose
 * Test Update_NEES_Detection_Slot() behavior when processed detection is not wheelspin.
 * \req
 * NA
 */
TEST(Update_NEES_Detection_Slot_test_group, Detection_Is_Not_Wheelspin)
{
   /** \precond
    * Set input arguments.
    */
   det_wheelspin_type = F360_DETECTION_WHEELSPIN_TYPE_INVALID;

   /** \action
    * Call Update_NEES_Detection_Slot()
    */
   Update_NEES_Detection_Slot(calibrations,
      det_time_since_measurement,
      detection_position,
      det_pos_cov,
      det_index,
      0U,
      det_rrate_comp,
      det_azimuth,
      det_wheelspin_type,
      det_motion_status,
      det_is_historical, 
      nees_detection_slot,
      nees_cfmi_information);

   /** \result
    * Compare nees structures expected results.
    */

   CHECK_TRUE(nees_detection_slot.f_inlier);
   CHECK_EQUAL(1U, nees_cfmi_information.init_dets_inliers_num);
}

/** \purpose
 * Test Update_NEES_Detection_Slot() behavior when processed detection is not wheelspin.
 * \req
 * NA
 */
TEST(Update_NEES_Detection_Slot_test_group, Detection_Is_Wheelspin)
{
   /** \precond
    * Set input arguments.
    */
   det_wheelspin_type = F360_DETECTION_WHEELSPIN_TYPE_OBJECT;

   /** \action
    * Call Update_NEES_Detection_Slot()
    */
   Update_NEES_Detection_Slot(calibrations,
      det_time_since_measurement,
      detection_position,
      det_pos_cov,
      det_index,
      0U,
      det_rrate_comp,
      det_azimuth,
      det_wheelspin_type,
      det_motion_status,
      det_is_historical,
      nees_detection_slot,
      nees_cfmi_information);

   /** \result
    * Compare nees structures expected results.
    */

   CHECK_FALSE(nees_detection_slot.f_inlier);
   CHECK_EQUAL(0U, nees_cfmi_information.init_dets_inliers_num);
}

/** \purpose
 * Test Update_NEES_Detection_Slot() behavior when processed detection's motion status is equal to moving.
 * \req
 * NA
 */
TEST(Update_NEES_Detection_Slot_test_group, Detection_Is_Moving)
{
   /** \precond
    * Set input arguments.
    */
   det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   /** \action
    * Call Update_NEES_Detection_Slot()
    */
   Update_NEES_Detection_Slot(calibrations,
      det_time_since_measurement,
      detection_position,
      det_pos_cov,
      det_index,
      0U,
      det_rrate_comp,
      det_azimuth,
      det_wheelspin_type,
      det_motion_status,
      det_is_historical,
      nees_detection_slot,
      nees_cfmi_information);

   /** \result
    * Compare nees structures expected results.
    */
   CHECK_EQUAL(1U, nees_cfmi_information.moving_dets_num);
}
/** @}*/

/** \defgroup  Get_NEES_CFMI_Information_Cloud_Current_Dets_test_group
 *  @{
 */

 /** \brief
  * Test group for Get_NEES_CFMI_Information_Cloud_Current_Dets testing.
  */
TEST_GROUP(Get_NEES_CFMI_Information_Cloud_Current_Dets_test_group)
{
   F360_Calibrations_T calibrations = {};
   F360_Cluster_T cluster_to_init = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};

   /** \setup
    * Reset input variables state.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};
/** \purpose
 * Test Get_NEES_CFMI_Information_Cloud_Current_Dets() behavior when number of detections in nees structure is equal to maximum.
 * \req
 * NA
 */
TEST(Get_NEES_CFMI_Information_Cloud_Current_Dets_test_group, nees_dets_over_maximum_value)
{
   /** \precond
    * Set input arguments.
    */
   uint32_t input_num_dets = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET;
   nees_cfmi_information.dets_num = input_num_dets;
   cluster_to_init.ndets = 5;

   /** \action
    * Call Get_NEES_CFMI_Information_Cloud_Current_Dets()
    */
   Get_NEES_CFMI_Information_Cloud_Current_Dets(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, nees_cfmi_information);

   /** \result
    * Number of detections should not change.
    */
   CHECK_EQUAL(input_num_dets, nees_cfmi_information.dets_num);
}

/** \purpose
 * Test Get_NEES_CFMI_Information_Cloud_Current_Dets() behavior when number of detections in cluster is below the max value.
 * \req
 * NA
 */
TEST(Get_NEES_CFMI_Information_Cloud_Current_Dets_test_group, nees_dets_gets_saturated)
{
   /** \precond
    * Set input arguments.
    */
   uint32_t input_num_dets = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET - 1U;
   nees_cfmi_information.dets_num = input_num_dets;
   cluster_to_init.ndets = 5;
   cluster_to_init.detids[0] = 1;
   cluster_to_init.detids[1] = 2;
   cluster_to_init.detids[2] = 3;
   cluster_to_init.detids[3] = 4;
   cluster_to_init.detids[4] = 5;

   /** \action
    * Call Get_NEES_CFMI_Information_Cloud_Current_Dets()
    */
   Get_NEES_CFMI_Information_Cloud_Current_Dets(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, nees_cfmi_information);

   /** \result
    * Number of detections should saturate at max possible value.
    */
   CHECK_EQUAL(F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET, nees_cfmi_information.dets_num);
}
/** @}*/

/** \defgroup  Get_NEES_CFMI_Information_Cloud_Old_Dets_test_group
 *  @{
 */

 /** \brief
  * Test group for Get_NEES_CFMI_Information_Cloud_Old_Dets testing.
  */
TEST_GROUP(Get_NEES_CFMI_Information_Cloud_Old_Dets_test_group)
{
   F360_Calibrations_T calibrations = {};
   F360_Cluster_T cluster_to_init = {};
   F360_Detection_Hist_T detection_hist = {};
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};

   /** \setup
    * Reset input variables state.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      cluster_to_init = {};
      detection_hist = {};
      nees_cfmi_information = {};
   }
};
/** \purpose
 * Test Get_NEES_CFMI_Information_Cloud_Old_Dets() behavior when number of detections in nees structure is equal to maximum.
 * \req
 * NA
 */
TEST(Get_NEES_CFMI_Information_Cloud_Old_Dets_test_group, nees_dets_over_maximum_value)
{
   /** \precond
    * Set input arguments.
    */
   uint32_t input_num_dets = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET;
   nees_cfmi_information.dets_num = input_num_dets;
   cluster_to_init.num_old_dets = 5;

   /** \action
    * Call Get_NEES_CFMI_Information_Cloud_Old_Dets()
    */
   Get_NEES_CFMI_Information_Cloud_Old_Dets(calibrations, cluster_to_init, detection_hist, nees_cfmi_information);

   /** \result
    * Number of detections should not change.
    */
   CHECK_EQUAL(input_num_dets, nees_cfmi_information.dets_num);
}

/** \purpose
 * Test Get_NEES_CFMI_Information_Cloud_Old_Dets() behavior when number of detections in cluster is below the max value.
 * \req
 * NA
 */
TEST(Get_NEES_CFMI_Information_Cloud_Old_Dets_test_group, nees_dets_gets_saturated)
{
   /** \precond
    * Set input arguments.
    */
   uint32_t input_num_dets = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET - 1U;
   nees_cfmi_information.dets_num = input_num_dets;
   cluster_to_init.num_old_dets = 5;

   /** \action
    * Call Get_NEES_CFMI_Information_Cloud_Old_Dets()
    */
   Get_NEES_CFMI_Information_Cloud_Old_Dets(calibrations, cluster_to_init, detection_hist, nees_cfmi_information);

   /** \result
    * Number of detections should saturate at max possible value.
    */
   CHECK_EQUAL(F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET, nees_cfmi_information.dets_num);
}
/** @}*/
