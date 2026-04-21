/** \file
   File contains test cases for Clusters_Preprocessing() function
*/

#include "f360_clusters_preprocessing.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"

//sneak in mocked functions
//Declaration of stubbed/mock functions

//Implementation of stubbed interfaces

using namespace f360_variant_A;

/** \defgroup  f360_clusters_preprocessing
 *  @{
 */

/** \brief
 *  Group of test for Clusters_Preprocessing function
 */

TEST_GROUP(f360_clusters_preprocessing)
{
   /** \setup
   * Seting up default tolerance, calibrations, clusters, historical detections, tracker info
   */
   float32_t tolerance = 10e-6F;
   F360_Calibrations_T calib;
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS];
   F360_Detection_Hist_T det_hist;
   F360_Tracker_Info_T tracker_info;
   F360_Host_Props_T host_props;

   TEST_SETUP()
   {
      calib.max_age_of_older_amb_dets = 0.350F;
      calib.max_age_of_older_dets_lr = 0.750F;
      for (int32_t cluester_idx = 0; cluester_idx < NUMBER_OF_CLUSTERS; cluester_idx++)
      {
         clusters[cluester_idx] = {};
      }
      det_hist = {};
      tracker_info = {};

      Set_Tracker_Variant(tracker_info.variant);

      // Preparing some clusters
      tracker_info.num_active_clusters = 6;
      tracker_info.active_cluster_ids[0] = 2;
      tracker_info.active_cluster_ids[1] = 4;
      tracker_info.active_cluster_ids[2] = 6;
      tracker_info.active_cluster_ids[3] = 7;
      tracker_info.active_cluster_ids[4] = 10;
      tracker_info.active_cluster_ids[5] = 16;

      clusters[1].f_to_be_killed = false;
      clusters[1].id = 2;
      clusters[1].vcs_position.x = 11.4F;
      clusters[1].vcs_position.y = 12.4F;
      clusters[1].rep_vcs_az =  0.8F;

      clusters[3].f_to_be_killed = true;
      clusters[3].id = 4;
      clusters[3].vcs_position.x = 11.0F;
      clusters[3].vcs_position.y = 12.0F;
      clusters[3].rep_vcs_az = 0.8F;

      clusters[5].f_to_be_killed = false;
      clusters[5].id = 6;
      clusters[5].vcs_position.x = 5.5F;
      clusters[5].vcs_position.y = 4.4F;
      clusters[5].rep_vcs_az = 0.6F;

      clusters[6].f_to_be_killed = true;
      clusters[6].id = 7;
      clusters[6].vcs_position.x = 23.3F;
      clusters[6].vcs_position.y = -45.8F;
      clusters[6].rep_vcs_az = -1.1F;

      clusters[9].f_to_be_killed = true;
      clusters[9].id = 10;
      clusters[9].vcs_position.x = 9.9F;
      clusters[9].vcs_position.y = 9.9F;
      clusters[9].rep_vcs_az = 0.7F;

      clusters[15].f_to_be_killed = false;
      clusters[15].id = 16;
      clusters[15].vcs_position.x = 20.0F;
      clusters[15].vcs_position.y = 20.0F;
      clusters[15].rep_vcs_az = -0.7F;

      // Preparing some historical detections
      det_hist.n_occupied = 5;
      det_hist.max_occupation = 5;

      det_hist.det_data[0].vcs_position.x = 10.0F;
      det_hist.det_data[0].vcs_position.y = 11.0F;
      det_hist.det_data[0].time_since_meas = 0.3F;
      det_hist.det_data[0].f_is_range_in_all_looks = false;
      det_hist.f_idx_occupied[0] = true;

      det_hist.det_data[1].vcs_position.x = 5.2F;
      det_hist.det_data[1].vcs_position.y = 1.5F;
      det_hist.det_data[1].time_since_meas = 0.3F;
      det_hist.det_data[1].f_is_range_in_all_looks = false;
      det_hist.f_idx_occupied[1] = true;

      det_hist.det_data[2].vcs_position.x = 20.0F;
      det_hist.det_data[2].vcs_position.y = 19.9F;
      det_hist.det_data[2].time_since_meas = 0.5F;
      det_hist.det_data[2].f_is_range_in_all_looks = true;
      det_hist.f_idx_occupied[2] = true;

      det_hist.det_data[3].vcs_position.x = 22.0F;
      det_hist.det_data[3].vcs_position.y = 12.9F;
      det_hist.det_data[3].time_since_meas = 0.5F;
      det_hist.det_data[3].f_is_range_in_all_looks = false;
      det_hist.f_idx_occupied[3] = true;

      det_hist.det_data[4].vcs_position.x = 22.0F;
      det_hist.det_data[4].vcs_position.y = 12.9F;
      det_hist.det_data[4].time_since_meas = 0.9F;
      det_hist.det_data[4].f_is_range_in_all_looks = false;
      det_hist.f_idx_occupied[4] = true;

      // Preparing host properties
      host_props.delta_pointing = 0.0125F;
      host_props.cos_delta_pointing = 0.999921876017247F;
      host_props.sin_delta_pointing = 0.012499674481710F;
      host_props.delta_position.x = 1.2498F;
      host_props.delta_position.y = 0.0164F;
   }

   /** \teardown
    * Nothing to teardown in this test group
    */
   TEST_TEARDOWN()
   {
      //mock.clear();
   }

};

