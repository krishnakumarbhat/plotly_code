/** \file
   Implementation of this file shall be done in DFT-347
*/

#include "f360_initialization_preconditions.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

//#include "headerfile_needed.h"

/** \defgroup  f360_initialization_preconditions
 *  @{
 */

using namespace f360_variant_A;

/** \brief
 *  Test group for Initialization_Preconditions function
 */

TEST_GROUP(f360_initialization_preconditions)
{
   F360_Cluster_T cluster_to_init = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Detection_Hist_T detection_hist = {};
   F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS] = {};
   F360_Calibrations_T calibrations = {};
   F360_Tracker_Info_T tracker_info = {};
   /** \setup
   * Initialization clusters, detections and calibrations by values which allows preconditions to pass
   */
   TEST_SETUP()
   {
      cluster_to_init.id = 5;

      cluster_to_init.f_dealiased = true;
      cluster_to_init.ndets = 5;
      cluster_to_init.num_old_dets = 5;
      cluster_to_init.num_types_of_dets[0] = cluster_to_init.ndets + cluster_to_init.num_old_dets;
      cluster_to_init.num_types_of_dets[1] = 0;

      for (int32_t idx = 0; idx < cluster_to_init.ndets; idx++)
      {
         cluster_to_init.detids[idx] = idx + 1;
      }

      for (int32_t idx = 0; idx < cluster_to_init.num_old_dets; idx++)
      {
         cluster_to_init.old_det_idx[idx] = idx;
      }

      calibrations.k_init_trk_preconditions_min_current_dets = 1;
      calibrations.k_init_trk_preconditions_min_mov_dets_sr = 5;
      calibrations.k_init_trk_preconditions_min_mov_amb_dets = 6;
      calibrations.k_init_trk_preconditions_min_amb_dets = 4;
      calibrations.k_init_trk_preconditions_min_mov_dets_lr = 4;
      calibrations.k_obj_init_min_correct_dets_ratio = 0.35F;
      calibrations.k_obj_init_min_number_of_correct_dets = 2;
      calibrations.f_nees_cfmi_extended_preconditions = false;
      calibrations.k_nees_cfmi_extended_prec_max_dets = 6;
      calibrations.k_nees_cfmi_extended_prec_2_dets_max_std = F360_KPH2MPS(0.1F);
      calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = F360_KPH2MPS(0.1F);
      calibrations.k_nees_cfmi_extended_prec_4_dets_max_std = F360_KPH2MPS(1.0F);
      calibrations.k_nees_cfmi_extended_prec_5_dets_max_std = F360_KPH2MPS(10.0F);
      calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = F360_KPH2MPS(25.0F);

      Set_Tracker_Variant(tracker_info.variant);
   }
};

/**
*\purpose  Check if preconditions will be invalid in case that all nominal conditions are met 
           (Positive test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__Nominal_Case_POS)
{
   /** \step{1}
    * Take default input form test setup and check output
    */

   /** \precond
    * NA
    */

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected True
    */
   CHECK_TRUE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be invalid in case that cluster is not de-aliased 
           (Negative test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__Not_Dealiased_NEG)
{
   /** \step{1}
    * Change de-aliased flag to false
    */

    /** \precond
     * NA
     */
   cluster_to_init.f_dealiased = false;

     /** \action
      * Call Initialization_Preconditions
      */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected False
    */
   CHECK_FALSE(f_precondition_valid);
}


