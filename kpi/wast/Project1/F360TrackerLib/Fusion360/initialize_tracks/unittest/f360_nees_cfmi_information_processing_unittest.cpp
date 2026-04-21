/** \file
 * This file contains unit tests for content of f360_nees_cfmi_information_processing.cpp file
 */

#include "f360_nees_cfmi_information_processing.h"
#include "f360_initialization_data_generator_support_functions.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

using namespace f360_variant_A;

/** \defgroup  Calculate_Detections_Mean_Center_test_group
 *  @{
 */

 /** \brief
  * Test group for Calculate_Detections_Mean_Center testing.
  */
TEST_GROUP(Calculate_Detections_Mean_Center_test_group)
{
   F360_Calibrations_T calibrations;
   F360_Cluster_T cluster_to_init;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   F360_NEES_CFMI_Information_T nees_cfmi_information;
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};

   const float32_t tolerance = 1e-5F;

   /** \setup
    * Reset detection state.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
      nees_cfmi_information = {};
      cluster_to_init = {};
      *det_props = {};
   }

   F360_Detection_Props_T Get_Detection(const float32_t pos_long, const float32_t pos_lat)
   {
      F360_Detection_Props_T detection = {};
      detection.vcs_position = { pos_long, pos_lat };
      return detection;
   }

   void Add_Detection_To_Cluster(const uint32_t det_index)
   {
      cluster_to_init.detids[cluster_to_init.ndets] = det_index + 1U;
      cluster_to_init.ndets++;
   }
};

/** \purpose
 * Test Calculate_Detections_Mean_Center() behavior with three detections with specified positions.
 * \req
 * NA
 */
TEST(Calculate_Detections_Mean_Center_test_group, Three_Detections_Case)
{
   /** \precond
    * Set cluster's detections states.
    */
   const float32_t expected_pos_lat = 10.0F;
   const float32_t expected_pos_long = 10.0F;
   const float32_t time_since_measurement = 0.05F;

   det_props[0U] = Get_Detection(expected_pos_lat, expected_pos_long);
   det_props[1U] = Get_Detection(expected_pos_lat + 0.1, expected_pos_long + 0.1);
   det_props[2U] = Get_Detection(expected_pos_lat - 0.1, expected_pos_long - 0.1);
   sensors[0].variable.time_since_measurement_s = time_since_measurement;
   raw_detection_list.detections[0].raw.sensor_id = 1;
   raw_detection_list.detections[1].raw.sensor_id = 1;
   raw_detection_list.detections[2].raw.sensor_id = 1;

   Add_Detection_To_Cluster(0U);
   Add_Detection_To_Cluster(1U);
   Add_Detection_To_Cluster(2U);

   /** \action
    * Call Calculate_Detections_Mean_Center()
    */
   Calculate_Detections_Mean_Center(calibrations, cluster_to_init, raw_detection_list, det_props, sensors, nees_cfmi_information);

   /** \result
    * Compare position and min_time_since_measurement with expected results.
    */

   DOUBLES_EQUAL(expected_pos_lat, nees_cfmi_information.pos_center.y, tolerance);
   DOUBLES_EQUAL(expected_pos_long, nees_cfmi_information.pos_center.x, tolerance);
   DOUBLES_EQUAL(time_since_measurement, nees_cfmi_information.min_time_since_meas, tolerance);
}
/** @}*/

/** \defgroup  Calculate_Mean_Range_Rate_Comp_Variance_test_group
 *  @{
 */

 /** \brief
  * Test group for Calculate_Mean_Range_Rate_Comp_Variance testing.
  */
TEST_GROUP(Calculate_Mean_Range_Rate_Comp_Variance_test_group)
{
   F360_NEES_CFMI_Information_T nees_cfmi_information;
   const float32_t tolerance = 1e-5F;

   /** \setup
    * Reset nees structure.
    */
   TEST_SETUP()
   {
      nees_cfmi_information = {};
   }

   void Add_Detection_Rrate_Comp_Var_Info_To_Nees_Structure(const float32_t detection_rrate_comp_var)
   {
      const uint32_t det_index = nees_cfmi_information.dets_num;
      nees_cfmi_information.detections[det_index].range_rate_comp_var = detection_rrate_comp_var;
      nees_cfmi_information.dets_num++;
   }
};

