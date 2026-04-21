/** \file
 *- File with set of qualification tests (which are also unit test) for Sensor_Postprocessing module functions.
 */

#include "f360_sensor_postprocessing.h"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>
#include <iostream>
#include "f360_set_variant.h"

//Implementation of stubbed interfaces

/** \defgroup  Sensor_Postprocessing
 *  @{
 */
using namespace f360_variant_A;
/** \brief
*  This test suit aims  to test Update_Detection_History function
**/
TEST_GROUP(f360_Update_Detection_History_qualtest)
{
   //set up variables for sensor postprocessing
   F360_Tracker_Info_T tracker_info = {};
   F360_Detection_Hist_T det_hist = {};
   F360_Cluster_T clusters[NUMBER_OF_CLUSTERS] = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_TRKR_TIMING_INFO_T timing_info = {};
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

      sensors[0].variable.time_since_measurement_s = 0.1F;
      
      tracker_info.num_active_clusters = 1;
      clusters[0].ndets=1;
      clusters[0].detids[0]=1;
      tracker_info.active_cluster_ids[0]=1;  
      det_props[0].range_rate_dealiased = 0.5F;
      raw_detection_list.detections[0].raw.sensor_id = 1;
      raw_detection_list.detections[0].processed.vcs_az = 0.2F;
   }
   
   /** \teardown
   * Nothing to teardown in this test group
   **/
};

/**
*\purpose  verify that The tracker shall maintain a history of the detections that were used to form clusters
*\req   FTCP-12845
*/
TEST(f360_Update_Detection_History_qualtest, Test_Update_Detection_History_update_one_detection)
{
   /** \precond
    *- prepare one detection to be stored in detection history.
    **/  
   det_hist.n_occupied=1;
   float expected_rdot_history_data = det_props[0].range_rate_dealiased; //which equal 0.5F from the test data setup
   float expected_vcs_az_history_data = raw_detection_list.detections[0].processed.vcs_az; //which equal 0.2F from the test data setup
   float expected_time_since_meas_history_data = sensors[0].variable.time_since_measurement_s; //which equal 0.01F from the test data setup
   
   /** \action
    *- Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- Detections history is updated with the expected values from test setup
    **/
   DOUBLES_EQUAL(expected_rdot_history_data,det_hist.det_data[0].rdot ,test_tolerance)
   DOUBLES_EQUAL(expected_vcs_az_history_data,det_hist.det_data[0].vcs_az ,test_tolerance)
   DOUBLES_EQUAL(expected_time_since_meas_history_data,det_hist.det_data[0].time_since_meas ,test_tolerance)
}
/**
*\purpose  Verify that the oldest historical detection associated to the cluster is removed and replaced with the new one
*\req   FTCP-12845    
*/
TEST(f360_Update_Detection_History_qualtest, Test_Update_Detection_History_old_dets_greater_than_zero)
{
   /** \precond
    *- Set detection history occupation to MAX_NUMBER_OF_HISTORIC_DETECTIONS
    *- Set the cluster num_old_dets to  1 (higher than zero)
    *- Set old_det_idx to zero to set detection history index to zero(the index that will be replaced)
    **/
   det_hist.n_occupied=800; // number of historical detections exceed MAX_NUMBER_OF_HISTORIC_DETECTIONS
   clusters[0].num_old_dets =1;
   clusters[0].old_det_idx[0] =0;
   float expected_rdot_history_data = det_props[0].range_rate_dealiased; //which equal 0.5F from the test data setup
   float expected_vcs_az_history_data = raw_detection_list.detections[0].processed.vcs_az; //which equal 0.2F from the test data setup
   float expected_time_since_meas_history_data = sensors[0].variable.time_since_measurement_s; //which equal 0.01F from the test data setup
   
   /** \action
    *- Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- The oldest historical detection is replaced with the new one 
    **/
   DOUBLES_EQUAL(expected_rdot_history_data,det_hist.det_data[0].rdot ,test_tolerance)
   DOUBLES_EQUAL(expected_vcs_az_history_data,det_hist.det_data[0].vcs_az ,test_tolerance)
   DOUBLES_EQUAL(expected_time_since_meas_history_data,det_hist.det_data[0].time_since_meas ,test_tolerance)
}
/**
*\purpose Verify that no new detections is stored in the detections history when it reached max occupation  and there is no old detections in the cluster 
*\req   FTCP-12845  
*/
TEST(f360_Update_Detection_History_qualtest, Test_Update_Detection_History_n_occupied_higher_than_max)
{
   /** \precond
    *-  Set detection history occupation to MAX_NUMBER_OF_HISTORIC_DETECTIONS
    *-  Set cluster num_old_dets to zero
    **/
   det_hist.n_occupied=2010;
   clusters[0].num_old_dets=0;
   float expected_rdot_history_data = 0;
   float expected_vcs_az_history_data = 0;
   float expected_time_since_meas_history_data =0;
   
   /** \action
    *- Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- Detections history isn't updated and remains at the initial value 
    **/
   DOUBLES_EQUAL(expected_rdot_history_data,det_hist.det_data[0].rdot ,test_tolerance)
   DOUBLES_EQUAL(expected_vcs_az_history_data,det_hist.det_data[0].vcs_az ,test_tolerance)
   DOUBLES_EQUAL(expected_time_since_meas_history_data,det_hist.det_data[0].time_since_meas ,test_tolerance)
}


/**
*\purpose   Verify that no new detection is stored in  detection history structure when Find_Next_Hist_Det_Idx fails to find the next detection history index 
*\req   FTCP-12845    
*/
TEST(f360_Update_Detection_History_qualtest, Test_Update_Detection_History_Find_Next_Hist_Det_Idx_false)
{
   /** \precond
    *- Set the cluster num_old_dets to MAX_DETS_IN_OBJ 
    *- Set the cluster old_det_idx to MAX_NUMBER_OF_HISTORIC_DETECTIONS
    **/
   clusters[0].num_old_dets=80;
   clusters[0].old_det_idx[0]=2000;
   float expected_rdot_history_data = 0;
   float expected_vcs_az_history_data = 0;
   float expected_time_since_meas_history_data =0;
   
   /** \action
    *- Call Sensor_Postprocessing module function
    **/
   Sensor_Postprocessing(tracker_info, det_props, raw_detection_list, sensors, det_hist, clusters, timing_info);
   
   /** \result
    *- Find_Next_Hist_Det_Idx function fails to find new index and then Detections history isn't updated 
    **/
   DOUBLES_EQUAL(expected_rdot_history_data,det_hist.det_data[0].rdot ,test_tolerance)
   DOUBLES_EQUAL(expected_vcs_az_history_data,det_hist.det_data[0].vcs_az ,test_tolerance)
   DOUBLES_EQUAL(expected_time_since_meas_history_data,det_hist.det_data[0].time_since_meas ,test_tolerance)
}

/** @}*/
