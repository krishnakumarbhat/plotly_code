/** \file
 * This file contains unit tests for content of f360_static_environment_class.cpp file
 */

#include "f360_static_environment_class.h"
#include <CppUTest/TestHarness.h>

#include "f360_lsc_data_generator.h"
#include <limits>

using namespace f360_variant_A;

/** \defgroup  f360_static_environment_class
 *  @{
 */

 /** \brief
  * Test group that holds test related to public methods of class Static_Env_T
  */
TEST_GROUP(f360_static_environment_class)
{

   Static_Env_T static_env;
   F360_Host_T host;

};

/** \purpose
 * Purpose of test is to verify the class's get and set method.
 * \req
 * NA
 */
TEST(f360_static_environment_class, Verify_Get_And_Set_Method)
{
   /** \precond
    * Define arbitrary polynomials
    */
   Static_Env_Poly_T static_env_poly_in[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   for (uint8_t i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      static_env_poly_in[i].age = i;
      static_env_poly_in[i].confidence = 1.0F;
      static_env_poly_in[i].lower_limit = -10.0F;
      static_env_poly_in[i].upper_limit = 10.0F;
      static_env_poly_in[i].p0 = -1.0F;
      static_env_poly_in[i].p1 = 2.0F;
      static_env_poly_in[i].p2 = 1.0F;
      static_env_poly_in[i].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      static_env_poly_in[i].poly_type = F360_STATIC_ENV_POLY_TYPE_LSC;
   }

   /** \action
    * Call set and get functions
    */
   static_env.Set_Polynomials(static_env_poly_in);

   const Static_Env_Poly_T(&static_env_poly_out)[F360_NUM_OF_STATIC_ENV_POLYS] = static_env.Get_Polynomials();

   /** \result
    * Compare that "Set data" is equal to "Get data"
    */
   for (uint8_t i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      CHECK_EQUAL(static_env_poly_in[i].confidence, static_env_poly_out[i].confidence);
      CHECK_EQUAL(static_env_poly_in[i].age, static_env_poly_out[i].age);
      CHECK_EQUAL(static_env_poly_in[i].lower_limit, static_env_poly_out[i].lower_limit);
      CHECK_EQUAL(static_env_poly_in[i].upper_limit, static_env_poly_out[i].upper_limit);
      CHECK_EQUAL(static_env_poly_in[i].p0, static_env_poly_out[i].p0);
      CHECK_EQUAL(static_env_poly_in[i].p1, static_env_poly_out[i].p1);
      CHECK_EQUAL(static_env_poly_in[i].p2, static_env_poly_out[i].p2);
      CHECK_EQUAL(static_env_poly_in[i].status, static_env_poly_out[i].status);
      CHECK_EQUAL(static_env_poly_in[i].poly_type, static_env_poly_out[i].poly_type);
   }
}

/** \purpose
 * Purpose of test is to verify that static environment polynomials are reset correctly.
 * \req
 * NA
 */
TEST(f360_static_environment_class, Initialize_Static_Env__Verify_Correct_Reset)
{
   /** \precond
    * Define arbitrary polynomials
    */
   Static_Env_Poly_T static_env_poly_in[F360_NUM_OF_STATIC_ENV_POLYS] = {};
   for (uint8_t i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      static_env_poly_in[i].age = i;
      static_env_poly_in[i].confidence = 1.0F;
      static_env_poly_in[i].lower_limit = -10.0F;
      static_env_poly_in[i].upper_limit = 10.0F;
      static_env_poly_in[i].p0 = -1.0F;
      static_env_poly_in[i].p1 = 2.0F;
      static_env_poly_in[i].p2 = 1.0F;
      static_env_poly_in[i].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      static_env_poly_in[i].poly_type = F360_STATIC_ENV_POLY_TYPE_LSC;
   }

   /** \action
    * Set arbitrary data in the private member through Set_Polynomials()
    * Call function Initialize_Static_Env()
    * Get data in private member from call to Get_Polynomials()
    */
   static_env.Set_Polynomials(static_env_poly_in);

   static_env.Initialize_Static_Env();

   const Static_Env_Poly_T(&static_env_poly_out)[F360_NUM_OF_STATIC_ENV_POLYS] = static_env.Get_Polynomials();

   /** \result
    * Compare that data have been reset
    */
   for (uint8_t i = 0U; i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      CHECK_EQUAL(0.0F, static_env_poly_out[i].confidence);
      CHECK_EQUAL(0U, static_env_poly_out[i].age);
      CHECK_EQUAL(0.0F, static_env_poly_out[i].lower_limit);
      CHECK_EQUAL(0.0F, static_env_poly_out[i].upper_limit);
      CHECK_EQUAL(0.0F, static_env_poly_out[i].p0);
      CHECK_EQUAL(0.0F, static_env_poly_out[i].p1);
      CHECK_EQUAL(0.0F, static_env_poly_out[i].p2);
      CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, static_env_poly_out[i].status);
      CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_INVALID, static_env_poly_out[i].poly_type);
   }
}