/** \purpose
 * Test Calculate_Mean_Range_Rate_Comp_Variance() behavior with three three detections' range rates variances specified.
 * \req
 * NA
 */
TEST(Calculate_Mean_Range_Rate_Comp_Variance_test_group, Three_Detections_Case)
{
   /** \precond
    * Add three detections' range rates variances to nees structure.
    */
   const float32_t expected_rrate_var = 0.3F;

   Add_Detection_Rrate_Comp_Var_Info_To_Nees_Structure(expected_rrate_var);
   Add_Detection_Rrate_Comp_Var_Info_To_Nees_Structure(expected_rrate_var + 0.05F);
   Add_Detection_Rrate_Comp_Var_Info_To_Nees_Structure(expected_rrate_var - 0.05F);

   /** \action
    * Call Calculate_Mean_Range_Rate_Comp_Variance()
    */
   Calculate_Mean_Range_Rate_Comp_Variance(nees_cfmi_information);

   /** \result
    * Compare mean variance with expected results.
    */

   DOUBLES_EQUAL(expected_rrate_var, nees_cfmi_information.mean_rr_comp_var, tolerance);
}

/** \purpose
 * Test Calculate_Mean_Range_Rate_Comp_Variance() behavior with 0 detections.
 * \req
 * NA
 */
TEST(Calculate_Mean_Range_Rate_Comp_Variance_test_group, Zero_Detections_Case)
{
   /** \precond
    * Set detections number within nees structure to be 0.
    */
   const float32_t expected_rrate_var = 0.0F;
   nees_cfmi_information.dets_num = 0U;

   /** \action
    * Call Calculate_Mean_Range_Rate_Comp_Variance()
    */
   Calculate_Mean_Range_Rate_Comp_Variance(nees_cfmi_information);

   /** \result
    * Compare mean variance with expected results.
    */
   DOUBLES_EQUAL(expected_rrate_var, nees_cfmi_information.mean_rr_comp_var, tolerance);
}

/** \purpose
 * Test Calculate_Mean_Range_Rate_Comp_Variance() when number of detections exceed max value.
 * \req
 * NA
 */
TEST(Calculate_Mean_Range_Rate_Comp_Variance_test_group, Detections_Number_Exceeds_Max)
{
   /** \precond
    * Fill NEES structure with some detections' range rates variances. Set number of detections to exceed max value.
    */
   const float32_t expected_rrate_var = 0.0F;
   for (uint16_t det_index = 0U; det_index < F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET; det_index++)
   {
      Add_Detection_Rrate_Comp_Var_Info_To_Nees_Structure(1.0F);
   }
   nees_cfmi_information.dets_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_DET + 1U;

   /** \action
    * Call Calculate_Mean_Range_Rate_Comp_Variance()
    */
   Calculate_Mean_Range_Rate_Comp_Variance(nees_cfmi_information);

   /** \result
    * Compare mean variance with expected results.
    */
   DOUBLES_EQUAL(expected_rrate_var, nees_cfmi_information.mean_rr_comp_var, tolerance);
}
/** @}*/

/** \defgroup  Calculate_Mean_Velocity_Covariance_test_group
 *  @{
 */

 /** \brief
  * Test group for Calculate_Mean_Velocity_Covariance testing.
  */
TEST_GROUP(Calculate_Mean_Velocity_Covariance_test_group)
{
   F360_NEES_CFMI_Information_T nees_cfmi_information;
   const float32_t tolerance = 1e-5F;

   /** \setup
    * Reset nees structure.
    */
   TEST_SETUP()
   {
      nees_cfmi_information = {};
   }

   void Add_Velocity_With_Covariance_To_Nees_Structure(float32_t cov_xx, float32_t cov_xy, float32_t cov_yy)
   {
      const uint32_t vel_index = nees_cfmi_information.vels_num;
      nees_cfmi_information.velocities[vel_index].vel_cov[F360_2D_IDX_X][F360_2D_IDX_X] = cov_xx;
      nees_cfmi_information.velocities[vel_index].vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y] = cov_xy;
      nees_cfmi_information.velocities[vel_index].vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X] = cov_xy;
      nees_cfmi_information.velocities[vel_index].vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y] = cov_yy;
      nees_cfmi_information.vels_num++;
   }
};