/**
*\purpose  Check if preconditions will be invalid in case that cluster has no detection from current instance 
           (Negative test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__N_Curent_Dets_Below_Thr_NEG) 
{
   /** \step{1}
    * Change number of current detection to 0
    */

    /** \precond
     * NA
     */
   cluster_to_init.ndets = 0;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected False
    */
   CHECK_FALSE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be valid in case that cluster number of detections will be bigger 
           than threshold but number of ambiguous and moving detection are blow threshold
           (Positive test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__N_All_Dets_Above_Thr_POS)
{
   /** \step{1}
    * Change number of detection to pass only one of 'or' conditions number of detections
    */

    /** \precond
     * NA
     */
   cluster_to_init.ndets = 1;
   cluster_to_init.num_old_dets = 5;
   cluster_to_init.num_types_of_dets[0] = 3;
   cluster_to_init.num_types_of_dets[1] = 3;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected True
    */
   CHECK_TRUE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be valid in case that cluster number of moving detections will be bigger
           than threshold but number of ambiguous and total detection are blow threshold
           (Positive test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__N_Mov_Dets_Above_Thr_POS)
{
   /** \step{1}
    * Change number of detection to pass only one of 'or' conditions number of detections
    */

    /** \precond
     * NA
     */
   cluster_to_init.ndets = 2;
   cluster_to_init.num_old_dets = 2;
   cluster_to_init.num_types_of_dets[0] = 4;
   cluster_to_init.num_types_of_dets[1] = 0;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected True
    */
   CHECK_TRUE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be valid in case that cluster number of ambiguous detections will be bigger
           than threshold but number of moving and total detection are blow threshold
           (Positive test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__N_Amb_Dets_Above_Thr_POS)
{
   /** \step{1}
    * Change number of detection to pass only one of 'or' conditions number of detections
    */

    /** \precond
     * NA
     */
   cluster_to_init.ndets = 2;
   cluster_to_init.num_old_dets = 2;
   cluster_to_init.num_types_of_dets[0] = 0;
   cluster_to_init.num_types_of_dets[1] = 4;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected True
    */
   CHECK_TRUE(f_precondition_valid);
}

/**
*\purpose  When preconditions will be valid in case that cluster number of ambiguous detections will be bigger
           than threshold but number of moving and total detections number are below threshold. Some detections are in all looks
           (Positive test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions_N_Amb_Dets_Above_Thr_POS_Dets_In_All_Looks)
{
    /** \precond
     * NA
     */
   cluster_to_init.ndets = 2;
   cluster_to_init.num_old_dets = 2;
   cluster_to_init.num_types_of_dets[0] = 0;
   cluster_to_init.num_types_of_dets[1] = 4;

   sensors[0].constant.range_limits[0] = 60.0F;
   sensors[0].constant.range_limits[1] = 70.0F;
   sensors[0].constant.range_limits[2] = 60.0F;
   sensors[0].constant.range_limits[3] = 70.0F;

   raw_detection_list.detections[0].raw.range = 10;
   raw_detection_list.detections[0].raw.sensor_id = 1;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected True
    */
   CHECK_TRUE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be invalid in case that cluster total number of detections, 
           number ambiguous and moving detections will be below threshold
           (Negative test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__N_Dets_Below_Thr_NEG)
{
   /** \step{1}
    * Change number of detection to fail all of 'or' conditions number of detections
    */

    /** \precond
     * NA
     */
   cluster_to_init.ndets = 1;
   cluster_to_init.num_old_dets = 1;
   cluster_to_init.num_types_of_dets[0] = 0;
   cluster_to_init.num_types_of_dets[1] = 2;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected False
    */
   CHECK_FALSE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be invalid in case that detections associated to cluster
           are potential angle jumps
           (Negative test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__Affected_Angle_Jump_NEG)
{
   /** \step{1}
    * Change each associated detection to be potential angle jump
    */

    /** \precond
     * NA
     */
   for (int32_t idx = 0; idx < cluster_to_init.ndets; idx++)
   {
      det_props[idx].f_potential_angle_jump = true;
   }

   for (int32_t idx = 0; idx < cluster_to_init.num_old_dets; idx++)
   {
      detection_hist.det_data[idx].f_potential_angle_jump = true;
   }

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected False
    */
   CHECK_FALSE(f_precondition_valid);
}

/**
*\purpose  Check if preconditions will be valid in case that extended preconditions are enabled
           (Positive test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__Extended_Preconditions_Nominal_POS)
{
   /** \step{1}
    * Change calibration to enable extended preconditions
    */

    /** \precond
     * NA
     */
   calibrations.f_nees_cfmi_extended_preconditions = true;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected True
    */
   CHECK_TRUE(f_precondition_valid);
}


/**
*\purpose  Check if preconditions will be invalid in case that extended preconditions are enabled, calibrations are 
           changed to check covariance trace and detections are invalid
           (Negative test case)
*\req  NA
*/
TEST(f360_initialization_preconditions, Initialization_Preconditions__Extended_Preconditions_NEG)
{
   /** \step{1}
    * Change calibration to enable extended preconditions
    */

    /** \precond
     * NA
     */
   calibrations.f_nees_cfmi_extended_preconditions = true;
   calibrations.k_nees_cfmi_extended_prec_max_dets = 60;

   /** \action
    * Call Initialization_Preconditions
    */

   bool f_precondition_valid = Initialization_Preconditions(calibrations, cluster_to_init, sensors, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Expected False
    */
   CHECK_FALSE(f_precondition_valid);
}

/** @}*/