/** \purpose
 * Purpose of test is to verify that static environment polynomials are updated by call to
 * function. This test also implicitly tests the private method Map_LSC_To_Static_Env_Poly() inside
 * the static environment class.
 * \req
 * NA
 */
TEST(f360_static_environment_class, Run_Longi_Stat_Curves__Verify_Stat_Environment_Is_Updated)
{
   /** \precond
    * Initialize tracker_info structure before creating groups of
    * object which in turn will generate 3 valid LSC's. LSC module will
    * prioritize the valid polynomials first. Group A, B and E will create LSC's.
    * Initialize tracker calibrations
    * Define a test pass threshold close to 0
    */
   F360_Tracker_Info_T tracker_info;
   Initialize_Tracker_Info(tracker_info);

   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_LSC_Object_Group_Settings_T group_a = Add_LSC_Group_A(tracker_info, objects);
   F360_LSC_Object_Group_Settings_T group_b = Add_LSC_Group_B(tracker_info, objects);
   F360_LSC_Object_Group_Settings_T group_e = Add_LSC_Group_E(tracker_info, objects);

   F360_Calibrations_T calibs;
   Initialize_Tracker_Calibrations(calibs);

   F360_TRKR_TIMING_INFO_T timing_info;

   float32_t test_pass_thres = 0.0001F;

   /** \action
    * Call function Run_Longi_Stat_Curves()
    * Get data in private member from call to Get_Polynomials()
    */
   static_env.Run_Longi_Stat_Curves(tracker_info, calibs, host, objects, timing_info);

   const Static_Env_Poly_T(&static_env_poly_out)[F360_NUM_OF_STATIC_ENV_POLYS] = static_env.Get_Polynomials();

   /** \result
    * Compare that data have been mapped as expected
    * 3 valid LSC's have been mapped to the static environment private member
    */
   DOUBLES_EQUAL(1.0F, static_env_poly_out[0].confidence, test_pass_thres);
   CHECK_EQUAL(1U, static_env_poly_out[0].age);
   DOUBLES_EQUAL(group_a.min_long_pos, static_env_poly_out[0].lower_limit, test_pass_thres);
   DOUBLES_EQUAL((group_a.min_long_pos + group_a.delta_long_pos * (group_a.nr_objects - 1)), static_env_poly_out[0].upper_limit, test_pass_thres);
   DOUBLES_EQUAL(group_a.lat_pos, static_env_poly_out[0].p0, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[0].p1, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[0].p2, test_pass_thres);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_UPDATED, static_env_poly_out[0].status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_LSC, static_env_poly_out[0].poly_type);

   DOUBLES_EQUAL(1.0F, static_env_poly_out[1].confidence, test_pass_thres);
   CHECK_EQUAL(1U, static_env_poly_out[1].age);
   DOUBLES_EQUAL(group_b.min_long_pos, static_env_poly_out[1].lower_limit, test_pass_thres);
   DOUBLES_EQUAL((group_b.min_long_pos + group_b.delta_long_pos * (group_b.nr_objects - 1)), static_env_poly_out[1].upper_limit, test_pass_thres);
   DOUBLES_EQUAL(group_b.lat_pos, static_env_poly_out[1].p0, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[1].p1, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[1].p2, test_pass_thres);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_UPDATED, static_env_poly_out[1].status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_LSC, static_env_poly_out[1].poly_type);

   DOUBLES_EQUAL(1.0F, static_env_poly_out[2].confidence, test_pass_thres);
   CHECK_EQUAL(1U, static_env_poly_out[2].age);
   DOUBLES_EQUAL(group_e.min_long_pos, static_env_poly_out[2].lower_limit, test_pass_thres);
   DOUBLES_EQUAL((group_e.min_long_pos + group_e.delta_long_pos * (group_e.nr_objects - 1)), static_env_poly_out[2].upper_limit, test_pass_thres);
   DOUBLES_EQUAL(group_e.lat_pos, static_env_poly_out[2].p0, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[2].p1, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[2].p2, test_pass_thres);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_UPDATED, static_env_poly_out[2].status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_LSC, static_env_poly_out[2].poly_type);

   DOUBLES_EQUAL(0.0F, static_env_poly_out[3].confidence, test_pass_thres);
   CHECK_EQUAL(0U, static_env_poly_out[3].age);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[3].lower_limit, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[3].upper_limit, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[3].p0, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[3].p1, test_pass_thres);
   DOUBLES_EQUAL(0.0F, static_env_poly_out[3].p2, test_pass_thres);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, static_env_poly_out[3].status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_INVALID, static_env_poly_out[3].poly_type);

}

