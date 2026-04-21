/** \file
 * This file contains unit tests for content of f360_is_cluster_visible.cpp file
 */

#include "f360_is_cluster_visible.h"
#include <CppUTest/TestHarness.h>
#include "f360_occlusion_ut_helpers.h"

using namespace f360_variant_A;

/** \defgroup  f360_is_cluster_visible
 *  @{
 */

 /** \brief
  * Test group of Is_Cluster_Visible function. 
  */
TEST_GROUP(f360_is_cluster_visible)
{
   F360_Cluster_T cluster = {};
   F360_Calibrations_T calibrations = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Tracker_Info_T tracker_info = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_TRKR_TIMING_INFO_T tracker_time_info = {};
   bool f_enable_occlusion = true;

   /** \setup
   * Setting up calibrations and sensor information
   * Setting up two existing object in front front left and front right quarter of VCS
   **/
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);

      Set_Left_Front_Sensor(sensors[0]);
      Set_Right_Front_Sensor(sensors[1]);
      Set_Left_Rear_Sensor(sensors[2]);
      Set_Right_Rear_Sensor(sensors[3]);

      Set_Base_Object_Parameters(object_tracks[0]);
      object_tracks[0].reference_point = F360_REFERENCE_POINT_REAR_RIGHT;
      object_tracks[0].confidenceLevel = 1.0F;
      Point center = {5.0F, -5.0F};
      object_tracks[0].bbox.Set_Center(center);
      object_tracks[0].vcs_position = object_tracks[0].bbox.Get_Corners().Rear_Right();



      Set_Base_Object_Parameters(object_tracks[1]);
      object_tracks[1].reference_point = F360_REFERENCE_POINT_REAR_LEFT;
      object_tracks[1].confidenceLevel = 1.0F;
      center = {5.0F, 5.0F};
      object_tracks[1].bbox.Set_Center(center);
      object_tracks[1].vcs_position = object_tracks[1].bbox.Get_Corners().Rear_Left();

      tracker_info.num_active_objs = 2;
      tracker_info.active_obj_ids[0] = 1;
      tracker_info.active_obj_ids[1] = 2;
   }
};


/**
*\purpose  Check if cluster is marked as visible when it contains single detection and it's positioned
*          inside of field of view of the sensor from which the detection is comming from
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Visible_Cluster_With_Single_Detection_Is_Marked_As_Visible)
{
   /** \precond
    * Set cluster with a single detection at position inside field of view of rear left sensor
    * Sensor is not occluded by any object
    */
   cluster.vcs_position.x = -10.0F;
   cluster.vcs_position.y = -5.0F;
   cluster.detids[0] = 1;
   cluster.ndets = 1;
   raw_detection_list.detections[0].raw.sensor_id = 3;

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if cluster was determined to be visible
    */
   CHECK_TRUE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as non visible when it contains single detection and it's position
*          is occluded by exisiting object
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Occluded_Cluster_With_Single_Detection_Is_Marked_As_Non_Visible)
{
   /** \precond
    * Set cluster with a single detection at position within FOV of front right sensor
    * Occluding object position was set in TEST_SETUP step
    */
   cluster.vcs_position.x = 10.0F;
   cluster.vcs_position.y = 9.0F;
   cluster.detids[0] = 1;
   cluster.ndets = 1;
   raw_detection_list.detections[0].raw.sensor_id = 2;

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if returned cluster status is non visible
    */
   CHECK_FALSE(f_cluster_visible);
}


