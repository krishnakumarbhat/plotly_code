/** \file
 * This file contains unit tests for content of f360_update_longi_stat_curves.cpp file
 */

#include "f360_update_longi_stat_curves.h"
#include "f360_set_variant.h"
#include <CppUTest/TestHarness.h>

#include "f360_lsc_data_generator.h"

// Unit testing guidelines: http://confluenceprod1.delphiauto.net:8090/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_update_longi_stat_curves_Arrange_First_Iteration
 *  @{
 */

/** \brief
 * This test group defines a set of objects to be used to test function Arrange_First_Iteration().
 */
TEST_GROUP(f360_update_longi_stat_curves_Arrange_First_Iteration)
{
   // Declare common variables used within all tests in this test group.
   F360_Tracker_Info_T tracker_info;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS];
   F360_Calibrations_T calibs;
   uint16_t nr_next_ids_of_interest;
   uint16_t next_ids_of_interest[NUMBER_OF_OBJECT_TRACKS];
   F360_Host_T host;

   bool f_is_data_remaining;

   F360_LSC_Object_Group_Settings_T group_A;
   
   /** \setup
    * Initialize default calibrations and create a set of objects.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      Set_Tracker_Variant(tracker_info.variant);

      Initialize_Tracker_Info(tracker_info);
      host.curvature_rear = 0.0F;
      group_A = Add_LSC_Group_A(tracker_info, objects);
   }

};

/** \purpose  
 * Purpose is to verify that all objects have been correctly selected as they are within
 * the valid interval in longitudinal direction for relevant objects. We also verify
 * that the function returns true which indicates that there are enough relevant object to
 * run LSC algorithm.
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Arrange_First_Iteration, Arrange_First_Iteration_All_Objects_Relevant)
{
   /** \precond
    * Modify interval in calibration to guarantee all objects are within relevant interval
    */
   uint16_t first_idx = group_A.ids[0] - 1U;
   calibs.k_lsc_min_long_pos = objects[first_idx].vcs_position.x - 1.0F;
   uint16_t last_index = group_A.ids[group_A.nr_objects - 1U] - 1U;
   calibs.k_lsc_max_long_pos = objects[last_index].vcs_position.x + 1.0F;

   /** \action
    * Call function
    */
   f_is_data_remaining = Arrange_First_Iteration(tracker_info, calibs, host, nr_next_ids_of_interest, next_ids_of_interest);

   /** \result
    * Verify all objects have been selected and that function has returned true
    */
   CHECK_TRUE(f_is_data_remaining);
   CHECK_EQUAL(group_A.nr_objects, nr_next_ids_of_interest);
   for (uint16_t i = 0U; i < nr_next_ids_of_interest; i++)
   {
      CHECK_EQUAL(group_A.ids[i], next_ids_of_interest[i]);
   }
}

/** \purpose
 * Purpose is to verify that all objects except for the first and the last one have been correctly selected as they are within
 * the valid interval in longitudinal direction for relevant objects. The first one should not be selected as it is CTCA and the last one 
 * should not be selected as it is moveable. 
 * We also verify that the function returns true which indicates that there are enough relevant object to run LSC algorithm.
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Arrange_First_Iteration, Arrange_First_Iteration_CTCA_and_Moveable_Objects_Excluded)
{
   /** \precond
    * Modify interval in calibration to guarantee all objects are within relevant interval
    * Set filter type of first object to CTCA
    * Set last object to moveable
    */
   uint16_t first_idx = group_A.ids[0] - 1U;
   calibs.k_lsc_min_long_pos = objects[first_idx].vcs_position.x - 1.0F;
   uint16_t last_index = group_A.ids[group_A.nr_objects - 1U] - 1U;
   calibs.k_lsc_max_long_pos = objects[last_index].vcs_position.x + 1.0F;
   
   objects[first_idx].trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;
   objects[last_index].f_moveable = true;

   /** \action
    * Call function
    */
   f_is_data_remaining = Arrange_First_Iteration(tracker_info, calibs, host, nr_next_ids_of_interest, next_ids_of_interest);

   /** \result
    * Verify all objects have been selected and that function has returned true
    */
   CHECK_TRUE(f_is_data_remaining);
   CHECK_EQUAL(group_A.nr_objects - 2U, nr_next_ids_of_interest);
   for (uint16_t i = 1U; i < nr_next_ids_of_interest - 1; i++)
   {
      CHECK_EQUAL(group_A.ids[i], next_ids_of_interest[i-1U]);
   }
}

