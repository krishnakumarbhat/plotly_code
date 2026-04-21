/** \file
 * This file contains unit tests for content of f360_calc_object_init_ep.cpp file
 */

#include "f360_calc_object_init_ep.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

/** \defgroup  f360_calc_object_init_ep
 *  @{
 */

/** \brief
 * Check calculation correctness of inital value of existenc probability for an object
 */
TEST_GROUP(f360_calc_object_init_ep)
{	
   F360_Calibrations_T cals;

   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(cals);
   }
};

/** \purpose  
 * Chaeck simple calculations
 * \req
 * NA
 */
TEST(f360_calc_object_init_ep, Calc_Object_Init_Existence_Prob__Simple_calculations)
{
   /** \precond
    * Init
    */
   const float32_t cluster_ep = 0.8F;
   const float32_t cluster_VCS_velocity_plausability = 0.7F;
	
   /** \action
    * Call tested function
    */
   const float32_t result = Calc_Object_Init_Existence_Prob(cluster_ep, cluster_VCS_velocity_plausability, cals);

   /** \result
    * Check if result is as expected
    */
   DOUBLES_EQUAL(0.728F, result, F360_EPSILON);
}

/** \purpose
 * Check if the value is saturated correctly (max)
 * \req
 * NA
 */
TEST(f360_calc_object_init_ep, Calc_Object_Init_Existence_Prob__Saturate_max)
{
   /** \precond
    * Init
    */
   const float32_t cluster_ep = 1.0F;
   const float32_t cluster_VCS_velocity_plausability = 1.0F;

   /** \action
    * Call tested function
    */
   const float32_t result = Calc_Object_Init_Existence_Prob(cluster_ep, cluster_VCS_velocity_plausability, cals);

   /** \result
    * Check if result is as expected
    */
   DOUBLES_EQUAL(1.0F, result, F360_EPSILON);
}
/** @}*/