/**
*\purpose  Check if cluster is marked as non visible when it contains single detection and it's positioned 
*          on the edge of occlusion field of the sensor from which the detection is comming from
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_On_Edge_Cluster_With_Single_Detection_Is_Marked_As_Visible)
{
   /** \precond
    * Set cluster with a single detection at position on the edge of occlusion field for front right sensor
    * Occluding object position was set in TEST_SETUP step
    */
   cluster.vcs_position.x = 4.2F;
   cluster.vcs_position.y = 4.2F;
   cluster.detids[0] = 1;
   cluster.ndets = 1;
   raw_detection_list.detections[0].raw.sensor_id = 2;

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if cluster was determined as non visible
    */
   CHECK_FALSE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as visible when it contains two detections from the same sensor
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Visible_Cluster_With_Two_Detections_From_The_Same_Sensoe_Is_Marked_As_Visible)
{
   /** \precond
    * Set cluster with a two detections at position inside field of view of rear left sensor
    * Sensor is not occluded by any object
    */
   cluster.vcs_position.x = -11.0F;
   cluster.vcs_position.y = -6.0F;
   cluster.detids[0] = 1;
   cluster.detids[1] = 2;
   cluster.ndets = 2;
   raw_detection_list.detections[0].raw.sensor_id = 3;
   raw_detection_list.detections[1].raw.sensor_id = 3;

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if cluster was determined to be visible
    */
   CHECK_TRUE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as non visible when it contains two detections and it's position
*          is occluded by exisiting object
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Occluded_Cluster_With_Two_Detections_From_The_Same_Sensor_Is_Marked_As_Non_Visible)
{
   /** \precond
    * Set cluster with two detections at position within FOV of front right sensor
    * Occluding object position was set in TEST_SETUP step
    */
   cluster.vcs_position.x = 10.0F;
   cluster.vcs_position.y = 9.0F;
   cluster.detids[0] = 1;
   cluster.detids[1] = 2;
   cluster.ndets = 2;
   raw_detection_list.detections[0].raw.sensor_id = 2;
   raw_detection_list.detections[1].raw.sensor_id = 2;

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if returned cluster status is non visible
    */
   CHECK_FALSE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as visible when it contains two detections from different sensors and for both
*          sensors cluster position is visible
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Cluster_With_Two_Detections_From_Two_Different_Sensors_Is_Marked_As_Visible_When_Cluster_Visible_For_Both_Sensors)
{
   /** \precond
    * Set cluster with a two detections at position inside field of view of rear left sensor and rear right
    * Sensor is not occluded by any object
    */
   cluster.vcs_position.x = -20.0F;
   cluster.vcs_position.y = -0.5F;
   cluster.detids[0] = 1;
   cluster.detids[1] = 2;
   cluster.ndets = 2;
   raw_detection_list.detections[0].raw.sensor_id = 3;
   raw_detection_list.detections[1].raw.sensor_id = 4;

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if cluster was determined to be visible
    */
   CHECK_TRUE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as non visible when it contains two detections from different sensors and for both
*          sensors cluster position is non visible
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Cluster_With_Two_Detections_From_Different_Sensors_Is_Marked_As_Non_Visible_When_Cluster_Occluded_For_Both_Sensors)
{
   /** \precond
    * Set cluster with two detections at position within FOV of front right and front left sensor
    * Occluding object position was set in TEST_SETUP step, modify lateral position of second existing object to occlude front left sensor
    */
   cluster.vcs_position.x = 9.0F;
   cluster.vcs_position.y = 5.0F;
   cluster.detids[0] = 1;
   cluster.detids[1] = 2;
   cluster.ndets = 2;
   raw_detection_list.detections[0].raw.sensor_id = 1;
   raw_detection_list.detections[1].raw.sensor_id = 2;
   Point center = {object_tracks[1].bbox.Get_Center().x, 2.0F};
   object_tracks[1].bbox.Set_Center(center);
   object_tracks[1].vcs_position = object_tracks[1].bbox.Get_Corners().Rear_Left();
   

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if returned cluster status is non visible
    */
   CHECK_FALSE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as visible when it contains two detections from different sensors and for first
*          sensors the cluster is visible, but it's occluded for the second sensor
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Cluster_With_Two_Detections_From_Different_Sensors_Is_Marked_As_Visible_When_Visible_From_First_Sensor)
{
   /** \precond
    * Set cluster with two detections at position within FOV of front right and front left sensor
    * Occluding object position was set in TEST_SETUP step, modify lateral position of second existing object to occlude front right sensor
    * Cluster is occluded for first sensor, but visible for the second sensor
    */
   cluster.vcs_position.x = 15.0F;
   cluster.vcs_position.y = -10.0F;
   cluster.detids[0] = 1;
   cluster.detids[1] = 2;
   cluster.ndets = 2;
   raw_detection_list.detections[0].raw.sensor_id = 1;
   raw_detection_list.detections[1].raw.sensor_id = 2;
   Point center = {object_tracks[0].bbox.Get_Center().x, -1.0F};
   object_tracks[0].bbox.Set_Center(center);
   object_tracks[0].vcs_position = object_tracks[0].bbox.Get_Corners().Rear_Right();

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if returned cluster status is visible
    */
   CHECK_TRUE(f_cluster_visible);
}

/**
*\purpose  Check if cluster is marked as visible when it contains two detections from different sensors and for first
*          sensors the cluster is occluded, but it's visible for the second sensor
*\req    NA
*/
TEST(f360_is_cluster_visible, Check_If_Cluster_With_Two_Detections_From_Different_Sensors_Is_Marked_As_Visible_When_Visible_From_Second_Sensor)
{
   /** \precond
    * Set cluster with two detections at position within FOV of front right and front left sensor
    * Occluding object position was set in TEST_SETUP step, modify lateral position of second existing object to occlude front left sensor
    * Cluster is occluded for first sensor, but visible for the second sensor
    */
   cluster.vcs_position.x = 15.0F;
   cluster.vcs_position.y = -1.0F;
   cluster.detids[0] = 1;
   cluster.detids[1] = 2;
   cluster.ndets = 2;
   raw_detection_list.detections[0].raw.sensor_id = 1;
   raw_detection_list.detections[1].raw.sensor_id = 2;
   Point center = {object_tracks[0].bbox.Get_Center().x, -1.0F};
   object_tracks[1].bbox.Set_Center(center);
   object_tracks[1].vcs_position = object_tracks[1].bbox.Get_Corners().Rear_Left();

   Occlusion_T occlusion(f_enable_occlusion, calibrations, tracker_info, sensors, tracker_time_info, object_tracks);

   /** \action
    * Calling Is_Cluster_Visible function
    */
   bool f_cluster_visible = Is_Cluster_Visible(occlusion, raw_detection_list, cluster);

   /** \result
    * Check if returned cluster status is visible
    */
   CHECK_TRUE(f_cluster_visible);
}

/** @}*/
