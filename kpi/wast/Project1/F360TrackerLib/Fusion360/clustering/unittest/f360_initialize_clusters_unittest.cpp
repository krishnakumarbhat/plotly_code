/** \file
   File with set of unit tests for initialize_clusters function
*/

#include "f360_initialize_clusters.h"
#include "f360_clustering_data_generator.h"
#include "f360_math.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

using namespace f360_variant_A;
/** \defgroup  initialize_clusters
 *  @{
 */


/** \brief
*  Test group for initialize_clusters function
**/
TEST_GROUP(initialize_clusters)
{
   /** \setup
   * Setting up arguments for initialize_clusters function and assigning them with basic values
   **/
   F360_Tracker_Info_T tracker_info = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Local_Clusters_T local_cluster_data = {};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS] = {};
   F360_Calibrations_T calibs;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      Set_Tracker_Variant(tracker_info.variant);

      Setup_TrackerInfo_Cluster_Detp(tracker_info, clusters, raw_detection_list);
   }
};

/**
*\purpose  Test to cover req if cluster ids are properly set for 10 detetions with F360_MSMT_UPD_TIMESTAMPING_MEAN_DET timestamp update scheme
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Mean_Det_Timestamping)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0;
   float32_t det_vcs_pos_lat = 0.0;
   float32_t det_range_rate = 10.0;
   float32_t det_probability_of_detection = 1.0;
   bool det_f_dealiased = true;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   f_take_new_cluster_to_init = false;
   for (int16_t i = 0; i < 10; i++)
   {
      det_vcs_pos_long += 0.01;
      det_vcs_pos_lat += 0.01;
      det_range_rate += 0.01;
      Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);
   }

   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if detections have correct set cluster ids
   **/
   for (int16_t i = 0; i < 11; i++)
   {
      CHECK_EQUAL(1, detection_props[i].cluster_id);
   }
}

/**
*\purpose  Test to cover req if cluster ids are properly set for 10 detetions with F360_MSMT_UPD_TIMESTAMPING_MEAN_DET timestamp update scheme
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Mean_Det_Timestamping2)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0;
   float32_t det_vcs_pos_lat = 0.0;
   float32_t det_range_rate = 10.0;
   float32_t det_probability_of_detection = 1.0;
   bool det_f_dealiased = true;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   f_take_new_cluster_to_init = false;
   for (int16_t i = 0; i < 10; i++)
   {
      det_vcs_pos_long += 0.01;
      det_vcs_pos_lat += 0.01;
      det_range_rate += 0.01;
      Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);
   }

   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if detections have correct set cluster ids
   **/
   for (int16_t i = 0; i < 11; i++)
   {
      CHECK_EQUAL(1, detection_props[i].cluster_id);
   }
}

/**
*\purpose  Test to check cluster initialization for multiple detections with different dealiased flag
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Many_Detection_To_Cluster_Diff_Dealiased_Flag)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0;
   float32_t det_vcs_pos_lat = 0.0;
   float32_t det_range_rate = 10.0;
   float32_t det_probability_of_detection = 1.0;
   bool det_f_dealiased = false;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   f_take_new_cluster_to_init = false;
   for (int16_t i = 0; i < 10; i++)
   {
      if (i % 2 == 0)
      {
         det_f_dealiased = true;
      }
      else
      {
         det_f_dealiased = false;
      }
      det_vcs_pos_long += 0.01;
      det_vcs_pos_lat += 0.01;
      det_range_rate += 0.01;
      Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);
   }

   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if detections have correct set cluster ids
   **/
   for (int16_t i = 0; i < 11; i++)
   {
      CHECK_EQUAL(1, detection_props[i].cluster_id);
   }
}

/**
*\purpose  Test to check if no cluster would initialized if there is clusters saturation
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Too_Many_Clusters)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0;
   float32_t det_vcs_pos_lat = 0.0;
   float32_t det_range_rate = 10.0;
   float32_t det_probability_of_detection = 1.0;
   bool det_f_dealiased = true;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   f_take_new_cluster_to_init = false;
   for (int16_t i = 0; i < 10; i++)
   {
      det_vcs_pos_long += 0.01;
      det_vcs_pos_lat += 0.01;
      det_range_rate += 0.01;
      Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
         det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);
   }

   tracker_info.num_active_clusters = NUMBER_OF_CLUSTERS + 1;

   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if detections have correct set cluster ids
   **/
   for (int16_t i = 0; i < 11; i++)
   {
      CHECK_EQUAL(0, detection_props[i].cluster_id);
   }
}

/**
*\purpose  Test to line cover zero division protection for Preinitialize_Cluster function
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Preinitialize_Cluster_Zero_Division_Coverage)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   uint16_t first_det_idx_in_cluster = 0U;
   uint16_t array_of_det_idxs_in_clusters[MAX_NUMBER_OF_DETECTIONS] = {};
   uint16_t number_of_dets_in_cluster = 0U;
   uint16_t max_dets_in_obj_track = tracker_info.variant.num_dets_in_track;

   /** \action
   * Calling Initialize_Clusters
   **/

   Preinitialize_Cluster(first_det_idx_in_cluster, array_of_det_idxs_in_clusters, raw_detection_list, calibs,
      max_dets_in_obj_track, detection_props, number_of_dets_in_cluster, clusters[0]);

   /** \result
   * Checking if there are no detections in cluster
   **/
   CHECK_EQUAL(0, clusters[0].ndets);
}

