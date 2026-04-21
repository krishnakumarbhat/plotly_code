/** \file
  File f360_calc_probabilities_of_detections_unittest.cpp
  contains unit tests for f360_calc_probabilities_of_detections
  in sensor preprocessing module.
 */

#include "f360_calc_probabilities_of_detections.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cstring>
#include "f360_math.h"

 //Declaration of stubbed/mock functions

 //Implementation of stubbed interfaces

 /** \defgroup  f360_calc_probabilities_of_detections
  *  @{
  */
using namespace f360_variant_A;
/** \brief
 *  Test group contains tests for functions
 *  in f360_calc_probabilities_of_detections file.
 **/
TEST_GROUP(f360_calc_probabilities_of_detections)
{
   /** \setup
    * Nothing to setup in this test group
    **/
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_dets = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Calibrations_T calibrations{};

   float32_t ant_sens_SCS_azim_deg[NUM_VTX_ANT_SENS_POLY] = { -75.0F, -70.0F, -65.0F, -55.0F, -45.0F, -35.0F, -25.0F, -15.0F,  -5.0F,   5.0F,  15.0F,  25.0F,  35.0F,  45.0F,  55.0F,  65.0F,  70.0F,  75.0F };
   float32_t ant_sens_SCS_rng_90[NUM_VTX_ANT_SENS_POLY] = { 31.93F, 35.46F, 36.12F, 36.20F, 36.71F, 38.08F, 39.74F, 41.95F, 44.80F, 46.75F, 48.50F, 50.32F, 52.22F, 53.85F, 54.30F, 52.52F, 43.49F, 33.96F };

   float32_t UT_float_threshold = 0.0001F;

   TEST_SETUP()
   {
      sensors[0].variable.f_ant_sens_available = true;
      sensors[0].variable.f_ant_sens_degraded = false;
      sensors[0].constant.polarity = 1;

      for (int idx = 0; idx < F360_NUM_LOOK_ID; idx++)
      {
         sensors[0].constant.range_limits[idx] = 100.0F;
         sensors[0].constant.fov_min_az_rad[idx] = F360_DEG2RAD(-76.0F);
         sensors[0].constant.fov_max_az_rad[idx] = F360_DEG2RAD(76.0F);
      }       

      for (int az_idx = 0; az_idx < NUM_VTX_ANT_SENS_POLY; az_idx++)
      {
         sensors[0].constant.ant_sens_SCS_azim[az_idx] = F360_DEG2RAD(ant_sens_SCS_azim_deg[az_idx]);
         sensors[0].constant.ant_sens_SCS_sq_rng_90[az_idx] = ant_sens_SCS_rng_90[az_idx] * ant_sens_SCS_rng_90[az_idx];
      }

      sensors[0].variable.look_id = F360_DET_LOOK_ID_0;

      raw_dets.number_of_valid_detections = 1;
      raw_dets.detections[0].raw.range = F360_Sqrtf(sensors[0].constant.ant_sens_SCS_sq_rng_90[4]) - UT_float_threshold;
      raw_dets.detections[0].raw.azimuth = sensors[0].constant.ant_sens_SCS_azim[4] + UT_float_threshold;
      raw_dets.detections[0].raw.sensor_id = 1;

      Initialize_Tracker_Calibrations(calibrations);
   }

   /** \teardown
    * Very important to clear the mock, to avoid memory leaks
    **/
   TEST_TEARDOWN()
   {
      mock().clear();
   }

};

