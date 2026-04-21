/** \file
 * This file aims to test sensor_postprocessing function in a pure unit test methode.
 */

#include "f360_sensor_postprocessing.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>


/** \defgroup  f360_Find_Next_Hist_Det_Idx_qualtest
 *  @{
 */
using namespace f360_variant_A;
/** \brief
*  This test suit aims  to test Find_Next_Hist_Det_Idx function
* Note: other sensor_postprocessing functions are being tested at the f360_sensor_postprocessing_qualtest_unittest.cpp file.
**/
TEST_GROUP(f360_Find_Next_Hist_Det_Idx_qualtest)
{
    //set up variables for sensor postprocessing
   F360_Tracker_Info_T tracker_info = {};
   F360_Detection_Hist_T det_hist={};
   F360_Cluster_T     clusters[NUMBER_OF_CLUSTERS]={};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS]={};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_TRKR_TIMING_INFO_T    timing_info={};  
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   float32_t test_tolerance = 1.0e-6F; 
   
   /** \setup
    * Set common variables used in all tests.
    * Setup one detection and its properties as follows:
    *    range_rate_dealiased to 0.5F
    *    vcs_az to 0.2F
    *    time_since_measurement to 0.01F
    **/
   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);

      tracker_info.num_active_clusters = 1;
      clusters[0].ndets=1;
      clusters[0].detids[0]=1;
      tracker_info.active_cluster_ids[0]=1;
      det_hist.n_occupied=1;
      raw_detection_list.detections[0].processed.vcs_az = 0.2F;
   }
   
   /** \teardown
   * Nothing to teardown in this test group
   **/
};
/**
*\purpose  Verify that detection history max occupation is updated correctly
*\req    NA
*/
TEST(f360_Find_Next_Hist_Det_Idx_qualtest, Test_Find_Next_Hist_Det_Idx_max_occupation_lower_than_n_occupied)
{
   /** \precond
    *- Set the detection history n_occupied higher than max occupation
    **/
   det_hist.n_occupied=3;
   det_hist.max_occupation=2;
   int expected_max_occupation_value = 4; // because n_occupied will be incremented by 1
   
   /** \action
    *-Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- detection history max occupation is updated with the higher value 
    **/
   DOUBLES_EQUAL(expected_max_occupation_value,det_hist.max_occupation,test_tolerance)
}
/**
*\purpose  Verify that detection history max occupation is updated correctly 
*\req    NA
*/
TEST(f360_Find_Next_Hist_Det_Idx_qualtest, Test_Find_Next_Hist_Det_Idx_max_occupation_greater_than_n_occupied )
{
   /** \precond
    *- Set the detection history number of occupation lower than max occupation
    **/
   det_hist.n_occupied=1;
   det_hist.max_occupation=2;
   int expected_max_occupation_value = 2;

   /** \action
    *-Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- detection history max occupation is updated with the higher value 
    **/
   DOUBLES_EQUAL(expected_max_occupation_value,det_hist.max_occupation,test_tolerance)
}
/**
*\purpose Verify that no new detections is added when there is no empty space in detections history
*\req   NA
*/
TEST(f360_Find_Next_Hist_Det_Idx_qualtest, Test_Find_Next_Hist_Det_Idx_no_empty_space)
{
   /** \precond
    *- Set all the detection history f_idx_occupied to true, leave no empty spaces
   **/
   det_hist.n_occupied=1;
   for (uint32_t k = 0U; (k < MAX_NUMBER_OF_HISTORIC_DETECTIONS);k++)
   {
   det_hist.f_idx_occupied[k]=true;
   }
   float expected_rdot_history_data = 0;
   float expected_vcs_az_history_data = 0;
   float expected_time_since_meas_history_data = 0;
   
   /** \action
    *-Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- no new detection is added
    **/
   DOUBLES_EQUAL(expected_rdot_history_data,det_hist.det_data[0].rdot ,test_tolerance)
   DOUBLES_EQUAL(expected_vcs_az_history_data,det_hist.det_data[0].vcs_az ,test_tolerance)
   DOUBLES_EQUAL(expected_time_since_meas_history_data,det_hist.det_data[0].time_since_meas ,test_tolerance)
}
/** @}*/
