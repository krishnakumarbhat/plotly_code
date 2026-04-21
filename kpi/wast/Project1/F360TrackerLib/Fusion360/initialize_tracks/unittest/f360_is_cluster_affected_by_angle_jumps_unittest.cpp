/** \file
   Set of UTs for Is_Cluster_Affected_By_Angle_Jumps function
*/

#include "f360_is_cluster_affected_by_angle_jumps.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"

/** \defgroup  f360_is_cluster_affected_by_angle_jumps
 *  @{
 */

using namespace f360_variant_A;
/** \brief
 *  Test group for testing Is_Cluster_Affected_By_Angle_Jumps function, 
 */

TEST_GROUP(f360_is_cluster_affected_by_angle_jumps)
{
   const static uint32_t num_of_dets = 10;
   const static uint32_t num_of_hist_dets = 4;

   F360_Cluster_T cluster = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   F360_Detection_Hist_T detection_hist = {};
   F360_Calibrations_T calibrations = {};

   /** \setup
   * Setting up cluster, detections, historical detections and calibrations
   */
   TEST_SETUP()
   {
      cluster.id = 5;

      cluster.ndets = num_of_dets;
      for (uint32_t idx = 0; idx < num_of_dets; idx++)
      {
         cluster.detids[idx] = idx + 1;
      }

      cluster.num_old_dets = num_of_hist_dets;
      for (uint32_t idx = 0; idx < num_of_hist_dets; idx++)
      {
         cluster.old_det_idx[idx] = idx;
      }

      calibrations.k_obj_init_min_correct_dets_ratio = 0.35;
      calibrations.k_obj_init_min_number_of_correct_dets = 2;
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
*\purpose  Check output correctness without any angle jump
*\req    NA
*/
TEST(f360_is_cluster_affected_by_angle_jumps, Is_Cluster_Affected_By_Angle_Jumps__Condition_True_no_angle_jumps)
{
   /** \step{1}
    * NA
    **/

    /** \precond
    * NA
    **/
   bool output;

   /** \action
   * NA
   **/
   output = Is_Cluster_Affected_By_Angle_Jumps(cluster, det_props, detection_hist, calibrations);

   /** \result
   * NA
   **/
   CHECK_FALSE(output);
}

/**
*\purpose  Check output correctness with one angle jump
*\req    NA
*/
TEST(f360_is_cluster_affected_by_angle_jumps, Is_Cluster_Affected_By_Angle_Jumps__Condition_False_with_one_angle_jump_from_props)
{
   /** \step{1}
    * NA
    **/

    /** \precond
    * NA
    **/
   bool output;
   det_props[0].f_potential_angle_jump = true;

   /** \action
   * NA
   **/

   output = Is_Cluster_Affected_By_Angle_Jumps(cluster, det_props, detection_hist, calibrations);

   /** \result
   * NA
   **/
   CHECK_FALSE(output);
}


/**
*\purpose  Check output correctness with one angle jump
*\req    NA
*/
TEST(f360_is_cluster_affected_by_angle_jumps, Is_Cluster_Affected_By_Angle_Jumps__Condition_False_with_one_angle_jump_from_hist)
{
   /** \step{1}
    * NA
    **/

    /** \precond
    * NA
    **/
   bool output;
   detection_hist.det_data[0].f_potential_angle_jump = true;

   /** \action
   * NA
   **/

   output = Is_Cluster_Affected_By_Angle_Jumps(cluster, det_props, detection_hist, calibrations);

   /** \result
   * NA
   **/
   CHECK_FALSE(output);
}

/**
*\purpose  Check output correctness with many angle jumps
*\req    NA
*/
TEST(f360_is_cluster_affected_by_angle_jumps, Is_Cluster_Affected_By_Angle_Jumps__Condition_False_with_many_angle_jumps_from_props)
{
   /** \step{1}
    * NA
    **/

    /** \precond
    * NA
    **/
   bool output;
   for (int32_t idx = 0; idx < cluster.ndets - 1; idx++)
   {
      det_props[idx].f_potential_angle_jump = true;
   }

   /** \action
   * NA
   **/

   output = Is_Cluster_Affected_By_Angle_Jumps(cluster, det_props, detection_hist, calibrations);

   /** \result
   * NA
   **/
   CHECK_FALSE(output);
}

/**
*\purpose  Check output correctness with many angle jumps
*\req    NA
*/
TEST(f360_is_cluster_affected_by_angle_jumps, Is_Cluster_Affected_By_Angle_Jumps__Condition_True_with_many_angle_jumps_from_props)
{
   /** \step{1}
    * NA
    **/

    /** \precond
    * NA
    **/
   bool output;
   for (int32_t idx = 0; idx < cluster.ndets; idx++)
   {
      det_props[idx].f_potential_angle_jump = true;
   }

   /** \action
   * NA
   **/

   output = Is_Cluster_Affected_By_Angle_Jumps(cluster, det_props, detection_hist, calibrations);

   /** \result
   * NA
   **/
   CHECK_TRUE(output);
}

/** @}*/

/**
*\purpose  Check output correctness with not enough correct dets
*\req    NA
*/
TEST(f360_is_cluster_affected_by_angle_jumps, Is_Cluster_Affected_By_Angle_Jumps__Condition_True_not_enough_correct_dets)
{
   /** \step{1}
    * NA
    **/

    /** \precond
    * NA
    **/
   bool output;
   cluster.ndets = 3;
   cluster.num_old_dets = 0;
   det_props[0].f_potential_angle_jump = true;
   det_props[1].f_potential_angle_jump = true;

   /** \action
   * NA
   **/

   output = Is_Cluster_Affected_By_Angle_Jumps(cluster, det_props, detection_hist, calibrations);

   /** \result
   * NA
   **/
   CHECK_TRUE(output);
}
/** @}*/