/**
 *\purpose  Correct calculation of high existence probability
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__high_prob)
{
   /** \precond
   **/

   /** \action
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.9F, UT_float_threshold);
}

/**
 *\purpose  Correct calculation of medium existence probability
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__medium_prob)
{
   /** \precond
   **/
   raw_dets.detections[0].raw.range = F360_Sqrtf(sensors[0].constant.ant_sens_SCS_sq_rng_90[4]) + UT_float_threshold;

   /** \action
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.7F, UT_float_threshold);
}

/**
 *\purpose  Correct calculation of low existence probability
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__low_prob)
{
   /** \precond
   **/
   raw_dets.detections[0].raw.range = F360_Sqrtf(sensors[0].constant.ant_sens_SCS_sq_rng_90[5]) + UT_float_threshold;

   /** \action
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.5F, UT_float_threshold);
}


/**
 *\purpose  Correct calculation of init existence probability when det's azimuth is higher than max azimuth from sensitivity map
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__init_prob_when_det_higher_than_sensor_max_azimuth_from_sensitivity_map)
{
   /** \precond 
   **/
   raw_dets.detections[0].raw.azimuth = sensors[0].constant.ant_sens_SCS_azim[NUM_VTX_ANT_SENS_POLY - 1];
   raw_dets.detections[0].raw.range = F360_Sqrtf(sensors[0].constant.ant_sens_SCS_sq_rng_90[NUM_VTX_ANT_SENS_POLY - 1]) + UT_float_threshold;

   /** \action 
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.1F, UT_float_threshold);
}

/**
 *\purpose  Correct calculation of  init existence probability when det's azimuth is lower than min azimuth from sensitivity map
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__init_prob_when_det_lower_than_sensor_min_azimuth_from_sensitivity_map)
{
   /** \precond 
   **/
   raw_dets.detections[0].raw.azimuth = sensors[0].constant.ant_sens_SCS_azim[0] - UT_float_threshold;
   raw_dets.detections[0].raw.range = F360_Sqrtf(sensors[0].constant.ant_sens_SCS_sq_rng_90[0]) - UT_float_threshold;

   /** \action 
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result 
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.1F, UT_float_threshold);
}

/**
 *\purpose  Correct calculation of init existence probability when det's range is higher than sensor max range
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__init_prob_when_det_higher_than_sensor_max_range)
{
   /** \precond 
   **/
   raw_dets.detections[0].raw.range = sensors[0].constant.range_limits[0] + UT_float_threshold;

   /** \action 
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result 
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.1F, UT_float_threshold);
}

/**
 *\purpose  Correct calculation of init existence probability when sensitivity map is not available and det's range is high 
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__init_prob_when_sensitivity_map_is_not_available_and_high_range)
{
   /** \precond 
   **/
   raw_dets.detections[0].raw.range = sensors[0].constant.range_limits[0] + UT_float_threshold;
   sensors[0].variable.f_ant_sens_available = false;

   /** \action 
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result 
   **/
   DOUBLES_EQUAL(det_props[0].probability_of_detection, 0.1F, UT_float_threshold);
}

/**
 *\purpose  Correct calculation of default existence probability when sensitivity map is not available
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections__default_prob_when_sensitivity_map_is_not_available)
{
   /** \precond
   **/
   sensors[0].variable.f_ant_sens_available = false;

   /** \action 
   **/
   Compute_Detection_Probability(raw_dets.detections[0], sensors[0], calibrations, det_props[0]);

   /** \result 
   **/
   DOUBLES_EQUAL(det_props[0] .probability_of_detection, 0.7F, UT_float_threshold);
}

/**
 *\purpose  Test the behaviour of Calculate_Probabilities_Of_Detections function
 *\req    NA
 */