/** \purpose
 * Purpose is to verify that all objects except the first and last have been correctly selected as they are not
 * within the valid interval in longitudinal direction for relevant objects. We also verify that the function 
 * returns true which indicates that there are enough relevant object to run LSC algorithm.
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Arrange_First_Iteration, Arrange_First_Iteration_First_And_Last_Object_Excluded)
{
   /** \precond
    * Modify interval in calibration so that first and last object are outside valid interval
    */
   uint16_t first_idx = group_A.ids[0] - 1U;
   calibs.k_lsc_min_long_pos = objects[first_idx].vcs_position.x + 1.0F;
   uint16_t last_index = group_A.ids[group_A.nr_objects - 1U] - 1U;
   calibs.k_lsc_max_long_pos = objects[last_index].vcs_position.x - 1.0F;

   /** \action
    * Call function
    */
   f_is_data_remaining = Arrange_First_Iteration(tracker_info, calibs, host, nr_next_ids_of_interest, next_ids_of_interest);

   /** \result
    * Verify that correct objects have been selected and that function has returned true
    */
   CHECK_TRUE(f_is_data_remaining);
   CHECK_EQUAL(group_A.nr_objects - 2U, nr_next_ids_of_interest);
   for (uint16_t i = 0U; i < nr_next_ids_of_interest; i++)
   {
      CHECK_EQUAL(group_A.ids[i + 1U], next_ids_of_interest[i]);
   }
}

/** \purpose
 * Purpose is to verify that only one object is selected when only one object is within valid interval.
 * One object is not enough to fit a second order polynomial so we also expect that the function returns false
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Arrange_First_Iteration, Arrange_First_Iteration_One_Relevant_Object)
{
   /** \precond
    * Modify interval in calibration so that only one object is in valid interval
    */
   uint16_t expected_id = 10U;

   uint16_t object_idx = group_A.ids[expected_id - 1U] - 1U;
   calibs.k_lsc_min_long_pos = objects[object_idx].vcs_position.x - 1.0F;
   calibs.k_lsc_max_long_pos = objects[object_idx].vcs_position.x + 1.0F;

   /** \action
    * Call function
    */
   f_is_data_remaining = Arrange_First_Iteration(tracker_info, calibs, host, nr_next_ids_of_interest, next_ids_of_interest);

   /** \result
    * Verify that correct object have been selected and that function has returned false
    */
   CHECK_FALSE(f_is_data_remaining);
   CHECK_EQUAL(1U, nr_next_ids_of_interest);
   CHECK_EQUAL(expected_id, next_ids_of_interest[0]);
}

/** @}*/

/** \defgroup  f360_update_longi_stat_curves_Sanity_Check_And_Populate_LSC_Output
 *  @{
 */

 /** \brief
  * This test group defines a set of new longi stat curves to test functionality
  * of function Sanity_Check_And_Populate_LSC_Output()
  */
