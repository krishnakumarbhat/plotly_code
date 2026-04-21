/** \file
   This file contains unit tests for functions from f360_nees_cfmi_preconditions file. 
*/

#include "f360_nees_cfmi_preconditions.h"
#include "f360_math.h"
#include "f360_set_variant.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>
#include <cfloat>

/** \defgroup  f360_nees_cfmi_preconditions
*  @{
*/

using namespace f360_variant_A;
/** \brief
*  This test group tests NEES_CFMI_Preconditions() function.
**/
TEST_GROUP(f360_nees_cfmi_preconditions)
{
   F360_Calibrations_T calibrations = {};
   F360_Cluster_T cluster_to_init = {};
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS] = {};
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list{};
   F360_Detection_Hist_T detection_hist = {};
   F360_Tracker_Info_T tracker_info = {};
   float32_t base_angle = 0.1F*F360_PI;
   /** \setup
   *   Initialize detections properties and historical detections properties. 
   **/
   TEST_SETUP()
   {
      calibrations.k_nees_cfmi_extended_prec_max_dets = 6;
      calibrations.k_nees_cfmi_rrate_comp_std = 0.1F;

      Set_Tracker_Variant(tracker_info.variant);

      // Initialize current detections parameters.
      for (uint32_t index = 0; index < MAX_DETS_IN_OBJ_TRK; index++)
      {
         cluster_to_init.detids[index] = index + 1;
         int32_t det_index = cluster_to_init.detids[index]-1;
         raw_detection_list.detections[det_index].processed.vcs_az = index* base_angle;
         raw_detection_list.detections[det_index].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[det_index].processed.vcs_az);
         raw_detection_list.detections[det_index].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[det_index].processed.vcs_az);
         det_props[det_index].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_X] = 0.0001F;
         det_props[det_index].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.00001F;
         det_props[det_index].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_X] = 0.00001F;
         det_props[det_index].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_Y] = 0.0001F;
      }

      // Initialize historical detections parameters.
      for (uint32_t index = 0; index < MAX_DETS_IN_OBJ_TRK; index++)
      {
         cluster_to_init.old_det_idx[index] = index;
         int32_t det_index = cluster_to_init.old_det_idx[index];
         detection_hist.det_data[det_index].vcs_az = index* base_angle;
         detection_hist.det_data[det_index].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_X] = 0.0001F;
         detection_hist.det_data[det_index].position_cov_nees[F360_2D_IDX_X][F360_2D_IDX_Y] = 0.00001F;
         detection_hist.det_data[det_index].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_X] = 0.00001F;
         detection_hist.det_data[det_index].position_cov_nees[F360_2D_IDX_Y][F360_2D_IDX_Y] = 0.0001F;
      }
   }

   TEST_TEARDOWN()
   {
   }
};

/**
*\purpose  This test checks function output when number all of detections in cluster
*          is greater that maximum number of detections for invalid precondition consideration.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestNumberOfDetsIndicatePreconditionPOS)
{
   /** \precond
    * Cluster contains 6 current detections and 1 historical detection.
    * Maximum number of detections for invalid precondition consideration is 6.
    **/
   cluster_to_init.ndets = 6;
   cluster_to_init.num_old_dets = 1;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Due to number of detections in cluster precondition should be valid.
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  This test checks function output when cluster
*          is empty.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestLackOfDetectionsInCluster)
{
   /** \precond
    * Cluster is empty all variable are already set in TEST_SETUP.
    **/

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Due to lack of detections in cluster precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  This test check function output when cluster
*          contains one detection.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestOneHistoricalDetectionInCluster)
{
   /** \precond
    * Cluster contains one historic detection.
    **/
   cluster_to_init.num_old_dets = 1;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * For one detection in cluster precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  This test check function output when cluster
*          contains 2 detections, which should not meet preconditions. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestTwoCurrentDetectionsInClusterConditonNEG)
{
   /** \precond
    Cluster contains 2 current detections.
    **/
   cluster_to_init.ndets = 2;
   calibrations.k_nees_cfmi_extended_prec_2_dets_max_std = 0.4F;

    /** \action
     * Call NEES_CFMI_Preconditions().
     **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * For those detections precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}

/**
*\purpose  This test check function output when cluster
*          contains 2 detections, which should meet preconditions.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestTwoCurrentDetectionsInClusterConditonPOS)
{
   /** \precond
    * Cluster contains 1 current detection and 1 historical detection.
    **/
   cluster_to_init.ndets = 1;
   cluster_to_init.num_old_dets = 1;
   calibrations.k_nees_cfmi_extended_prec_2_dets_max_std = 5.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * For those detections precondition should be valid.
    **/
   CHECK_TRUE(f_preconditions_valid);
}