/**
*\purpose  Check if historical detections are removed or stay in clusters
*\req    NA
*/
TEST(f360_clusters_preprocessing, test_two_hist_dets_stay_in_cluster)
{
   /** \precond
    * Adding 2 detections with time since meas lower than treshold to cluster that should not be killed
    */
   clusters[0].num_old_dets = 2;
   clusters[0].old_det_idx[0] = 0;
   clusters[0].old_det_idx[1] = 1;


   /** \action
    * Calling Clusters_Preprocessing()
    */
   Clusters_Preprocessing(calib, host_props, clusters, det_hist, tracker_info);

   /** \result
    * Cluster shoud not be removed and shall have 2 hist dets
    */
   CHECK_EQUAL(5, det_hist.n_occupied); // no detections should be removed
   CHECK_EQUAL(2, clusters[0].num_old_dets);
   DOUBLES_EQUAL(10.0F, det_hist.det_data[0].vcs_position.x, tolerance);
   DOUBLES_EQUAL(11.0F, det_hist.det_data[0].vcs_position.y, tolerance);
   CHECK_TRUE(det_hist.f_idx_occupied[0]);
   DOUBLES_EQUAL(5.2F, det_hist.det_data[1].vcs_position.x, tolerance);
   DOUBLES_EQUAL(1.5F, det_hist.det_data[1].vcs_position.y, tolerance);
   CHECK_TRUE(det_hist.f_idx_occupied[1]);
}

/**
*\purpose  Check if historical detections are removed or stay in clusters
*\req    NA
*/
TEST(f360_clusters_preprocessing, test_two_hist_dets_do_not_stay_in_cluster)
{
   /** \precond
    * Adding 2 detections with time since meas higher than treshold to cluster that should be killed
    */
   clusters[5].num_old_dets = 2;
   clusters[5].old_det_idx[0] = 2;
   clusters[5].old_det_idx[1] = 4;


   /** \action
    * Calling Clusters_Preprocessing()
    */
   Clusters_Preprocessing(calib, host_props, clusters, det_hist, tracker_info);

   /** \result
    * Cluster should be killed and has no hist dets
    */
   CHECK_EQUAL(3, det_hist.n_occupied); // 2 detections should be removed
   CHECK_EQUAL(0, clusters[5].num_old_dets);
   DOUBLES_EQUAL(0.0F, det_hist.det_data[2].vcs_position.x, tolerance);
   DOUBLES_EQUAL(0.0F, det_hist.det_data[2].vcs_position.y, tolerance);
   CHECK_FALSE(det_hist.f_idx_occupied[2]);
   DOUBLES_EQUAL(0.0F, det_hist.det_data[4].vcs_position.x, tolerance);
   DOUBLES_EQUAL(0.0F, det_hist.det_data[4].vcs_position.y, tolerance);
   CHECK_FALSE(det_hist.f_idx_occupied[4]);
   // Cluster should also be clean 
   DOUBLES_EQUAL(0.0F, clusters[5].vcs_position.x, tolerance);
   DOUBLES_EQUAL(0.0F, clusters[5].vcs_position.y, tolerance);
   CHECK_FALSE(clusters[5].f_to_be_killed);
   CHECK_EQUAL(6, clusters[5].id);
}