TEST_GROUP(f360_update_longi_stat_curves_Sanity_Check_And_Populate_LSC_Output)
{
   F360_Calibrations_T calibs;
   uint16_t nr_downselected_clusters;
   F360_Longi_Stat_Curve_T new_longi_stat_curves[MAX_NR_OF_LONGI_STAT_CURVES];
   F360_Longi_Stat_Curve_T longi_stat_curves[MAX_NR_OF_LONGI_STAT_CURVES];

   /** \setup
    * Initialize default calibrations and create a set of new longi stat curves.
    * The "a" coefficient of the second curve is too large for it to be valid
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      nr_downselected_clusters = 3U;

      new_longi_stat_curves[0].f_valid = true;
      new_longi_stat_curves[0].x_min = -25.0F;
      new_longi_stat_curves[0].x_max = -15.0F;
      new_longi_stat_curves[0].a = 0.5F * calibs.k_lsc_max_a_coeff;
      new_longi_stat_curves[0].b = 0.1F;
      new_longi_stat_curves[0].c = 1.0F;
      new_longi_stat_curves[0].mean_lat_pos = -25.0F;

      new_longi_stat_curves[1].f_valid = true;
      new_longi_stat_curves[1].x_min = -25.0F;
      new_longi_stat_curves[1].x_max = -15.0F;
      new_longi_stat_curves[1].a = 1.5F * calibs.k_lsc_max_a_coeff;
      new_longi_stat_curves[1].b = 0.1F;
      new_longi_stat_curves[1].c = 1.0F;
      new_longi_stat_curves[1].mean_lat_pos = -25.0F;

      new_longi_stat_curves[2].f_valid = true;
      new_longi_stat_curves[2].x_min = -25.0F;
      new_longi_stat_curves[2].x_max = -15.0F;
      new_longi_stat_curves[2].a = 0.0F * calibs.k_lsc_max_a_coeff;
      new_longi_stat_curves[2].b = 0.5F;
      new_longi_stat_curves[2].c = -1.0F;
      new_longi_stat_curves[2].mean_lat_pos = -25.0F;
   }

};

/** \purpose
 * Purpose is to verify that curve 0 and 2 have passed the sanity check and is populated
 * to the LSC output structure.
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Sanity_Check_And_Populate_LSC_Output, Sanity_Check_And_Populate_LSC_Output_Verify_LSC_Sanity_Check)
{

   /** \action
    * Call function
    */
   Sanity_Check_And_Populate_LSC_Output(calibs, nr_downselected_clusters, new_longi_stat_curves, longi_stat_curves);

   /** \result
    * Verify that curve 0 and 2 are valid and that curve 1 is invalid.
    * Also check that the LSC output have been filled correctly
    */
   CHECK_TRUE(longi_stat_curves[0].f_valid);
   CHECK_TRUE(longi_stat_curves[2].f_valid);
   CHECK_FALSE(longi_stat_curves[1].f_valid);

   for (uint32_t i = 0U; i < nr_downselected_clusters; i++)
   {
      DOUBLES_EQUAL(new_longi_stat_curves[i].x_min, longi_stat_curves[i].x_min, F360_EPSILON);
      DOUBLES_EQUAL(new_longi_stat_curves[i].x_max, longi_stat_curves[i].x_max, F360_EPSILON);
      DOUBLES_EQUAL(new_longi_stat_curves[i].a, longi_stat_curves[i].a, F360_EPSILON);
      DOUBLES_EQUAL(new_longi_stat_curves[i].b, longi_stat_curves[i].b, F360_EPSILON);
      DOUBLES_EQUAL(new_longi_stat_curves[i].c, longi_stat_curves[i].c, F360_EPSILON);
      DOUBLES_EQUAL(new_longi_stat_curves[i].mean_lat_pos, longi_stat_curves[i].mean_lat_pos, F360_EPSILON);
   }
   

}
/** @}*/

/** \defgroup  f360_update_longi_stat_curves_Fit_Second_Order_Polynomials_To_Clusters
 *  @{
 */

 /** \brief
  * This test group defines two LSC clusters. These clusters are then passed to the
  * function Fit_Second_Order_Polynomials_To_Clusters() to verify that fitted curves are populated correctly.
  * Note that this is not a test for the actual polynomial fit which is tested separately but rather a test
  * to verify that the population from the polynomial fit is assigned correctly. The objects are thus,
  * for simplicity placed on a line in longitudinal order and should yield a polynomial where both a and b
  * coefficients are 0.
  */
