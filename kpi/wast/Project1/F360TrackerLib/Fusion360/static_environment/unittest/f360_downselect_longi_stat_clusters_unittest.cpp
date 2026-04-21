/** \file
 * This file contains unit tests for content of f360_downselect_longi_stat_clusters.cpp file
 */

#include "f360_downselect_longi_stat_clusters.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

/** \defgroup  f360_downselect_longi_stat_clusters
 *  @{
 */

/** \brief
 * Create 5 clusters with various properties to cover all possible branches. 
 * These clusters are used to test both functions Downselect_Longi_Stat_Clusters() 
 * and Calc_Longi_Stat_Cluster_Score(). These functions only operates on 
 * cluster properties so there's no need to set up the actual objects.
 * Note that 5 clusters are choosen since MAX_NR_OF_LONGI_STAT_CURVES = 4.
 * We want to make sure to cover the case when there are more clusters than
 * the number of curves we allow. 
 */
TEST_GROUP(f360_downselect_longi_stat_clusters)
{
   uint16_t nr_valid_clusters = 5U;
   F360_Longi_Stat_Cluster_T valid_clusters[NR_LONGI_STAT_CLUSTERS];
   uint16_t nr_downselected_clusters;
   F360_Longi_Stat_Cluster_T downselected_clusters[MAX_NR_OF_LONGI_STAT_CURVES];
   F360_Calibrations_T calibs;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS] = {};
   
   /** \setup
    * Create 5 valid clusters with various properties.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      // Cluster adjacent to host
      float32_t lat_mean_cluster_0 = -5.0F;
      objects[0].vcs_position.x = -15.0F;
      objects[0].vcs_position.y = lat_mean_cluster_0;
      objects[0].reference_point = F360_REFERENCE_POINT_CENTER;
      Point center = objects[0].vcs_position;
      objects[0].bbox.Set_Center(center);
      objects[1].vcs_position.x = 15.0F;
      objects[1].vcs_position.y = lat_mean_cluster_0;
      objects[1].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[1].vcs_position;
      objects[1].bbox.Set_Center(center);
      valid_clusters[0].lat_mean = lat_mean_cluster_0;
      valid_clusters[0].first_object = &objects[0];
      valid_clusters[0].last_object = &objects[1];

      // Cluster behind host left side, should get the same score as cluster 2
      float32_t lat_mean_cluster_1 = -2.0F;
      objects[2].vcs_position.x = -35.0F;
      objects[2].vcs_position.y = lat_mean_cluster_1;
      objects[2].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[2].vcs_position;
      objects[2].bbox.Set_Center(center);
      objects[3].vcs_position.x = -20.0F;
      objects[3].vcs_position.y = lat_mean_cluster_1;
      objects[3].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[3].vcs_position;
      objects[3].bbox.Set_Center(center);
      valid_clusters[1].lat_mean = lat_mean_cluster_1;
      valid_clusters[1].first_object = &objects[2];
      valid_clusters[1].last_object = &objects[3];

      // Cluster in front host right side, should get the same score as cluster 1
      float32_t lat_mean_cluster_2 = 2.0F;
      objects[4].vcs_position.x = 20.0F;
      objects[4].vcs_position.y = lat_mean_cluster_2;
      objects[4].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[4].vcs_position;
      objects[4].bbox.Set_Center(center);
      objects[4].bbox.Set_Center(center);
      objects[5].vcs_position.x = 35.0F;
      objects[5].vcs_position.y = lat_mean_cluster_2;
      objects[5].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[5].vcs_position;
      objects[5].bbox.Set_Center(center);
      valid_clusters[2].lat_mean = lat_mean_cluster_2;
      valid_clusters[2].first_object = &objects[4];
      valid_clusters[2].last_object = &objects[5];

      // Cluster with same x_min and x_max. Should in practise never happen but is mimiced for the test
      float32_t lat_mean_cluster_3 = 10.0F;
      objects[6].vcs_position.x = -15.0F;
      objects[6].vcs_position.y = lat_mean_cluster_3;
      objects[6].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[6].vcs_position;
      objects[6].bbox.Set_Center(center);
      objects[7].vcs_position.x = -15.0F;
      objects[7].vcs_position.y = lat_mean_cluster_3;
      objects[7].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[7].vcs_position;
      objects[7].bbox.Set_Center(center);
      valid_clusters[3].lat_mean = lat_mean_cluster_3;
      valid_clusters[3].first_object = &objects[6];
      valid_clusters[3].last_object = &objects[7];

      // Cluster with same lateral position as cluster 0 but shorter
      float32_t lat_mean_cluster_4 = 25.0F;
      objects[8].vcs_position.x = -10.0F;
      objects[8].vcs_position.y = lat_mean_cluster_4;
      objects[8].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[8].vcs_position;
      objects[8].bbox.Set_Center(center);
      objects[9].vcs_position.x = 10.0F;
      objects[9].vcs_position.y = lat_mean_cluster_4;
      objects[9].reference_point = F360_REFERENCE_POINT_CENTER;
      center = objects[9].vcs_position;
      objects[9].bbox.Set_Center(center);
      valid_clusters[4].lat_mean = lat_mean_cluster_4;
      valid_clusters[4].first_object = &objects[8];
      valid_clusters[4].last_object = &objects[9];

   }

};

/** \purpose  
 * Verifies the score calculation for a cluster adjacent to host.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Calc_Longi_Stat_Cluster_Score_Cluster_Adjacent)
{
   /** \action
    * Call function
    */
   float32_t score = Calc_Longi_Stat_Cluster_Score(valid_clusters[0], calibs);

   /** \result
    * Verify that returned score matches expected data
    */
   DOUBLES_EQUAL(8.33333302F, score, F360_EPSILON);
}