TEST(f360_clusters_preprocessing, test_one_of_three_dets_should_be_removed_from_cluster)
{
   /** \precond
    * Adding 3 detections with time since meas lower than treshold to cluster that should not be killed. One det should be removed
    */
   clusters[5].num_old_dets = 3;
   clusters[5].old_det_idx[0] = 2;
   clusters[5].old_det_idx[1] = 0;
   clusters[5].old_det_idx[2] = 1;


   /** \action
    * Calling Clusters_Preprocessing()
    */
   Clusters_Preprocessing(calib, host_props, clusters, det_hist, tracker_info);

   /** \result
    * Cluster shoud not be removed and shall have 2 hist dets
    */
   CHECK_EQUAL(4, det_hist.n_occupied); // 1 detections should be removed
   CHECK_EQUAL(2, clusters[5].num_old_dets);
   DOUBLES_EQUAL(0.0F, det_hist.det_data[2].vcs_position.x, tolerance);
   DOUBLES_EQUAL(0.0F, det_hist.det_data[2].vcs_position.y, tolerance);
   CHECK_FALSE(det_hist.f_idx_occupied[2]);
   DOUBLES_EQUAL(10.0F, det_hist.det_data[0].vcs_position.x, tolerance);
   DOUBLES_EQUAL(11.0F, det_hist.det_data[0].vcs_position.y, tolerance);
   CHECK_TRUE(det_hist.f_idx_occupied[0]);
   DOUBLES_EQUAL(5.2F, det_hist.det_data[1].vcs_position.x, tolerance);
   DOUBLES_EQUAL(1.5F, det_hist.det_data[1].vcs_position.y, tolerance);
   CHECK_TRUE(det_hist.f_idx_occupied[1]);
}

/**
*\purpose  Check that the VCS properties of all clusters that are not killed have been updated
*\req    NA
*/
TEST(f360_clusters_preprocessing, test_vcs_props_updated)
{
   /** \precond
    * Assigning detections to clusters that should not be killed.
    * Setting up expected data for vcs properties of clusters
    */
   clusters[1].num_old_dets = 1;
   clusters[1].old_det_idx[0] = 0;
   float32_t expected_az_2 = 0.787500000000000F;
   float32_t expected_long_pos_2 = 10.304197994861966F;
   float32_t expected_lat_pos_2 = 12.255758347922933F;

   clusters[15].num_old_dets = 1;
   clusters[15].old_det_idx[0] = 1;
   float32_t expected_az_16 = -0.712500000000000F;
   float32_t expected_long_pos_16 = 18.998523654671285F;
   float32_t expected_lat_pos_16 = 19.747667405111308F;

   /** \action
    * Calling Clusters_Preprocessing()
    */
   Clusters_Preprocessing(calib, host_props, clusters, det_hist, tracker_info);

   /** \result
    * The VCS properties of clusters that are still active should have updated vcs position and vcs azimuth
    */
   DOUBLES_EQUAL_TEXT(expected_az_2, clusters[1].rep_vcs_az, tolerance, "Unexpected vcs azimuth for cluster with id 2");
   DOUBLES_EQUAL_TEXT(expected_long_pos_2, clusters[1].vcs_position.x, tolerance, "Unexpected vcs longitudinal position for cluster with id 2");
   DOUBLES_EQUAL_TEXT(expected_lat_pos_2, clusters[1].vcs_position.y, tolerance, "Unexpected vcs lateral position for cluster with id 2");

   DOUBLES_EQUAL_TEXT(expected_az_16, clusters[15].rep_vcs_az, tolerance, "Unexpected vcs azimuth for cluster with id 16");
   DOUBLES_EQUAL_TEXT(expected_long_pos_16, clusters[15].vcs_position.x, tolerance, "Unexpected vcs longitudinal position for cluster with id 16");
   DOUBLES_EQUAL_TEXT(expected_lat_pos_16, clusters[15].vcs_position.y, tolerance, "Unexpected vcs lateral position for cluster with id 16");
}
/** @}*/