TEST_GROUP(f360_update_longi_stat_curves_Fit_Second_Order_Polynomials_To_Clusters)
{
   F360_Tracker_Info_T tracker_info;

   uint16_t nr_valid_clusters;
   F360_Longi_Stat_Cluster_T valid_clusters[MAX_NR_OF_LONGI_STAT_CURVES];
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS];
   F360_Longi_Stat_Curve_T longi_stat_curves[MAX_NR_OF_LONGI_STAT_CURVES];

   F360_LSC_Object_Group_Settings_T group_A;
   F360_LSC_Object_Group_Settings_T group_B;

   // These variables are used to derive the expected lateral mean position of the curve.
   // Since objects are placed on a line in longitudinal order we also expect the c coefficent
   // to be equal to the lateral mean
   float32_t exp_lat_mean_A = 0.0F;
   float32_t exp_lat_mean_B = 0.0F;

   float32_t test_pass_thres = 1.0e-5;

   /** \setup
    * Create two groups of objects and assign each of them to a cluster.
    * Also derive a clusters lateral mean
    */
   TEST_SETUP()
   {
      Set_Tracker_Variant(tracker_info.variant);
      Initialize_Tracker_Info(tracker_info);

      nr_valid_clusters = 2U;

      group_A = Add_LSC_Group_A(tracker_info, objects);
      
      valid_clusters[0].nr_objects = group_A.nr_objects;
      uint32_t first_idx = group_A.ids[0] - 1U;
      valid_clusters[0].first_object = &objects[first_idx];
      uint32_t last_idx = group_A.ids[group_A.nr_objects - 1U] - 1U;
      valid_clusters[0].last_object = &objects[last_idx];

      uint32_t second_index = group_A.ids[1] - 1U;
      objects[first_idx].lsc_next_in_cluster = &objects[second_index];
      objects[first_idx].lsc_prev_in_cluster = NULL;
      exp_lat_mean_A += objects[first_idx].vcs_position.y;
      for (uint32_t i = 1U; i < (group_A.nr_objects - 1U); i++)
      {
         uint32_t curr_obj_idx = group_A.ids[i] - 1;
         objects[curr_obj_idx].lsc_next_in_cluster = &objects[group_A.ids[i + 1U] - 1U];
         objects[curr_obj_idx].lsc_prev_in_cluster = &objects[group_A.ids[i - 1U] - 1U];
         exp_lat_mean_A += objects[group_A.ids[i] - 1U].vcs_position.y;
      }
      objects[last_idx].lsc_next_in_cluster = NULL;
      uint32_t second_last_index = group_A.ids[group_A.nr_objects - 2U] - 1U;
      objects[last_idx].lsc_prev_in_cluster = &objects[second_last_index];
      exp_lat_mean_A += objects[last_idx].vcs_position.y;

      exp_lat_mean_A /= static_cast<float32_t>(group_A.nr_objects);
      valid_clusters[0].lat_mean = exp_lat_mean_A;


      group_B = Add_LSC_Group_B(tracker_info, objects);

      valid_clusters[1].nr_objects = group_B.nr_objects;
      first_idx = group_B.ids[0] - 1U;
      valid_clusters[1].first_object = &objects[first_idx];
      last_idx = group_B.ids[group_B.nr_objects - 1U] - 1U;
      valid_clusters[1].last_object = &objects[last_idx];

      second_index = group_B.ids[1] - 1U;
      objects[first_idx].lsc_next_in_cluster = &objects[second_index];
      objects[first_idx].lsc_prev_in_cluster = NULL;
      exp_lat_mean_B += objects[first_idx].vcs_position.y;
      for (uint32_t i = 1U; i < (group_B.nr_objects - 1U); i++)
      {
         uint32_t curr_obj_idx = group_B.ids[i] - 1;
         objects[curr_obj_idx].lsc_next_in_cluster = &objects[group_B.ids[i + 1U] - 1U];
         objects[curr_obj_idx].lsc_prev_in_cluster = &objects[group_B.ids[i - 1U] - 1U];
         exp_lat_mean_B += objects[group_B.ids[i] - 1U].vcs_position.y;
      }
      objects[last_idx].lsc_next_in_cluster = NULL;
      second_last_index = group_B.ids[group_B.nr_objects - 2U] - 1U;
      objects[last_idx].lsc_prev_in_cluster = &objects[second_last_index];
      exp_lat_mean_B += objects[last_idx].vcs_position.y;

      exp_lat_mean_B /= static_cast<float32_t>(group_B.nr_objects);
      valid_clusters[1].lat_mean = exp_lat_mean_B;
   }

};