/** @}*/


/** \defgroup  Test_Static_Env_Poly_T
 *  @{
 */

 /** \brief
  * Test group that holds test related to public methods of class/structure Test_Static_Env_Poly_T
  */
TEST_GROUP(Test_Static_Env_Poly_T)
{
   float tolerance = 0.00001F;
};

/** \purpose
 * Verify simple calculations
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, Simple_Calculations)
{
   /** \precond
   * Setup p0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(3.0F);

   /** \result
    * Output sholud be 22.5F
    */
   DOUBLES_EQUAL(22.5F, output, tolerance);
}

/** \purpose
 * Verify calculations correctness if input is zero
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, zero_input)
{
   /** \precond
   * Setup p0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(0.0F);

   /** \result
    * Output sholud be 0.3F
    */
   DOUBLES_EQUAL(0.3F, output, tolerance);
}

/** \purpose
 * Verify calculations correctness if input is negative
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, negative_input)
{
   /** \precond
   * Setup p0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(-3.0F);

   /** \result
    * Output sholud be 14.1F
    */
   DOUBLES_EQUAL(14.1F, output, tolerance);
}

/** \purpose
 * Verify calculations correctness if p2 is zero
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, p2_is_zero)
{
   /** \precond
   * Setup p0, p1, p2 = 0
   */
   Static_Env_Poly_T sep;
   sep.p2 = 0.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(3.0F);

   /** \result
    * Output sholud be 4.5F
    */
   DOUBLES_EQUAL(4.5F, output, tolerance);
}

/** \purpose
 * Verify calculations correctness if p1 is zero
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, p1_is_zero)
{
   /** \precond
   * Setup p0, p1 = 0, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 0.0F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(3.0F);

   /** \result
    * Output sholud be 18.3F
    */
   DOUBLES_EQUAL(18.3F, output, tolerance);
}

/** \purpose
 * Verify calculations correctness if p0 is zero
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, p0_is_zero)
{
   /** \precond
   * Setup p0 = 0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.0F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(3.0F);

   /** \result
    * Output sholud be 22.2F
    */
   DOUBLES_EQUAL(22.2F, output, tolerance);
}

/** \purpose
 * Verify calculations correctness if input is nan
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, input_is_nan)
{
   /** \precond
   * Setup p0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(std::numeric_limits<float32_t>::quiet_NaN());

   /** \result
    * Output sholud be nan.
    */
   CHECK_TRUE(std::isnan(output));
}

/** \purpose
 * Verify calculations correctness if input is positive inf
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, input_is_positive_inf)
{
   /** \precond
   * Setup p0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(std::numeric_limits<float32_t>::infinity());

   /** \result
    * Output sholud be positive inf
    */
   DOUBLES_EQUAL(std::numeric_limits<float32_t>::infinity(), output, tolerance);
}

