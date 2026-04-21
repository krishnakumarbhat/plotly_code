/** \file
 * This file contains unit tests for content of f360_detection_association_countermeasures.cpp file
 */

#include "f360_detection_association_countermeasures.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_detection_association_countermeasures
 *  @{
 */

/** \brief
* This test group includes test of the function Detection_Association_Countermeasures() defined in
* f360_detection_association_countermeasures.cpp.
 */
TEST_GROUP(f360_detection_association_countermeasures)
{	
   // Declare common variables used within all tests in this test group.
   F360_Tracker_Info_T tracker_info = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list = {};
   F360_Calibrations_T calibrations = {};
   F360_Host_T host = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Object_Track_T object_tracks[NUMBER_OF_OBJECT_TRACKS] = {};
   F360_Detection_Props_T detection_props[MAX_NUMBER_OF_DETECTIONS] = {};

   /** \setup
    * Set up a default scenario where the preconditions are set to that all countermeasures are called.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibrations);

      tracker_info.num_active_objs = 1;
      tracker_info.active_obj_ids[0] = 1;

      // initialization input data
      object_tracks[0].ndets = 3;
      object_tracks[0].detids[0] = 1;
      object_tracks[0].detids[1] = 2;
      object_tracks[0].detids[2] = 3;

      detection_props[0].object_track_id = 1;
      detection_props[1].object_track_id = 1;
      detection_props[2].object_track_id = 1;

      detection_props[0].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED;
      detection_props[1].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED;
      detection_props[2].any_other_assoc_det_close = F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED;
   }
};

/** \purpose  
 * Test that Detection_Association_Countermeasures() calls both the detection_based as well as the track-based countermeasures.
 * \req
 * NA
 */
TEST(f360_detection_association_countermeasures, DetectionAssociationCountermeasures_AllCM)
{
   /** \precond
    * No changes need to be made to the prepared input in setup.
    */
	
   /** \action
    * Call Detection_Association_Countermeasures().
    */
    Detection_Association_Countermeasures(tracker_info, raw_detection_list, calibrations, host, sensors, object_tracks, detection_props);

   /** \result
    * Check that both if clauses have been entered to detection and track-based countermeasures by:
    */	
   CHECK_EQUAL_TEXT(1, detection_props[0].object_track_id, "Detection with ID 1 was not correctly associated in an expected way.")
   CHECK_EQUAL_TEXT(1, detection_props[1].object_track_id, "Detection with ID 2 was not correctly associated in an expected way.")
   CHECK_EQUAL_TEXT(1, detection_props[2].object_track_id, "Detection with ID 3 was not correctly associated in an expected way.")
}

/** \purpose
* Test that Detection_Association_Countermeasures() calls none of the detection_based as well as the track-based countermeasures 
* when obj idx is, by mistake somewhere else, less than 0..
* \req
* NA
*/
TEST(f360_detection_association_countermeasures, DetectionAssociationCountermeasures_NoCM)
{
   /** \precond
   * Set first element of active objext IDs to 0.
   */
   tracker_info.active_obj_ids[0] = 0;

   /** \action
   * Call Detection_Association_Countermeasures().
   */
   Detection_Association_Countermeasures(tracker_info, raw_detection_list, calibrations, host, sensors, object_tracks, detection_props);

   /** \result
   * Check that the value of the detections' fields object_track_id and any_other_assoc_det_close have not been modified.
   */
   CHECK_EQUAL_TEXT(1, detection_props[0].object_track_id, "Detection with ID 1 was not correctly associated in an expected way.")
   CHECK_EQUAL_TEXT(1, detection_props[1].object_track_id, "Detection with ID 2 was not correctly associated in an expected way.")
   CHECK_EQUAL_TEXT(1, detection_props[2].object_track_id, "Detection with ID 3 was not correctly associated in an expected way.")

   CHECK_EQUAL_TEXT(F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED, detection_props[0].any_other_assoc_det_close, "Detection with ID 1 did not have the expected neighbor marking.")
   CHECK_EQUAL_TEXT(F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED, detection_props[1].any_other_assoc_det_close, "Detection with ID 2 did not have the expected neighbor marking.")
   CHECK_EQUAL_TEXT(F360_ANY_OTHER_ASSOC_DET_CLOSE_UNDETERMINED, detection_props[2].any_other_assoc_det_close, "Detection with ID 3 did not have the expected neighbor marking.")
}

/** \purpose
* Test that function that conditionally deassociates detections from objects is called in Detection_Association_Countermeasures().
* \req
* NA
*/
TEST(f360_detection_association_countermeasures, DetectionAssociationCountermeasures_Cond_Deassoc)
{
   /** \precond
   * For first object in object list set:
   * - position to (0,7)
   * - wid1 and wid2 to 0.5m and width to 1m
   * - speed to 5m/s
   * - visible edge to left edge
   * - pointing to 0 
   * Set position of first detection in detection list to (0,8)
   * Set range rate compensated for the first detection to 0
   */
   object_tracks[0].vcs_position.x = 0.0F;
   object_tracks[0].vcs_position.y = 7.0F;
   object_tracks[0].Set_Bbox_Orientation(Angle{ 1.0F });
   object_tracks[0].Update_Bbox_Size(1.0F, 1.0F);
   object_tracks[0].speed = 5.0F;
   object_tracks[0].reference_point = F360_REFERENCE_POINT_LEFT;

   detection_props[0].vcs_position.x = 0.0F;
   detection_props[0].vcs_position.y = 8.0F;
   detection_props[0].range_rate_compensated = 0.0F;


   /** \action
   * Call Detection_Association_Countermeasures().
   */
   Detection_Association_Countermeasures(tracker_info, raw_detection_list, calibrations, host, sensors, object_tracks, detection_props);

   /** \result
   * Check the first detection was deassociated from the object
   */
   CHECK_EQUAL_TEXT(2, object_tracks[0].ndets, "The detection was not deassociated from the object as expected.");
   CHECK_EQUAL_TEXT(0, detection_props[0].object_track_id, "Detection with ID 1 was not deassociated as expected.");
}
/** @}*/