/** \purpose
 * Purpose is to verify that both clusters have generated a curve and inherited the clusters properties.
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Fit_Second_Order_Polynomials_To_Clusters, Fit_Second_Order_Polynomials_To_Clusters_Verify_Curves_Have_Inherited_Cluster_Props)
{

   /** \action
    * Call function
    */
   Fit_Second_Order_Polynomials_To_Clusters(nr_valid_clusters, valid_clusters, longi_stat_curves);

   /** \result
    * Verify that curve 0 and 1 are valid 
    * Also check that the LSC output have been filled correctly
    * Note that c coefficient is expected to be equal to the lateral mean since a and b coefficient should be 0.
    */
   for (uint16_t i = 0U; i < nr_valid_clusters; i++)
   {
      CHECK_TRUE(longi_stat_curves[i].f_valid);
      DOUBLES_EQUAL(0.0F, longi_stat_curves[i].a, test_pass_thres);
      DOUBLES_EQUAL(0.0F, longi_stat_curves[i].b, test_pass_thres);
   }
   
   DOUBLES_EQUAL(valid_clusters[0].first_object->vcs_position.x, longi_stat_curves[0].x_min, test_pass_thres);
   DOUBLES_EQUAL(valid_clusters[0].last_object->vcs_position.x, longi_stat_curves[0].x_max, test_pass_thres);
   DOUBLES_EQUAL(valid_clusters[0].lat_mean, longi_stat_curves[0].c, test_pass_thres);
   DOUBLES_EQUAL(valid_clusters[0].lat_mean, longi_stat_curves[0].mean_lat_pos, test_pass_thres);

   DOUBLES_EQUAL(valid_clusters[1].first_object->vcs_position.x, longi_stat_curves[1].x_min, test_pass_thres);
   DOUBLES_EQUAL(valid_clusters[1].last_object->vcs_position.x, longi_stat_curves[1].x_max, test_pass_thres);
   DOUBLES_EQUAL(valid_clusters[1].lat_mean, longi_stat_curves[1].c, test_pass_thres);
   DOUBLES_EQUAL(valid_clusters[1].lat_mean, longi_stat_curves[1].mean_lat_pos, test_pass_thres);
}
/** @}*/

/** \defgroup  f360_update_longi_stat_curves_Update_Longi_Stat_Curves
 *  @{
 */

 /** \brief
  * This test group defines 4 groups of objects.
  * Objects are created on 4 lines in longitudinal direction with various lateral position.
  * These groups of objects are then checked that they have been used to create LSC's as expected.
  */