/**
*\purpose Test if the confidence blocker counter is set for a newly initialized cluster with a single, low rcs detection
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Conf_Blocker_1)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0F;
   float32_t det_vcs_pos_lat = 0.0F;
   float32_t det_range_rate = 1.5F;
   float32_t det_probability_of_detection = 1.0F;
   bool det_f_dealiased = true;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   raw_detection_list.detections[0].raw.rcs = -20.0F;
   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if counter is set to expected value
   **/
   CHECK_EQUAL(1, clusters[0].low_rcs_dets_cnt);
}

/**
*\purpose Test if the confidence blocker counter is set for a newly initialized cluster with a single, very low rcs detection
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Conf_Blocker_2)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0F;
   float32_t det_vcs_pos_lat = 0.0F;
   float32_t det_range_rate = 1.5F;
   float32_t det_probability_of_detection = 1.0F;
   bool det_f_dealiased = true;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   raw_detection_list.detections[0].raw.rcs = -30.0F;
   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if counter is set to expected value
   **/
   CHECK_EQUAL(2, clusters[0].low_rcs_dets_cnt);
}

/**
*\purpose Test if the confidence blocker counter is unset for a newly initialized cluster with a single, high rcs detection
*\req    NA
*/
TEST(initialize_clusters, Test_Initialize_Clusters_Conf_Blocker_3)
{
   /** \precond
   Preparing initial clusters data and detections
   **/
   bool f_take_new_cluster_to_init = true;
   float32_t det_vcs_pos_long = 1.0F;
   float32_t det_vcs_pos_lat = 0.0F;
   float32_t det_range_rate = 1.5F;
   float32_t det_probability_of_detection = 1.0F;
   bool det_f_dealiased = true;
   rspp_variant_A::RSPP_Detection_Motion_Status_T det_motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;

   Simple_Add_Det_for_Initialize_Clusters(f_take_new_cluster_to_init, det_vcs_pos_long, det_vcs_pos_lat, det_range_rate, det_probability_of_detection,
      det_f_dealiased, det_motion_status, raw_detection_list, local_cluster_data, detection_props);

   raw_detection_list.detections[0].raw.rcs = 10.0F;
   /** \action
   * Calling Initialize_Clusters
   **/
   Initialize_Clusters(raw_detection_list, calibs, sensors, local_cluster_data, tracker_info, detection_props, clusters);

   /** \result
   * Checking if counter is set to expected value
   **/
   CHECK_EQUAL(0, clusters[0].low_rcs_dets_cnt);
}


 /** \brief
  * Test group for Calculate_Mean_Range_Rate_Comp_Variance testing.
  */
TEST_GROUP(Compute_Detection_Position_Covariance_Matrix_group)
{
   F360_Calibrations_T calibrations;
   const float32_t tolerance = 1e-5F;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);
   }
};

/**
 *\purpose Test Calc_Pos_Cov Function and check position cov nees
 *\req NA
 */
TEST(Compute_Detection_Position_Covariance_Matrix_group, Test_Calc_Pos_Cov)
{
   /** \precond
    * Setting up input to function and expected output for Calc_Pos_Cov Function.
    **/
   F360_Cluster_T cluster_to_init;
   cluster_to_init.ndets = 1;
   cluster_to_init.detids[0] = 1;

   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list;
   const float32_t sensor_boresight = 0.5F;
   raw_detection_list.detections[0].raw.azimuth = -F360_PI / 4.0F;
   raw_detection_list.detections[0].processed.sin_vcs_az = sinf(raw_detection_list.detections[0].raw.azimuth + sensor_boresight);
   raw_detection_list.detections[0].processed.cos_vcs_az = cosf(raw_detection_list.detections[0].raw.azimuth + sensor_boresight);
   raw_detection_list.detections[0].raw.range = 11.7799997;
   raw_detection_list.detections[0].raw.range_rate = -2.640625;

   /** \action
    * Call Update_Detection_Property to call Calc_Pos_Cov
    **/

   Compute_Detection_Position_Covariance_Matrix(cluster_to_init, calibrations, raw_detection_list, det_props);
   /** \result
    * Check the expected value for position_cov_nees
    **/
   DOUBLES_EQUAL(0.0210181, det_props[0].position_cov_nees[0][0], tolerance)
   DOUBLES_EQUAL(-0.00505063, det_props[0].position_cov_nees[0][1], tolerance)
   DOUBLES_EQUAL(-0.00505063, det_props[0].position_cov_nees[1][0], tolerance)
   DOUBLES_EQUAL(0.00528631, det_props[0].position_cov_nees[1][1], tolerance)
}
/** @}*/
