/** \file
 * This file contains unit tests for content of f360_static_env_helpers.cpp file
 */

#include "f360_static_env_helpers.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

/** \defgroup  f360_static_env_helpers_Reset_Single_Static_Env_Poly
 *  @{
 */

/** \brief
 * Test group related to test of function Reset_Single_Static_Env_Poly()
 */
TEST_GROUP(f360_static_env_helpers_Reset_Single_Static_Env_Poly)
{
   
};

/** \purpose  
 * Purpose is to verify that static polynomial is reset as expected
 * \req
 * NA
 */
TEST(f360_static_env_helpers_Reset_Single_Static_Env_Poly, Reset_Single_Static_Env_Poly__Verify_Reset)
{
   /** \precond
    * Fill a static polynomial with arbitrary data
    */
   Static_Env_Poly_T static_env_poly;
   static_env_poly.age = 100U;
   static_env_poly.confidence = 1.0F;
   static_env_poly.lower_limit = 1.0F;
   static_env_poly.upper_limit = 1.0F;
   static_env_poly.p0 = 1.0F;
   static_env_poly.p1 = 1.0F;
   static_env_poly.p2 = 1.0F;
   static_env_poly.status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   static_env_poly.poly_type = F360_STATIC_ENV_POLY_TYPE_LSC;

   /** \action
    * Call Reset_Single_Static_Env_Poly()
    */
   Reset_Single_Static_Env_Poly(static_env_poly);

   /** \result
    * Verify that data have been reset
    */
   CHECK_EQUAL(0U, static_env_poly.age);
   CHECK_EQUAL(0.0F, static_env_poly.confidence);
   CHECK_EQUAL(0.0F, static_env_poly.lower_limit);
   CHECK_EQUAL(0.0F, static_env_poly.upper_limit);
   CHECK_EQUAL(0.0F, static_env_poly.p0);
   CHECK_EQUAL(0.0F, static_env_poly.p1);
   CHECK_EQUAL(0.0F, static_env_poly.p2);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, static_env_poly.status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_INVALID, static_env_poly.poly_type);
}
/** @}*/

/** \defgroup  f360_static_env_helpers_Map_Single_LSC_To_Static_Env_Poly
 *  @{
 */

 /** \brief
  * Test group related to test of function Map_Single_LSC_To_Static_Env_Poly()
  */
TEST_GROUP(f360_static_env_helpers_Map_Single_LSC_To_Static_Env_Poly)
{
   F360_Longi_Stat_Curve_T lsc = {};

   /** \setup
    * Initialize a valid LSC with arbitrary data
    */
   TEST_SETUP()
   {
      lsc.f_valid = true;
      lsc.a = 1.0F;
      lsc.b = 2.0F;
      lsc.c = -3.0F;
      lsc.x_min = -10.0F;
      lsc.x_max = 10.0F;
      lsc.mean_lat_pos = -3.0F;
   }
};

/** \purpose
 * Purpose is to verify that static polynomial is reset as expected when 
 * the LSC we try to map is invalid
 * \req
 * NA
 */
TEST(f360_static_env_helpers_Map_Single_LSC_To_Static_Env_Poly, Map_Single_LSC_To_Static_Env_Poly__Invalid_LSC)
{
   /** \precond
    * Fill a static polynomial with arbitrary data
    * Set LSC as invalid
    */
   Static_Env_Poly_T static_env_poly = {};
   static_env_poly.age = 100U;
   static_env_poly.confidence = 1.0F;
   static_env_poly.lower_limit = 1.0F;
   static_env_poly.upper_limit = 1.0F;
   static_env_poly.p0 = 1.0F;
   static_env_poly.p1 = 1.0F;
   static_env_poly.p2 = 1.0F;
   static_env_poly.status = F360_STATIC_ENV_POLY_STATUS_UPDATED;
   static_env_poly.poly_type = F360_STATIC_ENV_POLY_TYPE_LSC;

   lsc.f_valid = false;

   /** \action
    * Call Map_Single_LSC_To_Static_Env_Poly()
    */
   Map_Single_LSC_To_Static_Env_Poly(lsc, static_env_poly);

   /** \result
    * Verify that data have been reset
    */
   CHECK_EQUAL(0U, static_env_poly.age);
   CHECK_EQUAL(0.0F, static_env_poly.confidence);
   CHECK_EQUAL(0.0F, static_env_poly.lower_limit);
   CHECK_EQUAL(0.0F, static_env_poly.upper_limit);
   CHECK_EQUAL(0.0F, static_env_poly.p0);
   CHECK_EQUAL(0.0F, static_env_poly.p1);
   CHECK_EQUAL(0.0F, static_env_poly.p2);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_INVALID, static_env_poly.status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_INVALID, static_env_poly.poly_type);
}

/** \purpose
 * Purpose is to verify that static polynomial is mapped correctly when
 * the LSC we try to map is valid
 * \req
 * NA
 */
TEST(f360_static_env_helpers_Map_Single_LSC_To_Static_Env_Poly, Map_Single_LSC_To_Static_Env_Poly__Valid_LSC)
{
   /** \precond
    * Initialize a static environment polynomial
    * LSC contains valid information set in test group
    */
   Static_Env_Poly_T static_env_poly = {};

   /** \action
    * Call Map_Single_LSC_To_Static_Env_Poly()
    */
   Map_Single_LSC_To_Static_Env_Poly(lsc, static_env_poly);

   /** \result
    * Verify that data have been updated with data from LSC
    */
   CHECK_EQUAL(1U, static_env_poly.age);
   CHECK_EQUAL(1.0F, static_env_poly.confidence);
   CHECK_EQUAL(lsc.x_min, static_env_poly.lower_limit);
   CHECK_EQUAL(lsc.x_max, static_env_poly.upper_limit);
   CHECK_EQUAL(lsc.c, static_env_poly.p0);
   CHECK_EQUAL(lsc.b, static_env_poly.p1);
   CHECK_EQUAL(lsc.a, static_env_poly.p2);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_STATUS_UPDATED, static_env_poly.status);
   CHECK_EQUAL(F360_STATIC_ENV_POLY_TYPE_LSC, static_env_poly.poly_type);
}
/** @}*/