/**
*\purpose  This test check function output when cluster
*          contains 3 current detections, which should meet validity precondition.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestThreeCurrentDetectionsInClusterConditonPOS)
{
   /** \precond
    * Cluster contains 3 current detections.
    **/
   cluster_to_init.ndets = 3;
   calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = 5.0F;

   /** \action
    * Call NEES_CFMI_Preconditions(). 
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * For those detections precondition should be valid.
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  This test check function output when cluster
*          contains 3 current detections, which should meet validity precondition, but range rate
           standard deviation is zero.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestThreeCurrentDetectionsInClusterConditonWithZeroRRStdNEG)
{
   /** \precond
    * Cluster contains 3 current detections and range rate std to 0.
    **/
   cluster_to_init.ndets = 3;
   calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = 5.0F;
   calibrations.k_nees_cfmi_rrate_comp_std = 0.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * For this condition precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  This test checks function output when cluster
*          contains 3 detections, which should meet validity precondition.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestThreeDetectionsInClusterConditonPOS)
{
   /** \precond
     Cluster contains 2 current detections and 1 historical detection.
    **/
   cluster_to_init.ndets = 2;
   cluster_to_init.num_old_dets = 1;
   calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = 0.5F;

   /** \action
    * Call NEES_CFMI_Preconditions(). 
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * For those detections precondition should be valid.
    **/
   CHECK_TRUE(f_preconditions_valid);
}


/**
*\purpose  This test checks function output when cluster
*          contains 3 detections.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestThreeDetectionsInClusterConditonNEG)
{
   /** \precond
    * Cluster contains 1 current detection and two historical detections.
    **/
   cluster_to_init.ndets = 1;
   cluster_to_init.num_old_dets = 2;

   calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = 0.1F;

   /** \action
    * Call NEES_CFMI_Preconditions(). 
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * In this test case calculated cov trace should be greater than calculated max cov trace value.
    * For those detections precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  Test for 4 detections in cluster. 
*          They should meet validity condition.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestFourDetectionsInClusterConditonPOS)
{
   /** \precond
    Cluster contains 4 current detections.
    **/
   cluster_to_init.ndets = 2;
   cluster_to_init.num_old_dets = 2;

   calibrations.k_nees_cfmi_extended_prec_4_dets_max_std = 0.20F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *describe test result
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  Test for 4 detections in cluster. 
*          They comes from current data and should not meet validity condition.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestFourDetectionsInClusterConditonNEG)
{
   /** \precond
    * Cluster contains 4 current detections.
    **/
   cluster_to_init.ndets = 4;
   calibrations.k_nees_cfmi_extended_prec_4_dets_max_std = 0.15F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * In this testcase calculated cov trace should be greater than calculated max cov trace value.
    * Precondition validity should failed.
    **/
   CHECK_FALSE(f_preconditions_valid);
}

/**
*\purpose  Test for five detections in cluster. 
*          They comes from current data. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestFiveDetectionsInClusterConditonNEG)
{
   /** \precond
    * Cluster contains 5 current detections.
    **/
   cluster_to_init.ndets = 5;

   calibrations.k_nees_cfmi_extended_prec_5_dets_max_std = 0.1F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * In this testcase calculated cov trace should be greater than calculated max cov trace value.
    * Due to this fact this precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}

/**
*\purpose  Test for six detections in cluster. 
*          They comes from historical and current data. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestSixDetectionsInClusterConditonNEG)
{
   /** \precond
    * Cluster contains 4 historical ans 2 current detecions.
    **/
   cluster_to_init.num_old_dets = 4;
   cluster_to_init.ndets = 2;
   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 0.01F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * In this testcase calculated cov trace should be greater than calculated max cov trace value.
    * Due to this fact this precondition should be invalid.
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  Test for seven detections in cluster. 
*          They comes from historical and current data. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestSevenDetectionsInClusterConditonPOS)
{
   /** \precond
    * Cluster contains 4 historical detections and 3 current detections.
    **/

   cluster_to_init.num_old_dets = 4;
   cluster_to_init.ndets = 3;
   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 100.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * In this testcase calculated cov trace should be smaller than calculated max cov trace value.
    * Due to this fact this precondition should be valid.
    **/
   CHECK_TRUE(f_preconditions_valid);
}


/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          to many current detections.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestTooManyCurrentDetsInCluster)
{
   /** \precond
    * Numbers of current detections in cluster are greater 
    * than maximum number of detections in tracker object.
    * However, it is smaller than maximum number of detections for extended precondition calculation. 
    **/
   cluster_to_init.num_old_dets = 4;
   cluster_to_init.ndets = MAX_DETS_IN_OBJ_TRK+1;
   calibrations.k_nees_cfmi_extended_prec_max_dets = cluster_to_init.num_old_dets + cluster_to_init.ndets;
   calibrations.k_nees_cfmi_extended_prec_4_dets_max_std = 1.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  In this case only current detections was used for cov_trace calculation.
    *  Even though, extended precondition covariance threshold should be calculated from 6 and more max std
    *  Due to this fact this condition should fail. 
    **/
   CHECK_FALSE(f_preconditions_valid);
}