/** \defgroup  f360_clusters_preprocessing_correct_cluster_vcs_props
 *  @{
 */

/** \brief
 *  Test group for the Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion() function
 */

TEST_GROUP(f360_clusters_preprocessing_correct_cluster_vcs_props)
{
   /** \setup
   * Setting up default tolerance, calibrations, cluster, and host props
   */
   float32_t tolerance = 10e-6F;
   F360_Cluster_T cluster = {};
   F360_Host_Props_T host_props = {};

   TEST_SETUP()
   {
      cluster.vcs_position.x = -30.6F;
      cluster.vcs_position.y = 3.2F;
      cluster.rep_vcs_az = 3.0374F;
   }

   /** \teardown
    * Nothing to tear down in this test group
    */
   TEST_TEARDOWN()
   {
   }

};

/**
*\purpose  Check if the VCS properties of a cluster is correctly modified when host is driving forward and turning right
*\req    NA
*/
TEST(f360_clusters_preprocessing_correct_cluster_vcs_props, test_host_driving_forward_turning_right)
{
   /** \precond
    * Setting up host props to correspond to a case where host is driving forward and turning right.
    * Also setting up expected output
    */
   host_props.delta_pointing = 0.0125F;
   host_props.cos_delta_pointing = 0.999921876017247F;
   host_props.sin_delta_pointing = 0.012499674481710F;
   host_props.delta_position.x = 1.2498F;
   host_props.delta_position.y = 0.0164F;

   float32_t expected_azimuth = 3.024900000000000F;
   float32_t expected_long_pos = -31.807517803094154F;
   float32_t expected_lat_pos = 3.581463416796069F;



   /** \action
    * Calling Clusters_Preprocessing()
    */
   Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion(host_props, cluster);

   /** \result
    * Check that cluster VCS properties correspond to expected data
    */
   DOUBLES_EQUAL_TEXT(expected_azimuth, cluster.rep_vcs_az, tolerance, "Unexpected cluster VCS azimuth angle");
   DOUBLES_EQUAL_TEXT(expected_long_pos, cluster.vcs_position.x, tolerance, "Unexpected cluster VCS longitudinal position");
   DOUBLES_EQUAL_TEXT(expected_lat_pos, cluster.vcs_position.y, tolerance, "Unexpected cluster VCS lateral position");
}

/**
*\purpose  Check if the VCS properties of a cluster is correctly modified when host is driving forward and turning left
*\req    NA
*/
TEST(f360_clusters_preprocessing_correct_cluster_vcs_props, test_host_driving_forward_turning_left)
{
   /** \precond
    * Setting up host props to correspond to a case where host is driving forward and turning left.
    * Also setting up expected output
    */
   host_props.delta_pointing = -0.0125F;
   host_props.cos_delta_pointing = 0.999921876017247F;
   host_props.sin_delta_pointing = -0.012499674481710F;
   host_props.delta_position.x = 1.2498F;
   host_props.delta_position.y = -0.0164F;

   float32_t expected_azimuth = 3.049900000000000F;
   float32_t expected_long_pos = -31.887515719777099F;
   float32_t expected_lat_pos = 2.818036589714314F;



   /** \action
    * Calling Clusters_Preprocessing()
    */
   Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion(host_props, cluster);

   /** \result
    * Check that cluster VCS properties correspond to expected data
    */
   DOUBLES_EQUAL_TEXT(expected_azimuth, cluster.rep_vcs_az, tolerance, "Unexpected cluster VCS azimuth angle");
   DOUBLES_EQUAL_TEXT(expected_long_pos, cluster.vcs_position.x, tolerance, "Unexpected cluster VCS longitudinal position");
   DOUBLES_EQUAL_TEXT(expected_lat_pos, cluster.vcs_position.y, tolerance, "Unexpected cluster VCS lateral position");
}