TEST(f360_calc_probabilities_of_detections, Calculate_Probabilities_Of_Detections)
{
   /** \step{1}
    *Check if calculated probabilities are in range and the results are as expected.
    *Check the probabilities at boundary conditions of input.
    **/

    /** \precond
     * Set up the raw detection list, sensor calibration,
     * sensor list and the expected detection properties.
     **/
   rspp_variant_A::RSPP_Detection_List_T p_raw_detection_list;
   p_raw_detection_list.number_of_valid_detections = 11;
   memset(p_raw_detection_list.detections, 0, sizeof(p_raw_detection_list.detections));
   rspp_variant_A::RSPP_Detection_List_T* raw_detection_list = &p_raw_detection_list;

   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS];
   memset(sensors, 0, sizeof(sensors));

   F360_Detection_Props_T p_det_props[MAX_NUMBER_OF_DETECTIONS];
   memset(p_det_props, 0, sizeof(p_det_props));
   F360_Detection_Props_T* det_props = p_det_props;

   int look_id = 0;

   float expected_probability[MAX_NUMBER_OF_DETECTIONS] = {};


   // Generating the sensor calib data
   for (int i = 0; i < 7; i++)
   {
      sensors[i].variable.look_id = F360_DET_LOOK_ID_0;
      look_id = F360_DET_LOOK_ID_0;
      sensors[i].constant.polarity = 1;
      sensors[i].constant.range_limits[look_id] = 90.0F;
      sensors[i].constant.fov_min_az_rad[look_id] = -1.3F;
      sensors[i].constant.fov_max_az_rad[look_id] = 1.3F;
      sensors[i].constant.fov_min_el_rad[look_id] = -0.08F;
      sensors[i].constant.fov_max_el_rad[look_id] = 0.08F;
      sensors[i].constant.min_aliaised_range_rate[look_id] = -45.0F;
      sensors[i].constant.v_wrapping[look_id] = 70.0F;

      // Antenna sensitivity map
      sensors[i].variable.f_ant_sens_degraded = false;

      // Sensor calibs without antenna sensitivity map available
      if (i == 0)
      {
         sensors[i].variable.f_ant_sens_available = false;
      }

      // Sensor calib data for high probability
      else if (i == 1)
      {
         sensors[i].variable.f_ant_sens_available = true;
         sensors[i].constant.ant_sens_SCS_azim[0] = 0.0F;
         sensors[i].constant.ant_sens_SCS_azim[1] = 2.0F;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[1] = 150;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[0] = 150;
      }

      // Sensor calib data for medium probability
      else if (i == 2)
      {
         sensors[i].variable.f_ant_sens_available = true;
         sensors[i].constant.ant_sens_SCS_azim[0] = 0.0F;
         sensors[i].constant.ant_sens_SCS_azim[1] = 2.0F;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[1] = 150;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[0] = 50;
      }

      // Sensor calib data for low probability
      else if (i == 3)
      {
         sensors[i].variable.f_ant_sens_available = true;
         sensors[i].constant.ant_sens_SCS_azim[0] = 0.0F;
         sensors[i].constant.ant_sens_SCS_azim[1] = 2.0F;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[1] = 50;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[0] = 50;
      }

      else if (i == 4)
      {
         sensors[i].variable.f_ant_sens_available = true;
         // Production code expectes return value to be > 0 as algo maps the two closest array indexes in ant_sens_SCS_azim[]
         // A return of 0 leads to the condition "azimuth beyond sensitivity map azimuth range" and det prob becomes 0.1 
         sensors[i].constant.ant_sens_SCS_azim[0] = 0.0F;
         sensors[i].constant.ant_sens_SCS_azim[1] = 0.0F;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[1] = 50;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[0] = 50;
      }

      // Sensor calibs for azimuth beyond max range
      else if (i == 5)
      {
         sensors[i].variable.f_ant_sens_available = true;
         sensors[i].constant.range_limits[look_id] = 9;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[1] = 50;
         sensors[i].constant.ant_sens_SCS_sq_rng_90[0] = 50;
      }

      // Sensor calib data for degraded antennea sensitivity
      else if (i == 6)
      {
         sensors[i].variable.f_ant_sens_available = false;
         sensors[i].variable.f_ant_sens_degraded = true;
      }
      else
      {
         //Do Nothing

      }
   }

   // Generating the detection data
   for (unsigned int i = 0; i < raw_detection_list->number_of_valid_detections; i++)
   {
      raw_detection_list->detections[i].raw.det_id = i;
      raw_detection_list->detections[i].raw.range = 10;
      raw_detection_list->detections[i].raw.range_rate = 8;
      raw_detection_list->detections[i].raw.elevation = 0.01;
      raw_detection_list->detections[i].raw.elevation = 0.02;
      raw_detection_list->detections[i].raw.confid_azimuth = 1;
      raw_detection_list->detections[i].raw.confid_elevation = 2;
      raw_detection_list->detections[i].raw.rcs = -10.1f;
      raw_detection_list->detections[i].raw.f_super_res = false;
      raw_detection_list->detections[i].raw.f_host_veh_clutter = false;
      raw_detection_list->detections[i].raw.f_nd_target = false;
      det_props[i].f_double_bounce = false;

      // Data within the field of view, sensitivity map not available
      if (i == 0)
      {
         raw_detection_list->detections[i].raw.sensor_id = 1;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.7;
      }

      // Data at the boundaries of filed of view, for low probability, sensitivity map not available
      else if (i == 1)
      {
         raw_detection_list->detections[i].raw.sensor_id = 1;
         raw_detection_list->detections[i].raw.azimuth = 1.3;
         raw_detection_list->detections[i].raw.azimuth = 1.3;
         expected_probability[i] = 0.1;
      }

      // Data outside the field of view that calculates low probability. sensitivity map not available
      else if (i == 2)
      {
         raw_detection_list->detections[i].raw.sensor_id = 1;
         raw_detection_list->detections[i].raw.azimuth = 1.5;
         raw_detection_list->detections[i].raw.azimuth = 1.5;
         expected_probability[i] = 0.1;
      }

      // Valid azim interval, and high probability of detection and sensitivity map available
      else if (i == 3)
      {
         raw_detection_list->detections[i].raw.sensor_id = 2;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.9;
      }
      //valid azim interval, for sensor_id =4 -1, for medium proababilty of detection when antennea sensitivity map available
      else if (i == 4)
      {
         raw_detection_list->detections[i].raw.sensor_id = 3;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.7;
      }
      //valid azim interval, for sensor_id =5 -1, for low proababilty and init of detection when antennea sensitivity map available
      else if (i == 5)
      {
         raw_detection_list->detections[i].raw.sensor_id = 4;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.5;
      }
      // sensor_id 5 corresponds to sensor calibs of antennea sensitivity data for low probability of detection
      else if (i == 6)
      {
         raw_detection_list->detections[i].raw.sensor_id = 5;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.1;
      }
      // sensor_id corresponding to azimuth beyond sensor max range
      else if (i == 7)
      {
         raw_detection_list->detections[i].raw.sensor_id = 6;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.1;
      }
      // For sensor_id = 7 , f_ant_sens_degraded has been set true, therefore we should expect degragded probability, which is for example
      // probability_of_detection_default*degradation_factor (0.7*0.5 = 0.35)
      else if (i == 8)
      {
         raw_detection_list->detections[i].raw.sensor_id = 7;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.35;
      }
      // Low probability if f_double_bounce = true
      else if (i == 9)
      {
         raw_detection_list->detections[i].raw.sensor_id = 7;
         det_props[i].f_double_bounce = true;
         raw_detection_list->detections[i].raw.azimuth = 1;
         raw_detection_list->detections[i].raw.azimuth = 0.97;
         expected_probability[i] = 0.05;
      }
      else
      {
         // Do Nothing
      }
   }

   /** \action
    * Call Calculate_Probabilities_Of_Detections() to calculate the probabilities.
    **/
   for (uint32_t det_idx = 0U; det_idx < raw_detection_list->number_of_valid_detections; det_idx++)
   {
      const rspp_variant_A::RSPP_Detection_T &current_detection = raw_detection_list->detections[det_idx];
      F360_Detection_Props_T &current_detection_prop = det_props[det_idx];
      const int32_t current_sensor_id = current_detection.raw.sensor_id;
      const F360_Radar_Sensor_T &current_sensor = sensors[current_sensor_id - 1];
      Compute_Detection_Probability(current_detection, current_sensor, calibrations, current_detection_prop);
   }

   /** \result
    *Check calculated probabilities equal expected probabilities
    **/
   CHECK_EQUAL(expected_probability[0], det_props[0].probability_of_detection);
   CHECK_EQUAL(expected_probability[1], det_props[1].probability_of_detection);
   CHECK_EQUAL(expected_probability[2], det_props[2].probability_of_detection);
   CHECK_EQUAL(expected_probability[3], det_props[3].probability_of_detection);
   CHECK_EQUAL(expected_probability[4], det_props[4].probability_of_detection);
   CHECK_EQUAL(expected_probability[5], det_props[5].probability_of_detection);
   CHECK_EQUAL(expected_probability[6], det_props[6].probability_of_detection);
   CHECK_EQUAL(expected_probability[7], det_props[7].probability_of_detection);
   CHECK_EQUAL(expected_probability[8], det_props[8].probability_of_detection);
   CHECK_EQUAL(expected_probability[9], det_props[9].probability_of_detection);
}

/** @}*/
