/** \file
   File contains tests for Fine_Cluster_Gate function
*/

#include "f360_fine_cluster_gate.h"
#include "f360_cluster_grouping_data_generator.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

using namespace f360_variant_A;

/** \defgroup  f360_fine_cluster_gate
 *  @{
 */

/** \brief
*  Test Group for Fine_Cluster_Gate function
**/
TEST_GROUP(f360_fine_cluster_gate)
{
   /** \setup
   * Setting up clusers and intervals
   **/
   F360_Calibrations_T calib = {};
   F360_Cluster_T cluster_1 = {};
   F360_Cluster_T cluster_2 = {};
   float32_t rdot_interval_1 = 0.0;
   float32_t rdot_interval_2 = 0.0;
   float32_t alias_interval_1 = 0.0;
   float32_t alias_interval_2 = 0.0;

   float32_t tolerance = 1e-6;

   bool f_success = false;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calib);
   }
};

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates 1
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__NominalSimplePossitiveCase)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(0.0, 10.0, 10.0, 0.0, cluster_1);
   Fill_Cluster(0.0, 9.0, 10.0, 0.05, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/   
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is true
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates 2
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__NominalPossitiveCase)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(11.0, 11.0, 5.0, 0.0, cluster_1);
   Fill_Cluster(10.5, 10.5, 5.0, 0.05, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is true
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates 3
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__NominalPossitiveCase2)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(11.0, 11.0, -5.0, 0.0, cluster_2);
   Fill_Cluster(10.5, 10.5, -5.0, 0.05, cluster_1);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is true
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates, low time difference and low but significant position difference
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__HighPosDiffWithHighRR)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(10.0, 10.0, 40.0, 0.0, cluster_1);
   Fill_Cluster(8.0, 8.0, 40.0, 0.05, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is true
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates and high position difference and high time difference
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__HighTimeDiff)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(15.0, 15.0, 30.0, 0.0, cluster_1);
   Fill_Cluster(5.0, 5.0, 30.0, 0.5, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is true
   **/
   CHECK_TRUE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates, high position difference and no time difference
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__ClustersTooFarAwayLowRR)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(50.0, -15.0, 5.0, 0.0, cluster_1);
   Fill_Cluster(5.0, 5.0, 5.0, 0.0, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is false
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same high rage rates, low position difference and low time difference
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__CloseClustersHighRR)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(5.2, 5.2, 40.0, 0.0, cluster_1);
   Fill_Cluster(5.0, 5.0, 40.0, 0.05, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, rdot_interval_2, alias_interval_1, alias_interval_2);

   /** \result
   * Checking if gating flag is false
   **/
   CHECK_FALSE(f_success);
}

/**
*\purpose  Checking if gating flag is calculated correctly for two cluster with same rage rates but with interval change, low position difference and low time difference
*\req    NA
*/
TEST(f360_fine_cluster_gate, Fine_Cluster_Gate__NoneZeroInterval)
{
   /** \precond
   * Filling up cluster with position, range rates and time since cluster update
   **/
   Fill_Cluster(6.0, 6.0, 30.0, 0.0, cluster_1);
   Fill_Cluster(5.0, 5.0, 0.0, 0.05, cluster_2);

   /** \action
   * Call Fine_Cluster_Gate function
   **/
   f_success = Fine_Cluster_Gate(calib, cluster_1, cluster_2, rdot_interval_1, 30.0, alias_interval_1, 1.0);

   /** \result
   * Checking if gating flag is true
   **/
   CHECK_TRUE(f_success);
}

/** @}*/