/** \purpose
 * Verifies the score calculation for a cluster behind host left side.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Calc_Longi_Stat_Cluster_Score_Cluster_Behind_Left)
{
   /** \action
    * Call function
    */
   float32_t score = Calc_Longi_Stat_Cluster_Score(valid_clusters[1], calibs);

   /** \result
    * Verify that returned score matches expected data
    */
   DOUBLES_EQUAL(26.7664165F, score, F360_EPSILON);
}

/** \purpose
 * Verifies the score calculation for a cluster in front of host right side.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Calc_Longi_Stat_Cluster_Score_Cluster_In_Front_Right)
{
   /** \action
    * Call function
    */
   float32_t score = Calc_Longi_Stat_Cluster_Score(valid_clusters[2], calibs);

   /** \result
    * Verify that returned score matches expected data
    */
   DOUBLES_EQUAL(26.7664165F, score, F360_EPSILON);
}

/** \purpose
 * Verifies the score calculation for a cluster with the same x_min and x_max. 
 * This should in practise never happen and we expect the maximum (worst) score possible.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Calc_Longi_Stat_Cluster_Score_Cluster_Invalid_X_Interval)
{
   /** \action
    * Call function
    */
   float32_t score = Calc_Longi_Stat_Cluster_Score(valid_clusters[3], calibs);

   /** \result
    * Verify that returned score matches expected data
    */
   DOUBLES_EQUAL(INFTY, score, F360_EPSILON);
}

/** \purpose
 * Verifies the score calculation for a cluster with the same lateral position as cluster 0.
 * Since this cluster is shorter we expect a higher (worse) score than for cluster 0.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Calc_Longi_Stat_Cluster_Score_Cluster_Short_Curve)
{
   /** \action
    * Call function
    */
   float32_t score = Calc_Longi_Stat_Cluster_Score(valid_clusters[4], calibs);

   /** \result
    * Verify that returned score matches expected data
    */
   DOUBLES_EQUAL(30.0F, score, F360_EPSILON);
}


/** \purpose
 * Verifies that clusters are downselected correctly when there are more clusters than the number of
 * curves we allow. I.e the curves that are closest to host and long curves should be prioritized.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Downselect_Longi_Stat_Clusters_More_Clusters_Than_Max)
{
   /** \action
    * Call function
    */
   Downselect_Longi_Stat_Clusters(nr_valid_clusters, valid_clusters, calibs, nr_downselected_clusters, downselected_clusters);

   /** \result
    * Verify that correct clusters have been prioritized. I.e, the are stored in prioritized descending order in the 
    * downselected_clusters array. We verify this by comparing the cluster properties of the downselected clusters with
    * the properties of the clusters we expect to be prioritized.
    */
   DOUBLES_EQUAL(valid_clusters[0].lat_mean, downselected_clusters[0].lat_mean, F360_EPSILON);
   CHECK_EQUAL(valid_clusters[0].first_object, downselected_clusters[0].first_object);
   CHECK_EQUAL(valid_clusters[0].last_object, downselected_clusters[0].last_object);

   DOUBLES_EQUAL(valid_clusters[1].lat_mean, downselected_clusters[1].lat_mean, F360_EPSILON);
   CHECK_EQUAL(valid_clusters[1].first_object, downselected_clusters[1].first_object);
   CHECK_EQUAL(valid_clusters[1].last_object, downselected_clusters[1].last_object);

   DOUBLES_EQUAL(valid_clusters[2].lat_mean, downselected_clusters[2].lat_mean, F360_EPSILON);
   CHECK_EQUAL(valid_clusters[2].first_object, downselected_clusters[2].first_object);
   CHECK_EQUAL(valid_clusters[2].last_object, downselected_clusters[2].last_object);

   DOUBLES_EQUAL(valid_clusters[4].lat_mean, downselected_clusters[3].lat_mean, F360_EPSILON);
   CHECK_EQUAL(valid_clusters[4].first_object, downselected_clusters[3].first_object);
   CHECK_EQUAL(valid_clusters[4].last_object, downselected_clusters[3].last_object);
}

/** \purpose
 * Verifies that clusters are downselected correctly when there are less clusters than the number of
 * curves we allow. Thus, all valid clusters should be downselected in this case.
 * \req
 * NA
 */
TEST(f360_downselect_longi_stat_clusters, Downselect_Longi_Stat_Clusters_Less_Clusters_Than_Max)
{
   /** \precond
    * Reduce the number of valid clusters to less than the number of curves we allow.
    */
   nr_valid_clusters = 2U;

   /** \action
    * Call function
    */
   Downselect_Longi_Stat_Clusters(nr_valid_clusters, valid_clusters, calibs, nr_downselected_clusters, downselected_clusters);

   /** \result
    * Verify that all valid clusters have been prioritized.
    */
   DOUBLES_EQUAL(valid_clusters[0].lat_mean, downselected_clusters[0].lat_mean, F360_EPSILON);
   CHECK_EQUAL(valid_clusters[0].first_object, downselected_clusters[0].first_object);
   CHECK_EQUAL(valid_clusters[0].last_object, downselected_clusters[0].last_object);

   DOUBLES_EQUAL(valid_clusters[1].lat_mean, downselected_clusters[1].lat_mean, F360_EPSILON);
   CHECK_EQUAL(valid_clusters[1].first_object, downselected_clusters[1].first_object);
   CHECK_EQUAL(valid_clusters[1].last_object, downselected_clusters[1].last_object);

}
/** @}*/