TEST_GROUP(f360_update_longi_stat_curves_Update_Longi_Stat_Curves)
{
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS];
   F360_Tracker_Info_T tracker_info;
   F360_Longi_Stat_Curve_T longi_stat_curves[MAX_NR_OF_LONGI_STAT_CURVES];
   F360_Calibrations_T calibs;
   F360_TRKR_TIMING_INFO_T timing_info;
   F360_Host_T host;

   F360_LSC_Object_Group_Settings_T group_A;
   F360_LSC_Object_Group_Settings_T group_B;
   F360_LSC_Object_Group_Settings_T group_C;
   F360_LSC_Object_Group_Settings_T group_D;

   // These variables are used to derive the expected lateral mean position of the curves.
   // Since objects are placed on a line in longitudinal order we also expect the c coefficent
   // to be equal to the lateral mean
   float32_t exp_lat_mean_A = 0.0F;
   float32_t exp_lat_mean_B = 0.0F;

   float32_t test_pass_thres = 1.0e-5;

   /** \setup
    * Create 4 groups of objects and assign each of them to a cluster.
    * Also derive a clusters lateral mean
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      Set_Tracker_Variant(tracker_info.variant);

      Initialize_Tracker_Info(tracker_info);

      host.curvature_rear = 0.0F;
      group_A = Add_LSC_Group_A(tracker_info, objects);
      group_B = Add_LSC_Group_B(tracker_info, objects);
      group_C = Add_LSC_Group_C(tracker_info, objects);
      group_D = Add_LSC_Group_D(tracker_info, objects);

      exp_lat_mean_A = objects[group_A.ids[0] - 1U].vcs_position.y;
      exp_lat_mean_B = objects[group_B.ids[0] - 1U].vcs_position.y;
   }

};

/** \purpose
 * Purpose is to verify that group A and B have generated two LSC's as expected.
 * Also verify that curve 2 and 3 are invalid.
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Update_Longi_Stat_Curves, Update_Longi_Stat_Curves_Verify_LSC_Created_Group_A_And_B)
{

   /** \action
    * Call function
    */
   Update_Longi_Stat_Curves(tracker_info, calibs, host, objects, longi_stat_curves, timing_info);

   /** \result
    * Verify that curve 0 is valid and that properties of the curve is as expected from group A
    * Verify that curve 1 is valid and that properties of the curve is as expected from group B
    * Verify that curve 2 and 3 are invalid with default properties
    */
   CHECK_TRUE(longi_stat_curves[0].f_valid);
   uint32_t first_obj_idx_A = group_A.ids[0] - 1U;
   uint32_t last_obj_idx_A = group_A.ids[group_A.nr_objects - 1U] - 1U;
   DOUBLES_EQUAL(objects[first_obj_idx_A].vcs_position.x, longi_stat_curves[0].x_min, test_pass_thres);
   DOUBLES_EQUAL(objects[last_obj_idx_A].vcs_position.x, longi_stat_curves[0].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].b, test_pass_thres);
   DOUBLES_EQUAL(exp_lat_mean_A, longi_stat_curves[0].c, test_pass_thres);
   DOUBLES_EQUAL(exp_lat_mean_A, longi_stat_curves[0].mean_lat_pos, test_pass_thres);

   CHECK_TRUE(longi_stat_curves[1].f_valid);
   uint32_t first_obj_idx_B = group_B.ids[0] - 1U;
   uint32_t last_obj_idx_B = group_B.ids[group_B.nr_objects - 1U] - 1U;
   DOUBLES_EQUAL(objects[first_obj_idx_B].vcs_position.x, longi_stat_curves[1].x_min, test_pass_thres);
   DOUBLES_EQUAL(objects[last_obj_idx_B].vcs_position.x, longi_stat_curves[1].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].b, test_pass_thres);
   DOUBLES_EQUAL(exp_lat_mean_B, longi_stat_curves[1].c, test_pass_thres);
   DOUBLES_EQUAL(exp_lat_mean_B, longi_stat_curves[1].mean_lat_pos, test_pass_thres);

   CHECK_FALSE(longi_stat_curves[2].f_valid);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].x_min, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].b, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].c, test_pass_thres);
   DOUBLES_EQUAL(INFTY, longi_stat_curves[2].mean_lat_pos, test_pass_thres);

   CHECK_FALSE(longi_stat_curves[3].f_valid);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].x_min, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].b, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].c, test_pass_thres);
   DOUBLES_EQUAL(INFTY, longi_stat_curves[3].mean_lat_pos, test_pass_thres);
}