/**
*\purpose  Check if the VCS properties of a cluster is correctly modified when host is reversing and turning right
*\req    NA
*/
TEST(f360_clusters_preprocessing_correct_cluster_vcs_props, test_host_reversing_turning_right)
{
   /** \precond
    * Setting up host props to correspond to a case where host is reversing and turning right.
    * Also setting up expected output
    */
   host_props.delta_pointing = -0.0070F;
   host_props.cos_delta_pointing = 0.999975500100042F;
   host_props.sin_delta_pointing = -0.006999942833473F;
   host_props.delta_position.x = -0.1373F;
   host_props.delta_position.y = 0.0267F;

   float32_t expected_azimuth = 3.044400000000000F;
   float32_t expected_long_pos = -30.484166585490996F;
   float32_t expected_lat_pos = 2.959985095914212F;



   /** \action
    * Calling Clusters_Preprocessing()
    */
   Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion(host_props, cluster);

   /** \result
    * Check that cluster VCS properties correspond to expected data
    */
   DOUBLES_EQUAL_TEXT(expected_azimuth, cluster.rep_vcs_az, tolerance, "Unexpected cluster VCS azimuth angle");
   DOUBLES_EQUAL_TEXT(expected_long_pos, cluster.vcs_position.x, tolerance, "Unexpected cluster VCS longitudinal position");
   DOUBLES_EQUAL_TEXT(expected_lat_pos, cluster.vcs_position.y, tolerance, "Unexpected cluster VCS lateral position");
}

/**
*\purpose  Check if the VCS properties of a cluster is correctly modified when host is reversing and turning left
*\req    NA
*/
TEST(f360_clusters_preprocessing_correct_cluster_vcs_props, test_host_reversing_turning_left)
{
   /** \precond
    * Setting up host props to correspond to a case where host is reversing and turning left.
    * Also setting up expected output
    */
   host_props.delta_pointing = 0.0070F;
   host_props.cos_delta_pointing = 0.999975500100042F;
   host_props.sin_delta_pointing = 0.006999942833473F;
   host_props.delta_position.x = -0.1373F;
   host_props.delta_position.y = 0.0272F;

   float32_t expected_azimuth = 3.030400000000000F;
   float32_t expected_long_pos = -30.439744248275492F;
   float32_t expected_lat_pos = 3.385959425270662F;



   /** \action
    * Calling Clusters_Preprocessing()
    */
   Correct_Cluster_VCS_Props_Based_On_Host_Delta_Motion(host_props, cluster);

   /** \result
    * Check that cluster VCS properties correspond to expected data
    */
   DOUBLES_EQUAL_TEXT(expected_azimuth, cluster.rep_vcs_az, tolerance, "Unexpected cluster VCS azimuth angle");
   DOUBLES_EQUAL_TEXT(expected_long_pos, cluster.vcs_position.x, tolerance, "Unexpected cluster VCS longitudinal position");
   DOUBLES_EQUAL_TEXT(expected_lat_pos, cluster.vcs_position.y, tolerance, "Unexpected cluster VCS lateral position");
}
/** @}*/

/** \defgroup  f360_is_det_valid_to_keep
 *  @{
 */

 /** \brief
  *  Group of test for Is_Det_Valid_To_Keep function
  */


TEST_GROUP(f360_is_det_valid_to_keep)
{
   /** \setup
   * Setting up default calibrations.
   */ 
   F360_Calibrations_T calib;
   F360_Detection_Hist_Data_T det_hist = {};

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }

};

/**
*\purpose  Check if function returns true for ambiguous detection which is younger then max age of ambiguous detection.
*\req    NA
*/
TEST(f360_is_det_valid_to_keep, test_valid_amb_det)
{
   /** \precond
    *  Define ambiguous detection with time since created below threshold.
    */
   det_hist.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   det_hist.time_since_meas = calib.max_age_of_older_amb_dets - 0.1F;
   det_hist.f_is_range_in_all_looks = true;

   /** \action
    * Call Is_Det_Valid_To_Keep()
    */
   const bool f_det_valid_too_keep =  Is_Det_Valid_To_Keep(calib, det_hist);

   /** \result
    *  Check if function returns true.
    */
   CHECK_TRUE(f_det_valid_too_keep);
}