/** \purpose
 * Verify calculations correctness if input is negative inf
 * \req
 * NA
 */
TEST(Test_Static_Env_Poly_T, input_is_negative_inf)
{
   /** \precond
   * Setup p0, p1, p2
   */
   Static_Env_Poly_T sep;
   sep.p2 = 2.0F;
   sep.p1 = 1.4F;
   sep.p0 = 0.3F;

   /** \action
    * Call Lateral_Pos()
    */
   const float output = sep.Lateral_Pos_At(-std::numeric_limits<float32_t>::infinity());

   /** \result
    * Output sholud be nan.
    */
   CHECK_TRUE(std::isnan(output));
}
/** @}*/


/** \defgroup  Run_Concrete_Wall_Detector
 *  @{
 */

 /** \brief
  * Test group that holds tests related to public method Run_Concrete_Wall_Detector() of Static_Env_T class .
  */
TEST_GROUP(Run_Concrete_Wall_Detector)
{
   // Declare common variables used within all tests in this test group.
   Static_Env_T static_env{};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS]{};
   rspp_variant_A::RSPP_Detection_List_T raw_detect_list{};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS]{};
   F360_Radar_Sensor_Props_T sensor_props[MAX_NUMBER_OF_SENSORS]{};
   F360_Tracker_Info_T tracker_info{};
   F360_Calibrations_T calib{};
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS]{};
   F360_Host_T host{};
   F360_TRKR_TIMING_INFO_T timing_info{};

   const float32_t numeric_threshold = 0.000001F;

   void Append_Detection(const uint32_t det_idx, const float32_t long_posn, const float32_t lat_posn)
   {
      if (det_idx > 0U)
      {
         raw_detect_list.detections[det_idx - 1U].processed.next_sorted_idx = det_idx; // first extend the list of sorted detections by one
      }
      det_props[det_idx].vcs_position.y = lat_posn;
      det_props[det_idx].vcs_position.x = long_posn;
      raw_detect_list.detections[det_idx].processed.motion_status =rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
      det_props[det_idx].f_ok_to_use = true;
      raw_detect_list.detections[det_idx].processed.prev_sorted_idx = det_idx - 1U;
      raw_detect_list.detections[det_idx].processed.next_sorted_idx = F360_INVALID_ID;
   }

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }

};

/** \purpose
 * Purpose of this test is to verify that static environment polynomials that represent CWD are updated when
 * Run_Concrete_Wall_Detector() is call.
 * \req
 * NA
 */
TEST(Run_Concrete_Wall_Detector, If_Run_Concrete_Wall_Detector_updates_suitable_structures)
{
   /** \precond
   * Setup polynomials parameters
   */
   // Define arbitrary polynomials
   Static_Env_Poly_T static_env_poly_in[F360_NUM_OF_STATIC_ENV_POLYS]{};
   for (uint8_t i = (F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD); i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      static_env_poly_in[i].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      static_env_poly_in[i].poly_type = F360_STATIC_ENV_POLY_TYPE_INVALID;
   }

   // Set above polynomials in static_env object
   static_env.Set_Polynomials(static_env_poly_in);

   /** \action
    * Call Run_Concrete_Wall_Detector() and get updated polynomials
    */
   static_env.Run_Concrete_Wall_Detector(det_props, raw_detect_list, sensors, sensor_props, calib, objects, host, timing_info);
   const Static_Env_Poly_T(&static_env_poly_out)[F360_NUM_OF_STATIC_ENV_POLYS] = static_env.Get_Polynomials();

   /** \result
    * Compare that data have been modified
    */
   for (uint8_t i = (F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD); i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, static_env_poly_out[i].status);
      CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_CWD, static_env_poly_out[i].poly_type);
   }
}

/** \purpose
 * Purpose of this test is to verify that static environment polynomials that represent CWD are updated when
 * Run_Concrete_Wall_Detector() is call. It also checks if estimation status is INVALID despite lack of proper input
 * data and f_initialized flag set to true.
 * \req
 * NA
 */