/** \purpose
 * Purpose is to verify that no LSCs are generated when all tracked objects are moveable
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Update_Longi_Stat_Curves, Update_Longi_Stat_Curves_Verify_No_LSCs_Are_Created)
{
   /** \precond
    * Set moveable flag for all active objects to true
    */
   for (uint32_t i = 0U; i < static_cast<uint32_t>(tracker_info.num_active_objs); i++)
   {
      objects[i].f_moveable = true;
   }
   
   /** \action
    * Call function
    */
   Update_Longi_Stat_Curves(tracker_info, calibs, host, objects, longi_stat_curves, timing_info);

   /** \result
    * Verify that curve 0 is valid and that properties of the curve is as expected from group A
    * Verify that curve 1 is valid and that properties of the curve is as expected from group B
    * Verify that curve 2 and 3 are invalid with default properties
    */
   CHECK_FALSE(longi_stat_curves[0].f_valid);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].x_min, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].b, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[0].c, test_pass_thres);
   DOUBLES_EQUAL(INFTY, longi_stat_curves[0].mean_lat_pos, test_pass_thres);

   CHECK_FALSE(longi_stat_curves[1].f_valid);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].x_min, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].b, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[1].c, test_pass_thres);
   DOUBLES_EQUAL(INFTY, longi_stat_curves[1].mean_lat_pos, test_pass_thres);

   CHECK_FALSE(longi_stat_curves[2].f_valid);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].x_min, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].b, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[2].c, test_pass_thres);
   DOUBLES_EQUAL(INFTY, longi_stat_curves[2].mean_lat_pos, test_pass_thres);

   CHECK_FALSE(longi_stat_curves[3].f_valid);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].x_min, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].x_max, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].a, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].b, test_pass_thres);
   DOUBLES_EQUAL(0.0F, longi_stat_curves[3].c, test_pass_thres);
   DOUBLES_EQUAL(INFTY, longi_stat_curves[3].mean_lat_pos, test_pass_thres);
}
/** @}*/

/** \defgroup  f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster
 *  @{
 */

 /** \brief
  * The test group for testing Is_Object_Valid_For_LSC_Cluster function
  */
TEST_GROUP(f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster)
{
   F360_Object_Track_T object;
   F360_Calibrations_T calibs;
   float host_turn_radius;

   /** \setup
    * Set host curvature to 0.0
    * Add an object to be valid for LSC cluster
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);

      host_turn_radius = 0.0F;
      object.vcs_position.x = 20.0F;
      object.vcs_position.y = 5.0F;
      object.f_moveable = false;
      object.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA; 
   }

};

/** \purpose
 * Purpose is to verify that object which is stationary CCA and is not close to the host path is concidered valid for LSC clustering 
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster, Verify_If_Object_Is_Allowed_for_LSC_Cluster_Stationary_CCA_Not_Close_To_Host_Path)
{
   /** \precond
    * The default case from the test setup
    */

   /** \action
    * Call function
    */
   bool f_object_valid_for_lsc = Is_Object_Valid_For_LSC_Cluster(object,calibs,host_turn_radius);

   /** \result
    * Verify that the object is considered valid for LSC clustering
    */
   CHECK_TRUE_TEXT(f_object_valid_for_lsc, "The object is incorrectly considered to be invalid for lsc clustering");
}

/** \purpose
 * Purpose is to verify that object which is stationary CTCA and is not close to the host path is not concidered valid for LSC clustering 
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster, Verify_If_Object_Is_Allowed_for_LSC_Cluster_Stationary_CTCA_Not_Close_To_Host_Path)
{
   /** \precond
    * The object filer type set to CTCA
    */
   object.trk_fltr_type = F360_TRACKER_TRKFLTR_CTCA;

   /** \action
    * Call function
    */
   bool f_object_valid_for_lsc = Is_Object_Valid_For_LSC_Cluster(object,calibs,host_turn_radius);

   /** \result
    * Verify that the object is considered invalid for LSC clustering
    */
   CHECK_FALSE_TEXT(f_object_valid_for_lsc, "The object is incorrectly considered to be valid for lsc clustering");
}

/** \purpose
 * Purpose is to verify that object which is moving and is not close to the host path is not concidered valid for LSC clustering 
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster, Verify_If_Object_Is_Allowed_for_LSC_Cluster_Moving_Not_Close_To_Host_Path)
{
   /** \precond
    * The object f_movable flag is set to true
    */
   object.f_moveable = true;

   /** \action
    * Call function
    */
   bool f_object_valid_for_lsc = Is_Object_Valid_For_LSC_Cluster(object,calibs,host_turn_radius);

   /** \result
    * Verify that the object is considered invalid for LSC clustering
    */
   CHECK_FALSE_TEXT(f_object_valid_for_lsc, "The object is incorrectly considered to be valid for lsc clustering");
}


