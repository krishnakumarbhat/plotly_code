/** \file
 * This file contains unit tests for content of f360_detect_veh_trk_close_to_stat_host.cpp file
 */

#include "f360_detect_veh_trk_close_to_stat_host.h"
#include <CppUTest/TestHarness.h>

//#include "headerfile_needed.h"

// Unit testing guidelines: http://confluenceprod1.delphiauto.net:8090/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_detect_veh_trk_close_to_stat_host
 *  @{
 */

/** \brief
 * Test group containing needed data to verify that flagging of objects close to host
 * works as intended. Data is set up so that object is expected to be flagged and variables
 * are then tweaked for each test case to get full branch coverage
 */
TEST_GROUP(f360_detect_veh_trk_close_to_stat_host)
{	
   F360_Host_T host;
   F360_Globals_T global;
   F360_Calibrations_T calib;
   F360_Object_Track_T object_track;
   
   /** \setup
    * Setup data so that object is expected to be flagged as close to stationary host
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);

      global.obj_mov_stat_spd_thresh = 1.0F;

      host.vcs_speed = 0.0F;

      object_track.vcs_position.x = 0.0F;
      object_track.vcs_position.y = 0.0F;
      object_track.f_vehicular_trk = true;
      object_track.status = F360_OBJECT_STATUS_UPDATED;
      object_track.time_since_cluster_created = 1.0F;
   }
};

/** \purpose  
 * Verify that object is flagged as close to stationary host
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Flagged)
{
	
   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to be flagged
    */
   CHECK_TRUE(object_track.f_veh_trk_near_stat_host);
}

/** \purpose
 * Verify that object is not flagged as close to stationary host due to
 * host speed too high
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Not_Flagged_Host_Speed_Too_High)
{
   /** \precond
   * Set host speed above global threshold
   */
   host.vcs_speed = global.obj_mov_stat_spd_thresh + 1.0F;

   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to not be flagged
    */
   CHECK_FALSE(object_track.f_veh_trk_near_stat_host);
}

/** \purpose
 * Verify that object is not flagged as close to stationary host due to 
 * longitudinal position too far away
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Not_Flagged_Long_Pos_Too_Far)
{
   /** \precond
   * Set object longitudinal position far away
   */
   object_track.vcs_position.x = 100.0F;

   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to not be flagged
    */
   CHECK_FALSE(object_track.f_veh_trk_near_stat_host);
}

/** \purpose
 * Verify that object is not flagged as close to stationary host due to
 * lateral position too far away
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Not_Flagged_Lat_Pos_Too_Far)
{
   /** \precond
   * Set object lateral position far away
   */
   object_track.vcs_position.y = 100.0F;

   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to not be flagged
    */
   CHECK_FALSE(object_track.f_veh_trk_near_stat_host);
}

/** \purpose
 * Verify that object is not flagged as close to stationary host due to
 * object not flagged as a vehicular track
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Not_Flagged_Object_Not_Vehicular)
{
   /** \precond
   * Set object as not a vehicular track
   */
   object_track.f_vehicular_trk = false;

   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to not be flagged
    */
   CHECK_FALSE(object_track.f_veh_trk_near_stat_host);
}

/** \purpose
 * Verify that object is not flagged as close to stationary host due to
 * object status is new
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Not_Flagged_Object_Status_New)
{
   /** \precond
   * Set object status to new
   */
   object_track.status = F360_OBJECT_STATUS_NEW_UPDATED;

   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to not be flagged
    */
   CHECK_FALSE(object_track.f_veh_trk_near_stat_host);
}

/** \purpose
 * Verify that object is not flagged as close to stationary host due to
 * objects time since cluster created is too small
 * \req
 * NA
 */
TEST(f360_detect_veh_trk_close_to_stat_host, Det_VehTrk_Close_To_Stat_Host_Object_Is_Not_Flagged_Object_Cluster_Too_Young)
{
   /** \precond
   * Set objects time since cluster created below threshold
   */
   object_track.time_since_cluster_created = calib.min_object_age_for_object_close_to_stat_host - 0.5F;

   /** \action
    * Call function
    */
   Det_VehTrk_Close_To_Stat_Host(host, global, calib, object_track);

   /** \result
    * Expect object to not be flagged
    */
   CHECK_FALSE(object_track.f_veh_trk_near_stat_host);
}
/** @}*/
