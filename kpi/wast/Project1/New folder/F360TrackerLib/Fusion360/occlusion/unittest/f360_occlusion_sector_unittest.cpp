/** \file
 * This file contains unit tests for content of f360_occlusion_sector.cpp file
 */

#include "f360_occlusion_sector.h"
#include <CppUTest/TestHarness.h>

using namespace f360_variant_A;

/** \defgroup  f360_occlusion_sector
 *  @{
 */

 /** \brief
  * Test group of Occlusion_Sector_T() class. Tests verify whether intersection line is properly calculated and lower range is selected.
  */
TEST_GROUP(f360_occlusion_sector)
{
   Occlusion_Sector_T sector;

   /** \setup
    * Set up initial range of sector
    */
   TEST_SETUP()
   {
      sector.Set_Range(100.0F);
   }
};

/** \purpose
 * Purpose of this test is to verify whether set and get functions work correctly
 * \req
 * NA.
 */
TEST(f360_occlusion_sector, f360_occlusion_sector_Set_And_Get_Work_Correctly)
{
   /** \precond
    * Save current sector range
    */
   const float32_t current_range = sector.Get_Range();

   /** \action
    * Update sector range
    * Save next sector range
    */
   sector.Set_Range(50.0F);
   const float32_t next_range = sector.Get_Range();

   /** \result
    * Check whether current_range is equal to 100.0F
    * Check whether next_range is equal to 50.0F
    */
   DOUBLES_EQUAL(100.0F, current_range, F360_EPSILON);
   DOUBLES_EQUAL(50.0F, next_range, F360_EPSILON);
}

/** \purpose
 * Purpose of this test is to verify whether intersection of two lines is properly calculated and range
 * is updated when is lower than current sectorrange.
 * \req
 * NA.
 */
TEST(f360_occlusion_sector, f360_occlusion_sector_Intersection_Properly_Determined_And_Range_Updated_When_Lower)
{
   /** \precond
    * Set up parmaters of two lines
    */
   float32_t A[2] = { 1.0F, -1.0F };
   float32_t B[2] = { 1.0F, 1.0F };

   float32_t C[2] = { 2.0F, -1.0F };
   float32_t D[2] = { 0.0F, 1.0F };

   /** \action
    * Call Update_Range_With_Intersection_Of_Two_Lines() on sector object.
    */
   sector.Update_Range_With_Intersection_Of_Two_Lines(A, B, C, D);

   /** \result
    * Check whether updated sector range is equal to 1.0F
    */
   const float32_t new_sector_range = sector.Get_Range();
   DOUBLES_EQUAL(1.0F, new_sector_range, F360_EPSILON);

}

/** \purpose
 * Purpose of this test is to verify whether intersection of two lines is properly calculated and range
 * is not updated when new range is higher than current sector range
 * \req
 * NA.
 */
TEST(f360_occlusion_sector, f360_occlusion_sector_Intersection_Properly_Determined_And_Range_Not_Updated_When_Higher)
{
   /** \precond
    * Set up parmaters of two lines
    */
   float32_t A[2] = { 120.0F, -1.0F };
   float32_t B[2] = { 120.0F, 1.0F };

   float32_t C[2] = { 122.0F, -1.0F };
   float32_t D[2] = { 118.0F, 1.0F };

   /** \action
    * Call Update_Range_With_Intersection_Of_Two_Lines() on sector object.
    */
   sector.Update_Range_With_Intersection_Of_Two_Lines(A, B, C, D);

   /** \result
    * Check whether updated sector range is equal to 100.0F
    */
   const float32_t new_sector_range = sector.Get_Range();
   DOUBLES_EQUAL(100.0F, new_sector_range, F360_EPSILON);

}

/** \purpose
 * Purpose of this test is to verify whether range is not updated when lines do not intersect.
 * \req
 * NA.
 */
TEST(f360_occlusion_sector, f360_occlusion_sector__Range_Not_Updated_When_Lines_Do_Not_Intersect)
{
   /** \precond
    * Set up parmaters of two lines so they do not intersect
    */
   float32_t A[2] = { 1.0F, -1.0F };
   float32_t B[2] = { 1.0F, 1.0F };

   float32_t C[2] = { 20.0F, -1.0F };
   float32_t D[2] = { 22.0F, 1.0F };

   /** \action
    * Call Update_Range_With_Intersection_Of_Two_Lines() on sector object.
    */
   sector.Update_Range_With_Intersection_Of_Two_Lines(A, B, C, D);

   /** \result
    * Check whether updated sector range is equal to 100.0F
    */
   const float32_t new_sector_range = sector.Get_Range();
   DOUBLES_EQUAL(100.0F, new_sector_range, F360_EPSILON);

}
/** @}*/
