/** \file
   File with unit tests for Sort_Clusters_By_Priority function
*/

#include "f360_sort_clusters_by_priority.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"
#include "f360_initialization_data_generator.h"

/** \defgroup  f360_sort_clusters_by_priority
 *  @{
 */

using namespace f360_variant_A;
/** \brief
 *  Test group for testing Sort_Clusters_By_Priority function, 
 *  for this tests purpose, priority sorted clusters will be sorted only by distance 
 *  (closer cluster shall have higher priority)
 */
TEST_GROUP(f360_sort_clusters_by_priority)
{
   /** \setup
   * Setup arguments for Sort_Clusters_By_Priority
   */
   F360_Calibrations_T calibrations;
   F360_Detection_Hist_T det_hist;
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Host_T host;
   F360_Tracker_Info_T tracker_info;
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS];
   int32_t sorted_clusters_id_by_prior[NUMBER_OF_CLUSTERS];
   uint32_t num_predond_valid_clusters;
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};

   TEST_SETUP()
   {
      calibrations = {};
      det_hist = {};
      host = {};
      tracker_info = {};
      num_predond_valid_clusters = 0;
      Set_Tracker_Variant(tracker_info.variant);

      calibrations.k_priority_distance_coefficient = 1.0F;
      calibrations.k_priority_distance_for_min_priority_inverse = 1.0F / 200.0F;

      calibrations.k_init_trk_preconditions_min_current_dets = 1;
      calibrations.k_init_trk_preconditions_min_mov_dets_sr = 5;
      calibrations.k_init_trk_preconditions_min_mov_amb_dets = 6;
      calibrations.k_init_trk_preconditions_min_amb_dets = 4;

      for (uint32_t cluster_idx = 0; cluster_idx < NUMBER_OF_CLUSTERS; cluster_idx++)
      {
         clusters[cluster_idx] = {};
         sorted_clusters_id_by_prior[cluster_idx] = 0;
      }

      for (uint32_t det_idx = 0; det_idx < MAX_NUMBER_OF_DETECTIONS; det_idx++)
      {
         det_props[det_idx] = {};
      }
      
      Define_Test_Vector_4_Dealiased_and_2_NotDealiased_Clusters(clusters);
   }
};

/**
*\purpose  Check if function sorts clusters properly
*\req     NA
*/
TEST(f360_sort_clusters_by_priority, test_all_clust_shall_be_sorted_without_saturation)
{
   /** \precond
    * Setting proper clusters as active
    */
   tracker_info.num_active_clusters = 3;
   tracker_info.active_cluster_ids[0] = 2;
   tracker_info.active_cluster_ids[1] = 4;
   tracker_info.active_cluster_ids[2] = 7;

   /** \action
    * Calling Sort_Clusters_By_Priority
    */
   Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detection_list, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_EQUAL(3, num_predond_valid_clusters);
   CHECK_EQUAL(2, sorted_clusters_id_by_prior[0]);
   CHECK_EQUAL(4, sorted_clusters_id_by_prior[1]);
   CHECK_EQUAL(7, sorted_clusters_id_by_prior[2]);
}

/**
*\purpose  Check if function sorts clusters properly
*\req     NA
*/
TEST(f360_sort_clusters_by_priority, test_one_clust_shall_not_be_sorted_without_saturation)
{
   /** \precond
    * Setting proper clusters as active
    */
   tracker_info.num_active_clusters = 4;
   tracker_info.active_cluster_ids[0] = 2;
   tracker_info.active_cluster_ids[1] = 4;
   tracker_info.active_cluster_ids[2] = 6; // invalid cluster
   tracker_info.active_cluster_ids[3] = 7;

   /** \action
    * Calling Sort_Clusters_By_Priority
    */
   Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detection_list, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_EQUAL(3, num_predond_valid_clusters);
   CHECK_EQUAL(2, sorted_clusters_id_by_prior[0]);
   CHECK_EQUAL(4, sorted_clusters_id_by_prior[1]);
   CHECK_EQUAL(7, sorted_clusters_id_by_prior[2]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[3]); // no cluster
}