/** \purpose
 * Purpose is to verify that object which is stationary CCA and is close to the host path is not concidered valid for LSC clustering 
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster, Verify_If_Object_Is_Allowed_for_LSC_Cluster_Stationary_CCA_Close_To_Host_Path)
{
   /** \precond
    * The object location is set to be located close to the host path
    */
   object.vcs_position.x = 2.0F;
   object.vcs_position.y = 0.0F;

   /** \action
    * Call function
    */
   bool f_object_valid_for_lsc = Is_Object_Valid_For_LSC_Cluster(object,calibs,host_turn_radius);

   /** \result
    * Verify that the object is considered invalid for LSC clustering
    */
   CHECK_FALSE_TEXT(f_object_valid_for_lsc, "The object is incorrectly considered to be valid for lsc clustering");
}

/** \purpose
 * Purpose is to verify that object which is stationary CCA and is not close to the host path while host has a non-zero curvature is concidered valid for LSC clustering 
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Is_Object_Valid_For_LSC_Cluster, Verify_If_Object_Is_Allowed_for_LSC_Cluster_Stationary_CCA_Not_Close_To_NOT_Streight_Host_Path)
{
   /** \precond
    * set up the pbject postion and host curvature
    */
   host_turn_radius = 1.0F;
   object.vcs_position.x = 20.0F;
   object.vcs_position.x = 2.0F;

   /** \action
    * Call function
    */
   bool f_object_valid_for_lsc = Is_Object_Valid_For_LSC_Cluster(object,calibs,host_turn_radius);

   /** \result
    * Verify that the object is considered valid for LSC clustering
    */
   CHECK_TRUE_TEXT(f_object_valid_for_lsc, "The object is incorrectly considered to be invalid for lsc clustering");
}

/** \defgroup  f360_update_longi_stat_curves_Arrange_First_Iteration
 *  @{
 */

/** \brief
 * This test group defines a set of objects to be used to test function Arrange_First_Iteration().
 */
TEST_GROUP(f360_update_longi_stat_curves_Arrange_First_Iteration_Track_In_Host_Path)
{
   // Declare common variables used within all tests in this test group.
   F360_Tracker_Info_T tracker_info;
   F360_Object_Track_T objects[NUMBER_OF_OBJECT_TRACKS];
   F360_Calibrations_T calibs;
   uint16_t nr_next_ids_of_interest;
   uint16_t next_ids_of_interest[NUMBER_OF_OBJECT_TRACKS];
   F360_Host_T host;

   bool f_is_data_remaining;

   F360_LSC_Object_Group_Settings_T group_A_modified;

   /** \setup
    * Initialize default calibrations and create a set of objects.
    */
   TEST_SETUP()
   {
      Initialize_Tracker_Calibrations(calibs);
      Set_Tracker_Variant(tracker_info.variant);

      Initialize_Tracker_Info(tracker_info);
      host.curvature_rear = 0.0F;
      group_A_modified = Add_LSC_Group_A_modified(tracker_info, objects);
   }

};

/** \purpose
 * Purpose is to verify that no objects have been selected if they are within
 * the valid interval in longitudinal direction but located in the host path 
 * \req
 * NA
 */
TEST(f360_update_longi_stat_curves_Arrange_First_Iteration_Track_In_Host_Path, Arrange_First_Iteration_All_Objects_Relevant_But_In_The_Host_Path)
{
   /** \precond
    * Use group_A_modified (the roup is designed to create objects in the host path)
    * Modify interval in calibration to guarantee all objects are within relevant interval
    */
   uint16_t first_idx = group_A_modified.ids[0] - 1U;
   calibs.k_lsc_min_long_pos = objects[first_idx].vcs_position.x - 1.0F;
   uint16_t last_index = group_A_modified.ids[group_A_modified.nr_objects - 1U] - 1U;
   calibs.k_lsc_max_long_pos = objects[last_index].vcs_position.x + 1.0F;

   /** \action
    * Call function
    */
   f_is_data_remaining = Arrange_First_Iteration(tracker_info, calibs, host, nr_next_ids_of_interest, next_ids_of_interest);

   /** \result
    * Verify no objects have been selected and that function has returned false
    */
   CHECK_FALSE_TEXT(f_is_data_remaining, "LSC created despite objects being located in the host path");
}

/** @}*/
