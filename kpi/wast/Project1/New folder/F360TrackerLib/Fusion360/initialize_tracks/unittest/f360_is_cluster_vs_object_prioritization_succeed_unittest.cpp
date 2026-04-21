/** \file
   File with unit tests for Is_Cluster_Vs_Object_Prioritization_Succeed function
*/

#include "f360_is_cluster_vs_object_prioritization_succeed.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"

/** \defgroup  f360_Is_Cluster_Vs_Object_Prioritization_Succeed
 *  @{
 */
using namespace f360_variant_A;
/** \brief
 *  Test group for testing Is_Cluster_Vs_Object_Prioritization_Succeed function
 */

TEST_GROUP(f360_Is_Cluster_Vs_Object_Prioritization_Succeed)
{
   /** \setup
   * Setting up arguments for Is_Cluster_Vs_Object_Prioritization_Succeed
   */
   F360_Tracker_Info_T tracker_info;
   F360_Cluster_T cluster;
   F360_Object_Track_T lowest_prior_object;

   bool result;

   TEST_SETUP()
   {
      tracker_info = {};
      tracker_info.p_lowest_priority_track = {};
      lowest_prior_object = {};
      cluster = {};

      tracker_info.p_lowest_priority_track = &lowest_prior_object;

      Set_Tracker_Variant(tracker_info.variant);
      result = NULL;
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
*\purpose  Check if function providing correct flag according to lowest tracks priority
*\req    NA
*/
TEST(f360_Is_Cluster_Vs_Object_Prioritization_Succeed, test_cluster_with_higher_prior_than_min)
{
   /** \precond
    * Setting number of active objects, lowest trakc priority and cluster's priority
    */
   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS + 1;
   lowest_prior_object.priority = 0.5F;
   cluster.priority = 1.0F;

   /** \action
    * Calling Is_Cluster_Vs_Object_Prioritization_Succeed
    */
   result = Is_Cluster_Vs_Object_Prioritization_Succeed(tracker_info, cluster);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_TRUE(result);
}

/**
*\purpose  Check if function providing correct flag according to lowest tracks priority
*\req    NA
*/
TEST(f360_Is_Cluster_Vs_Object_Prioritization_Succeed, test_cluster_with_lower_prior_than_min)
{
   /** \precond
    * Setting number of active objects, lowest trakc priority and cluster's priority
    */
   tracker_info.num_active_objs = NUMBER_OF_OBJECT_TRACKS + 1;
   lowest_prior_object.priority = 0.5F;
   cluster.priority = 0.4F;

   /** \action
    * Calling Is_Cluster_Vs_Object_Prioritization_Succeed
    */
   result = Is_Cluster_Vs_Object_Prioritization_Succeed(tracker_info, cluster);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_FALSE(result);
}

/**
*\purpose  Check if function providing correct flag according to lowest tracks priority
*\req    NA
*/
TEST(f360_Is_Cluster_Vs_Object_Prioritization_Succeed, test_num_of_active_objects_lower_than_max)
{
   /** \precond
    * Setting number of active objects, lowest trakc priority and cluster's priority
    */
   tracker_info.num_active_objs = 1;
   lowest_prior_object.priority = 0.5F;
   cluster.priority = 0.4F;

   /** \action
    * Calling Is_Cluster_Vs_Object_Prioritization_Succeed
    */
   result = Is_Cluster_Vs_Object_Prioritization_Succeed(tracker_info, cluster);

   /** \result
    * Checking if results matches the expectations
    */
   CHECK_TRUE(result);
}
/** @}*/
