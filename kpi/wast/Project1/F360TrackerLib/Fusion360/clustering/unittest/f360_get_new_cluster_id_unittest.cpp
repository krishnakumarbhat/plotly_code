/** \file
   This file contains unit tests for content of f360_get_new_cluster_id.cpp file
*/

#include "f360_get_new_cluster_id.h"
#include <CppUTest/TestHarness.h>
#include "f360_set_variant.h"

// Unit testing guidelines: http://confluenceprod1.delphiauto.net:8090/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;


/** \defgroup  f360_get_new_cluster_id
 *  @{
 */

/** \brief
 *  test group contains test cases for Get_New_Cluster_ID() function.
 */

TEST_GROUP(f360_get_new_cluster_id)
{
   F360_Tracker_Info_T tracker_info = {};

   /** \setup
   * Reset cluster information
   */
   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);
      
      tracker_info.num_active_clusters = 0;
      for (int i = 0; i < NUMBER_OF_CLUSTERS; i++)
      {
         tracker_info.inactive_cluster_ids[i] = i + 1;
         tracker_info.active_cluster_ids[i] = 0;
      }
   }
};

/**
*\purpose  Check if ID will be get if there is empty array of active_cluster_ids
*\req    NA
*/
TEST(f360_get_new_cluster_id, get_id_from_emtpy_array_POS)
{
   /** \precond
    * None
    */
   int16_t new_id;

   /** \action
    *  Call Get_New_Cluster_ID function
    */
   Get_New_Cluster_ID(new_id, tracker_info);

   /** \result
    * Expected valid output
    */
   CHECK_FALSE(new_id == 0);
   CHECK_TRUE(tracker_info.num_active_clusters == 1);
}

/**
*\purpose  Check if ID will be get if there is array of active_cluster_ids is nearly full
*\req    NA
*/
TEST(f360_get_new_cluster_id, get_id_from_nearly_full_array_POS)
{
   /** \precond
    * Get ids for NUMBER_OF_CLUSTERS-1 clusters
    */
   int16_t new_id;

   for (unsigned int i = 0; i < (NUMBER_OF_CLUSTERS-1U); i++)
   {
      Get_New_Cluster_ID(new_id, tracker_info);
   }

   /** \action
    *  Call Get_New_Cluster_ID function
    */
   Get_New_Cluster_ID(new_id, tracker_info);

   /** \result
    * Expected valid output
    */
   CHECK_FALSE(new_id == 0);
   CHECK_TRUE(tracker_info.num_active_clusters == 2000);
}

/**
*\purpose  Check if invalid will be get if there is full array of active_cluster_ids
*\req    NA
*/
TEST(f360_get_new_cluster_id, get_id_from_full_array_NEG)
{
   /** \precond
    * Get ids for NUMBER_OF_CLUSTERS clusters
    */
   int16_t new_id;

   for (int i = 0; i < NUMBER_OF_CLUSTERS; i++)
   {
      Get_New_Cluster_ID(new_id, tracker_info);
   }

   /** \action
    *  Call Get_New_Cluster_ID function
    */
   Get_New_Cluster_ID(new_id, tracker_info);

   /** \result
    * Expected invalid output
    */
   CHECK_TRUE(new_id == 0);
   CHECK_TRUE(tracker_info.num_active_clusters == NUMBER_OF_CLUSTERS);
}

/**
*\purpose  Check if invalid will be get if there is full array of active_cluster_ids,
*          additionally try to get id couple times more
*\req    NA
*/
TEST(f360_get_new_cluster_id, get_id_from_full_array_try_10_times_NEG)
{
   /** \precond
    *  Get ids for NUMBER_OF_CLUSTERS-1 clusters
    */
   int16_t new_id;

   for (int i = 0; i < NUMBER_OF_CLUSTERS; i++)
   {
      Get_New_Cluster_ID(new_id, tracker_info);
   }

   /** \action
    *  Try to get new ID 10 times once array is already full
    */
   for (int i = 0; i < 10; i++)
   {
      Get_New_Cluster_ID(new_id, tracker_info);

      /** \result
       * Expected invalid output
       */
      CHECK_TRUE(new_id == 0);
      CHECK_TRUE(tracker_info.num_active_clusters == NUMBER_OF_CLUSTERS);
   }
}

/** @}*/