/**
*\purpose  Tests checks how function handle the situation when cluster contains
*          to many current detections and does not contain any historical detections.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestTooManyCurrentDetsInClusterWithoutHistoricalDets)
{
   /** \precond
    * Numbers of current detections in cluster are greater
    * than maximum number of detections in tracker object.
    * However, it is smaller than maximum number of detections for extended precondition calculation.
    * Cluster does not contain historical detections. 
    **/
   cluster_to_init.ndets = MAX_DETS_IN_OBJ_TRK + 1;
   calibrations.k_nees_cfmi_extended_prec_max_dets = cluster_to_init.ndets;
   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  In this case any detection was used for cov_tracke calculation, so its value would be inverse of min denominator.
    *  Due to this fact this condition should fail. 
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          to many historical detections. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestTooManyHistoricalDetsInCluster)
{
   /** \precond
    * Numbers of historical detections in cluster are greater 
    * than maximum number of detections in tracker object.
    * However, it is smaller than maximum number of detections for extended precondition calculation. 
    **/
   cluster_to_init.num_old_dets = MAX_DETS_IN_OBJ_TRK +1;
   cluster_to_init.ndets = 3;
   calibrations.k_nees_cfmi_extended_prec_max_dets = cluster_to_init.num_old_dets + cluster_to_init.ndets;
   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  In this case only current detections was used for cov_trace calculation.
    *  Even though, extended precondition covariance threshold should be calculated from 6 and more max std
    *  Due to this fact this condition should pass. 
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          to many historical and current detections. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestTooManyDetsInCluster)
{
   /** \precond
    * Numbers of historical and current detection in cluster are greater 
    * than corresponding maximum number of detections in tracker object.
    * However, it is smaller than maximum number of detections for extended precondition calculation. 
    **/

   cluster_to_init.num_old_dets = MAX_DETS_IN_OBJ_TRK+1;
   cluster_to_init.ndets = MAX_DETS_IN_OBJ_TRK+1;
   calibrations.k_nees_cfmi_extended_prec_max_dets = cluster_to_init.num_old_dets + cluster_to_init.ndets;
   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  In this case any detection was used for cov_tracke calculation, so its value would be inverse of min denominator.
    *  Due to this fact this condition should fail. 
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          historical detection with invalid index. 
*          Current detections in cluster have valid ids.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestHistoricDetsWithInvalidIndex)
{
   /** \precond
   * Cluster contains 3 old detections and 3 detections from current instance. 
    * one historical detection have invalid index.
    **/

   cluster_to_init.num_old_dets = 4;
   cluster_to_init.ndets = 3;
   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;
   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  One historical detection have invalid index, algorithm should calculated cov_trace
    *  based on 5 others detections. Even though, algorithm used only 5 detections 
    *  for evaluation of cov_trace, cov_track threshold will be calculated for 6 detections.
    *  Precondition should be valid in this case.
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          current detections with invalid ids. 
*          Historical detections in cluster have valid indexes.  
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestCurrentDetWithInvalidId)
{
   /** \precond
    * Cluster contains 3 old detections and 3 detections from current instance. 
    * one current detection have invalid id.
    **/

   cluster_to_init.num_old_dets = 3;
   cluster_to_init.ndets = 3;

   cluster_to_init.detids[0] = MAX_NUMBER_OF_DETECTIONS + 1;

   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;

   /** \action
    *  Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  One current detection have invalid id, algorithm should calculated cov_trace
    *  based on 5 others detections. Even though, algorithm used only current detections 
    *  for evaluation of cov_trace, cov_track threshold will be calculated for 6 detections.
    *  Precondition should be valid in this case.
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  Tests checks how function handle the situation when cluster contains
*          current detections with invalid ids.
*          Historical detections in cluster have valid indexes.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestCurrentDetWithInvalidIdAndWrongCalibrationSet)
{
   /** \precond
    Cluster contains 3 old detections and 3 detections from current instance.
    * one current detection have invalid id.
    **/

   cluster_to_init.num_old_dets = 3;
   cluster_to_init.ndets = 3;

   cluster_to_init.detids[0] = MAX_NUMBER_OF_DETECTIONS + 1;

   calibrations.k_nees_cfmi_extended_prec_5_dets_max_std = 1.0F;

   /** \action
    *  Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  One current detection have invalid id, algorithm should calculated cov_trace
    *  based on 5 others detections. Even though, algorithm used only current detections
    *  for evaluation of cov_trace, cov_track threshold will be calculated for 6 detections.
    *  Precondition should fail in this case.
    **/
   CHECK_FALSE(f_preconditions_valid);
}