TEST(Run_Concrete_Wall_Detector, If_Run_Concrete_Wall_Detector_sets_INVALID_polinomial_status_when_no_input_data_and_f_initialized_equal_to_true)
{
   /** \precond
   * Setup polynomials parameters
   */
   // Define arbitrary polynomials
   Static_Env_Poly_T static_env_poly_in[F360_NUM_OF_STATIC_ENV_POLYS]{};
   for (uint8_t i = (F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD); i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      static_env_poly_in[i].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
      static_env_poly_in[i].poly_type = F360_STATIC_ENV_POLY_TYPE_INVALID;
   }

   // Set above polynomials in static_env object
   static_env.Set_Polynomials(static_env_poly_in);

   // Set CWD f_initialized flag to true
   F360_Concrete_Wall_Detector_T & cwd = const_cast<F360_Concrete_Wall_Detector_T &>(static_env.Get_CWD());
   cwd.Set_f_initialized(true);

   /** \action
    * Call Run_Concrete_Wall_Detector() and get updated polynomials
    */
   static_env.Run_Concrete_Wall_Detector(det_props, raw_detect_list, sensors, sensor_props, calib, objects, host, timing_info);
   const Static_Env_Poly_T(&static_env_poly_out)[F360_NUM_OF_STATIC_ENV_POLYS] = static_env.Get_Polynomials();

   /** \result
    * Compare that data have been modified
    */
   for (uint8_t i = (F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD); i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, static_env_poly_out[i].status);
      CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_CWD, static_env_poly_out[i].poly_type);
   }
}

/** \purpose
 * Purpose of this test is to verify if Run_Concrete_Wall_Detector() returns correct output
 * when correct input provided.
 * \req
 * NA
 */