/** \purpose
 * Test Calculate_Mean_Velocity_Covariance() behavior with 0 position difference based velocities.
 * \req
 * NA
 */
TEST(Calculate_Mean_Velocity_Covariance_test_group, Zero_Velocities_Case)
{
   /** \precond
    * NEES position velocities number should be 0.
    */
   nees_cfmi_information.vels_num = 0U;
   const float32_t expected_cov_xx = 0.0F;
   const float32_t expected_cov_xy = 0.0F;
   const float32_t expected_cov_yx = 0.0F;
   const float32_t expected_cov_yy = 0.0F;
   /** \action
    * Call Calculate_Mean_Velocity_Covariance()
    */
   Calculate_Mean_Velocity_Covariance(nees_cfmi_information);

   /** \result
    * Compare covariance matrix elements with expected values.
    */
   DOUBLES_EQUAL(expected_cov_xx, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov_xy, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_cov_yx, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov_yy, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
 * Test Calculate_Mean_Velocity_Covariance() behavior with 3 position difference based velocities.
 * \req
 * NA
 */
TEST(Calculate_Mean_Velocity_Covariance_test_group, Three_Velocities_Common_Case)
{
   /** \precond
    * Fill nees structure with three velocities with defined covariances.
    */
   const float32_t expected_cov_xx = 1.0F;
   const float32_t expected_cov_xy = 0.1F;
   const float32_t expected_cov_yx = 0.1F;
   const float32_t expected_cov_yy = 0.7F;

   Add_Velocity_With_Covariance_To_Nees_Structure(expected_cov_xx, expected_cov_xy, expected_cov_yy);
   Add_Velocity_With_Covariance_To_Nees_Structure(expected_cov_xx + 0.1F, expected_cov_xy + 0.15, expected_cov_yy);
   Add_Velocity_With_Covariance_To_Nees_Structure(expected_cov_xx - 0.1F, expected_cov_xy - 0.15, expected_cov_yy);

   /** \action
    * Call Calculate_Mean_Velocity_Covariance()
    */
   Calculate_Mean_Velocity_Covariance(nees_cfmi_information);

   /** \result
    * Compare covariance matrix elements with expected values.
    */
   DOUBLES_EQUAL(expected_cov_xx, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov_xy, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_cov_yx, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov_yy, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}

/** \purpose
 * Test Calculate_Mean_Velocity_Covariance() behavior when number of position based velocities is exceeded.
 * \req
 * NA
 */
TEST(Calculate_Mean_Velocity_Covariance_test_group, Velocities_Number_Exceeded)
{
   /** \precond
    * Fill nees structure with velocities with defined covariances. Set velocity number to exceed maximum value.
    */
   for (uint16_t vel_index=0U; vel_index < F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL; vel_index++)
   {
      Add_Velocity_With_Covariance_To_Nees_Structure(1.0F, 0.0F, 1.0F);
   }
   nees_cfmi_information.vels_num = F360_NEES_COST_FUNCTION_INFORMATION_MAX_VEL + 1U;

   const float32_t expected_cov_xx = 0.0F;
   const float32_t expected_cov_xy = 0.0F;
   const float32_t expected_cov_yx = 0.0F;
   const float32_t expected_cov_yy = 0.0F;

   /** \action
    * Call Calculate_Mean_Velocity_Covariance()
    */
   Calculate_Mean_Velocity_Covariance(nees_cfmi_information);

   /** \result
    * Compare covariance matrix elements with expected values.
    */
   DOUBLES_EQUAL(expected_cov_xx, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov_xy, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_X][F360_2D_IDX_Y], tolerance);
   DOUBLES_EQUAL(expected_cov_yx, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_X], tolerance);
   DOUBLES_EQUAL(expected_cov_yy, nees_cfmi_information.mean_vel_cov[F360_2D_IDX_Y][F360_2D_IDX_Y], tolerance);
}
/** @}*/