/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          current detections with invalid ids. 
*          However, historical detections in cluster have valid indexes.  
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestCurrentDetsWithInvalidIDs)
{
   /** \precond
    * Cluster contains 3 old detections and 3 detections from current instance. 
    * All current dets have invalid ids.
    **/

   cluster_to_init.num_old_dets = 3;
   cluster_to_init.ndets = cluster_to_init.num_old_dets;
   for (int32_t inx = 0; inx < cluster_to_init.ndets; inx++)
   { 
      int32_t invalid_detids = MAX_NUMBER_OF_DETECTIONS + inx + 1;
      cluster_to_init.detids[inx] = invalid_detids;
   }

   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  Current detections have invalid ids, algorithm should calculated cov_trace
    *  based on current detection. Even though, algorithm used only current detections 
    *  for evaluation of cov_trace, cov_track threshold will be calculated for all detections.
    *  Precondition should be valid in this case.
    **/
   CHECK_TRUE(f_preconditions_valid);
}


/**
*\purpose  Tests checks how function handle the situation when cluster contains 
*          historical detections with invalid indexes. 
*          However, current detections in cluster have valid ids.   
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestHistoricDetsWithInvalidIndexes)
{
   /** \precond
    *  Cluster contains 3 old detections and 3 detections from current instance. 
    *  All historical dets have invalid indexes.
    **/
   cluster_to_init.num_old_dets = 3;
   cluster_to_init.ndets = 3;
   for (int32_t inx = 0; inx < cluster_to_init.num_old_dets; inx++)
   {
      cluster_to_init.old_det_idx[inx] = MAX_NUMBER_OF_HISTORIC_DETECTIONS + inx;
   }

   calibrations.k_nees_cfmi_extended_prec_6_and_more_dets_max_std = 1.0F;

   /** \action
    *  Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  Historical detections have invalid indexes, algorithm should calculated cov_trace
    *  based on current detection. Even though, algorithm used only current detections 
    *  for evaluation of cov_trace, cov_track threshold will be calculated for all detections.
    *  Precondition should be valid in this case.
    **/
   CHECK_TRUE(f_preconditions_valid);
}

/**
*\purpose  Tests checks how function handle the situation when cluster only contains detections 
*          with invalid indexes and ids. 
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestAllDetsWithInvalidIndexesandIDs)
{
   /** \precond
    *  Cluster contains 3 old detections and 2 detections from current instance. 
    *  All current dets in cluster have invalid ids 
    *  and all historical dets have invalid indexes.
    **/
   cluster_to_init.num_old_dets = 3;
   cluster_to_init.ndets = 2;
   calibrations.k_nees_cfmi_extended_prec_5_dets_max_std = 100.0F ;
   
   for (int32_t inx = 0; inx < cluster_to_init.ndets; inx++)
   {
      int32_t invalid_detids = MAX_NUMBER_OF_DETECTIONS + inx+1;
      cluster_to_init.detids[inx] = invalid_detids;
   }
   
   for (int32_t inx = 0; inx < cluster_to_init.num_old_dets; inx++)
   {
      cluster_to_init.old_det_idx[inx] = MAX_NUMBER_OF_HISTORIC_DETECTIONS + inx;
   }

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    * Due to the fact that determinants cannot be calculated based on any detections, 
    * calculated cov_trace value should be inverse of MIN_denominator value  and it is greater than 
    * max cov_trace value for 5 detections. 
    * That's why precondition should fail in this case.
    **/
   CHECK_FALSE(f_preconditions_valid);
}

/**
*\purpose  Tests checks how function handle the situation when cluster contains
*          current detections with invalid ids.
*          Cluster  does not contain historical detections.
*\req      NA
*/
TEST(f360_nees_cfmi_preconditions, TestOnlyCurrentDetsWithInvalidIDs)
{
   /** \precond
    * Cluster contains only 3 detections from current instance.
    * All current dets have invalid ids.
    **/
   cluster_to_init.ndets = 3;
   for (int32_t inx = 0; inx < cluster_to_init.ndets; inx++)
   {
      int32_t invalid_detids = MAX_NUMBER_OF_DETECTIONS + inx + 1;
      cluster_to_init.detids[inx] = invalid_detids;
   }

   calibrations.k_nees_cfmi_extended_prec_3_dets_max_std = 1.0F;

   /** \action
    * Call NEES_CFMI_Preconditions().
    **/
   bool f_preconditions_valid = NEES_CFMI_Preconditions(calibrations, cluster_to_init, det_props, raw_detection_list, detection_hist, tracker_info);

   /** \result
    *  Current detections have invalid ids, algorithm should not use any detection for 
    *  cov_trace calculation. 
    *  That's why precondition should be invalid in this case.
    **/
   CHECK_FALSE(f_preconditions_valid);
}
/** @}*/