TEST(Run_Concrete_Wall_Detector, If_Run_Concrete_Wall_Detector_returns_correct_output_when_correct_input_provided)
{
   /** \precond 1
    * Set necessary host and sensors params
    */
   calib.k_cwd_sensor_zone_half_length = 0.5F;
   host.speed = 2.0F;
   host.curvature_rear = 1.0F;

   // Front left sensor
   sensors[0U].variable.is_valid = true;
   sensors[0U].constant.mounting_position.vcs_position.longitudinal = 0.0F;
   sensors[0U].constant.mounting_position.vcs_position.lateral = -1.0F;
   sensors[0U].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_FORWARD;

   // Front right sensor
   sensors[1U].variable.is_valid = true;
   sensors[1U].constant.mounting_position.vcs_position.longitudinal = 0.0F;
   sensors[1U].constant.mounting_position.vcs_position.lateral = 1.0F;
   sensors[1U].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_FORWARD; ;

   // Rear left sensor
   sensors[2U].variable.is_valid = true;
   sensors[2U].constant.mounting_position.vcs_position.longitudinal = -4.0F;
   sensors[2U].constant.mounting_position.vcs_position.lateral = -1.0F;
   sensors[2U].constant.mounting_location = F360_MOUNTING_LOCATION_LEFT_REAR;

   // Rear right sensor
   sensors[3U].variable.is_valid = true;
   sensors[3U].constant.mounting_position.vcs_position.longitudinal = -4.0F;
   sensors[3U].constant.mounting_position.vcs_position.lateral = 1.0F;
   sensors[3U].constant.mounting_location = F360_MOUNTING_LOCATION_RIGHT_REAR;

   /** \precond 2
    * Set necessary detections and tracker info
    */
    // Front left sensor detections
   Append_Detection(0U, 0.0F, -2.0F);

   // Front right sensor detections
   Append_Detection(1U, 0.0F, 2.0F);

   // Rear left sensor detections
   Append_Detection(2U, -4.0F, -2.0F);

   // Rear right sensor detections
   Append_Detection(3U, -4.0F, 2.0F);

   raw_detect_list.detections[0U].processed.next_sorted_idx = -1;
   raw_detect_list.detections[1U].processed.next_sorted_idx = 0;
   raw_detect_list.detections[2U].processed.next_sorted_idx = 1;
   raw_detect_list.detections[3U].processed.next_sorted_idx = 2;

   raw_detect_list.vcslong_sorted_ref_det_idx[0] = 3;
   raw_detect_list.vcslong_sorted_ref_det_idx[1] = 2;
   raw_detect_list.vcslong_sorted_ref_det_idx[2] = 1;
   raw_detect_list.vcslong_sorted_ref_det_idx[3] = 0;

   /** \precond 3
    * Set expected data
    */
   const uint8_t idx_cwd_left = F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD;
   const uint8_t idx_cwd_right = F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD + 1;

   Static_Env_Poly_T static_env_poly_expected[F360_NUM_OF_STATIC_ENV_POLYS];
   // Expected data for CWD left
   static_env_poly_expected[idx_cwd_left].age = 1U;
   static_env_poly_expected[idx_cwd_left].confidence = 0.2F;
   static_env_poly_expected[idx_cwd_left].lower_limit = -6.0F;
   static_env_poly_expected[idx_cwd_left].upper_limit = 2.0F;
   static_env_poly_expected[idx_cwd_left].p0 = -2.0F;
   static_env_poly_expected[idx_cwd_left].p1 = 0.0F;
   static_env_poly_expected[idx_cwd_left].p2 = 0.5F;
   static_env_poly_expected[idx_cwd_left].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   static_env_poly_expected[idx_cwd_left].poly_type = F360_STATIC_ENV_POLY_TYPE_CWD;

   // Expected data for CWD right
   static_env_poly_expected[idx_cwd_right].age = 1U;
   static_env_poly_expected[idx_cwd_right].confidence = 0.2F;
   static_env_poly_expected[idx_cwd_right].lower_limit = -6.0F;
   static_env_poly_expected[idx_cwd_right].upper_limit = 2.0F;
   static_env_poly_expected[idx_cwd_right].p0 = 2.0F;
   static_env_poly_expected[idx_cwd_right].p1 = 0.0F;
   static_env_poly_expected[idx_cwd_right].p2 = 0.5F;
   static_env_poly_expected[idx_cwd_right].status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   static_env_poly_expected[idx_cwd_right].poly_type = F360_STATIC_ENV_POLY_TYPE_CWD;

   /** \action
    * Call Run_Concrete_Wall_Detector() and get updated polynomials
    */
   static_env.Run_Concrete_Wall_Detector(det_props, raw_detect_list, sensors, sensor_props, calib, objects, host, timing_info);
   static_env.Run_Concrete_Wall_Detector(det_props, raw_detect_list, sensors, sensor_props, calib, objects, host, timing_info);

   const Static_Env_Poly_T(&static_env_poly_out)[F360_NUM_OF_STATIC_ENV_POLYS] = static_env.Get_Polynomials();

   /** \result
    * Check that CWD output data are equal to expected
    */
   for (uint8_t i = (F360_NUM_OF_STATIC_ENV_POLYS - MAX_NR_OF_CWD); i < F360_NUM_OF_STATIC_ENV_POLYS; i++)
   {
      CHECK_EQUAL(static_env_poly_expected[i].age, static_env_poly_out[i].age);
      DOUBLES_EQUAL(static_env_poly_expected[i].confidence, static_env_poly_out[i].confidence, numeric_threshold);
      DOUBLES_EQUAL(static_env_poly_expected[i].lower_limit, static_env_poly_out[i].lower_limit, numeric_threshold);
      DOUBLES_EQUAL(static_env_poly_expected[i].upper_limit, static_env_poly_out[i].upper_limit, numeric_threshold);
      DOUBLES_EQUAL(static_env_poly_expected[i].p0, static_env_poly_out[i].p0, numeric_threshold);
      DOUBLES_EQUAL(static_env_poly_expected[i].p1, static_env_poly_out[i].p1, numeric_threshold);
      DOUBLES_EQUAL(static_env_poly_expected[i].p2, static_env_poly_out[i].p2, numeric_threshold);
      CHECK_EQUAL(static_env_poly_expected[i].status, static_env_poly_out[i].status);
      CHECK_EQUAL(static_env_poly_expected[i].poly_type, static_env_poly_out[i].poly_type);
   }
}
/** @}*/