/**
*\purpose  Check if function return false for ambiguous detection which is older then max age of any detection.
*\req    NA
*/
TEST(f360_is_det_valid_to_keep, test_invalid_amb_det)
{
   /** \precond
    *  Define ambiguous detection with time since created above threshold.
    */

   det_hist.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_AMBIGUOUS;
   det_hist.time_since_meas = calib.max_age_of_older_dets_lr + 0.1F;
   det_hist.f_is_range_in_all_looks = true;

   /** \action
    * Call Is_Det_Valid_To_Keep()
    */
   const bool f_det_valid_too_keep = Is_Det_Valid_To_Keep(calib, det_hist);

   /** \result
    *  Check if function returns false.
    */
   CHECK_FALSE(f_det_valid_too_keep);
}

/**
*\purpose  Check if function return true for moving detection which is younger then max age of any detection.
*\req    NA
*/
TEST(f360_is_det_valid_to_keep, test_valid_moving_det)
{
   /** \precond
    *  Define moving detection with time since created below threshold.
    */

   det_hist.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_hist.time_since_meas = calib.max_age_of_older_no_amb_dets - 0.1F;
   det_hist.f_is_range_in_all_looks = true;

   /** \action
    * Call Is_Det_Valid_To_Keep()
    */
   const bool f_det_valid_too_keep = Is_Det_Valid_To_Keep(calib, det_hist);

   /** \result
    *  Check if function returns true.
    */
   CHECK_TRUE(f_det_valid_too_keep);
}

/**
*\purpose  Check if function return false for moving detection which is older then max age of any detection.
*\req    NA
*/
TEST(f360_is_det_valid_to_keep, test_invalid_moving_det)
{
   /** \precond
    *  Define ambiguous detection with time since created above threshold.
    */
   det_hist.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_hist.time_since_meas = calib.max_age_of_older_no_amb_dets + 0.1F;
   det_hist.f_is_range_in_all_looks = true;

   /** \action
    * Call Is_Det_Valid_To_Keep()
    */
   const bool f_det_valid_too_keep = Is_Det_Valid_To_Keep(calib, det_hist);

   /** \result
    *  Check if function returns false.
    */
   CHECK_FALSE(f_det_valid_too_keep);
}

/**
*\purpose  Check if function return true for moving detection which is in long range and is younger than max age in long range.
*\req    NA
*/
TEST(f360_is_det_valid_to_keep, test_valid_moving_det_from_long_look)
{
   /** \precond
    *  Define moving detection from long range and is young enough.
    */
   det_hist.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_hist.time_since_meas = calib.max_age_of_older_dets_lr - 0.01F;
   det_hist.f_is_range_in_all_looks = false;

   /** \action
    * Call Is_Det_Valid_To_Keep()
    */
   const bool f_det_valid_too_keep = Is_Det_Valid_To_Keep(calib, det_hist);

   /** \result
    *  Check if function returns true.
    */
   CHECK_TRUE(f_det_valid_too_keep);
}

/**
*\purpose  Check if function return false for moving detection which is in long range and is older than max age in long range.
*\req    NA
*/
TEST(f360_is_det_valid_to_keep, test_invalid_moving_det_from_long_look)
{
   /** \precond
    *  Define moving detection from long range and is young enough.
    */
   det_hist.motion_status = rspp_variant_A::RSPP_DETECTION_MOTION_STATUS_MOVING;
   det_hist.time_since_meas = calib.max_age_of_older_dets_lr + 1.00F;
   det_hist.f_is_range_in_all_looks = false;

   /** \action
    * Call Is_Det_Valid_To_Keep()
    */
   const bool f_det_valid_too_keep = Is_Det_Valid_To_Keep(calib, det_hist);

   /** \result
    *  Check if function returns false.
    */
   CHECK_FALSE(f_det_valid_too_keep);
}

/** @}*/