/**
*\purpose  Check if function sorts clusters properly
*\req     NA
*/
TEST(f360_sort_clusters_by_priority, test_all_clust_shall_be_sorted_with_saturation)
{
   /** \precond
    * Setting proper clusters as active
    */
   tracker_info.num_active_clusters = 4;
   tracker_info.active_cluster_ids[0] = 2;
   tracker_info.active_cluster_ids[1] = 4;
   tracker_info.active_cluster_ids[2] = 7;
   tracker_info.active_cluster_ids[3] = 16;

   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS; // setting saturation

   /** \action
    * Calling Sort_Clusters_By_Priority
    */
   Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detection_list, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_EQUAL(4, num_predond_valid_clusters);
   CHECK_EQUAL(4, sorted_clusters_id_by_prior[0]);
   CHECK_EQUAL(2, sorted_clusters_id_by_prior[1]);
   CHECK_EQUAL(16, sorted_clusters_id_by_prior[2]);
   CHECK_EQUAL(7, sorted_clusters_id_by_prior[3]);
}

/**
*\purpose  Check if function sorts clusters properly
*\req     NA
*/
TEST(f360_sort_clusters_by_priority, test_two_clust_shall_not_be_sorted_with_saturation)
{
   /** \precond
    * Setting proper clusters as active
    */
   tracker_info.num_active_clusters = 6;
   tracker_info.active_cluster_ids[0] = 2;
   tracker_info.active_cluster_ids[1] = 4;
   tracker_info.active_cluster_ids[2] = 6; // invalid cluster
   tracker_info.active_cluster_ids[3] = 7;
   tracker_info.active_cluster_ids[4] = 10; // invalid cluster
   tracker_info.active_cluster_ids[5] = 16;

   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS; // setting saturation

   /** \action
    * Calling Sort_Clusters_By_Priority
    */
   Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detection_list, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_EQUAL(4, num_predond_valid_clusters);
   CHECK_EQUAL(4, sorted_clusters_id_by_prior[0]);
   CHECK_EQUAL(2, sorted_clusters_id_by_prior[1]);
   CHECK_EQUAL(16, sorted_clusters_id_by_prior[2]);
   CHECK_EQUAL(7, sorted_clusters_id_by_prior[3]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[4]); // no cluster
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[5]); // no cluster
}

/**
*\purpose  Check if function sorts clusters properly
*\req     NA
*/
TEST(f360_sort_clusters_by_priority, test_two_invalid_clusters_and_no_valid_cluster_at_all_without_saturation)
{
   /** \precond
    * Setting proper clusters as active
    */
   tracker_info.num_active_clusters = 2;
   tracker_info.active_cluster_ids[0] = 6; // invalid cluster
   tracker_info.active_cluster_ids[1] = 10; // invalid cluster

   /** \action
    * Calling Sort_Clusters_By_Priority
    */
   Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detection_list, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_EQUAL(0, num_predond_valid_clusters);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[0]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[1]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[2]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[3]);
}

/**
*\purpose  Check if function sorts clusters properly
*\req     NA
*/
TEST(f360_sort_clusters_by_priority, test_two_invalid_clusters_and_no_valid_cluster_at_all_with_saturation)
{
   /** \precond
    * Setting proper clusters as active
    */
   tracker_info.num_active_clusters = 2;
   tracker_info.active_cluster_ids[0] = 6; // invalid cluster
   tracker_info.active_cluster_ids[1] = 10; // invalid cluster

   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS; // setting saturation

   /** \action
    * Calling Sort_Clusters_By_Priority
    */
   Sort_Clusters_By_Priority(calibrations, det_hist, det_props, raw_detection_list, sensors, host, tracker_info, clusters, sorted_clusters_id_by_prior, num_predond_valid_clusters);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_EQUAL(0, num_predond_valid_clusters);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[0]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[1]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[2]);
   CHECK_EQUAL(0, sorted_clusters_id_by_prior[3]);
}
/** @}*/
