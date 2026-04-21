/** \file
 * This file contains unit tests for content of f360_msmt_update_obj_trks_cca_moveable.cpp file
 */

#include "f360_msmt_update_obj_trks_cca_moveable.h"
#include <CppUTest/TestHarness.h>

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

/** \defgroup  f360_msmt_update_obj_trks_cca_moveable
 *  @{
 */

/** \brief
 * This test group is for testing the function Msmt_Update_Obj_Trks_CCA_Moveable() which does a KF measurement updaate of a CCA object.
 */
TEST_GROUP(f360_msmt_update_obj_trks_cca_moveable)
{
   F360_Detection_Props_T det_props[MAX_NUMBER_OF_DETECTIONS];
   rspp_variant_A::RSPP_Detection_List_T raw_detection_list;
      F360_Calibrations_T calib;
   uint32_t selected_dets_idx[MAX_DETS_IN_OBJ_TRK];
   uint32_t selected_dets_num;
   F360_Object_Track_T object_track;
   F360_TRKR_TIMING_INFO_T timing_info;

   const float32_t test_pass_th = 1e-6F;
   const float32_t test_pass_acc_th = 1e-3F;

   /** \setup
    * Setup general values for object and detections that are used in the default test case.
   *    Calibrations
   *       - Default tracker calibrations
   *    Object:
   *       - trk_flitr_type: CCA
   *       - vcs_position: [10, 0] [m]
   *       - vcs_velocity: [5, 2] [m/s]
   *       - speed: length of the vcs_velocity vector
   *       - vcs_heading: direction of vcs_velocity
   *       - vcs_accel: [0.1, 0.1] [m/s^2]
   *       - tang_accel: scalar product of vcs_accel with unit vector in the same direction as vcs_velocity
   *       - bbox orientation: vcs_heading + 5deg
   *       - yaw_rate: -0.1F [rad/s]
   *       - curvature: yaw_rate / speed
   *       - errcov: random covariance matrix [6.516902025806293,   0.194512490773417,  -0.038455610730064,  -0.385033075453222,   0.128525832942101   0.093852564482656;
   *                                           0.194512490773417,   6.663822593242870,   0.104355463057540,   0.166129709039925,   0.427907717373890,  -0.184096722846899;
   *                                          -0.038455610730064,   0.104355463057540,   6.571751616753093,  -0.046312466759188,  -0.047463652995767,   0.405748620499433;
   *                                          -0.385033075453222,   0.166129709039925,  -0.046312466759188,   6.948385410472763,  -0.072459134186868,  -0.409329684004222;
   *                                           0.128525832942101,   0.427907717373890,  -0.047463652995767,  -0.072459134186868,   6.449236517036669,  -0.276834993063314;
   *                                           0.093852564482656,  -0.184096722846899,   0.405748620499433,  -0.409329684004222,  -0.276834993063314,   6.957360618835150]
   *       - cca_pnt_filter_cov: random covariance matrix [2.696766889080332, 1.305369243191352; 1.305369243191352 1.395963987598590]
   *       - pseudo_vcs_position: vcs_position + [0.1, 0.1] [m] (i.e object position + some noise)
   *       - meascov: random covariance matrix: [0.628373671485667, 0.386109725683730; 0.386109725683730,  0.246138936434065]
   *       - num_updates_since_init: a large value (such that the kalman gain is not modified), 255
   *       - f_moving: true
   *       - ndets: 3
   *    Detection 1:
   *       - idx: 3
   *       - vcs_az: 0deg
   *       - r_compensated: predicted range rate + 0.1
   *    Detection 2:
   *       - idx: 17
   *       - vcs_az: 10deg
   *       - r_compensated: predicted range rate
   *    Detection 3:
   *       - idx: 55
   *       - vcs_az: 5deg
   *       - r_compensated: predicted range rate - 0.1
   *    Selected detections
   *      - all three of the above measurements
   */
   TEST_SETUP()
   {
      // Calibrations
      Initialize_Tracker_Calibrations(calib);

      // Object
      object_track.trk_fltr_type = F360_TRACKER_TRKFLTR_CCA;

      object_track.vcs_position.x = 10.0F;
      object_track.vcs_position.y = 0.0F;

      object_track.vcs_velocity.longitudinal = 5.0F;
      object_track.vcs_velocity.lateral = 2.0F;
      object_track.speed = F360_Get_Hypotenuse(object_track.vcs_velocity.longitudinal, object_track.vcs_velocity.lateral);
      (void)object_track.vcs_heading.Value(F360_Atan2f(object_track.vcs_velocity.lateral, object_track.vcs_velocity.longitudinal)).Normalize();
      
      object_track.vcs_accel.longitudinal = 0.1F;
      object_track.vcs_accel.lateral = 0.1F;
      object_track.tang_accel = object_track.vcs_accel.longitudinal * object_track.vcs_heading.Cos() + object_track.vcs_accel.lateral * object_track.vcs_heading.Sin();

      object_track.bbox.Set_Orientation(object_track.vcs_heading.Value() + F360_DEG2RAD(5.0F));
      object_track.heading_rate = -0.1F;
      object_track.curvature = object_track.heading_rate / object_track.speed;

      object_track.errcov[0][0] = 6.5169020F;
      object_track.errcov[0][1] = 0.1945125F;
      object_track.errcov[0][2] = -0.0384556F;
      object_track.errcov[0][3] = -0.3850331F;
      object_track.errcov[0][4] = 0.1285258F;
      object_track.errcov[0][5] = 0.0938526F;

      object_track.errcov[1][0] = 0.1945125F;
      object_track.errcov[1][1] = 6.6638226F;
      object_track.errcov[1][2] = 0.1043555F;
      object_track.errcov[1][3] = 0.1661297F;
      object_track.errcov[1][4] = 0.4279077F;
      object_track.errcov[1][5] = -0.1840967F;

      object_track.errcov[2][0] = -0.0384556F;
      object_track.errcov[2][1] = 0.1043555F;
      object_track.errcov[2][2] = 6.5717516F;
      object_track.errcov[2][3] = -0.0463125F;
      object_track.errcov[2][4] = -0.0474637F;
      object_track.errcov[2][5] = 0.4057486F;

      object_track.errcov[3][0] = -0.3850331F;
      object_track.errcov[3][1] = 0.1661297F;
      object_track.errcov[3][2] = -0.0463125F;
      object_track.errcov[3][3] = 6.9483854F;
      object_track.errcov[3][4] = -0.0724591F;
      object_track.errcov[3][5] = -0.4093297F;

      object_track.errcov[4][0] = 0.1285258F;
      object_track.errcov[4][1] = 0.4279077F;
      object_track.errcov[4][2] = -0.0474637F;
      object_track.errcov[4][3] = -0.0724591F;
      object_track.errcov[4][4] = 6.4492365F;
      object_track.errcov[4][5] = -0.2768350F;

      object_track.errcov[5][0] = 0.0938526F;
      object_track.errcov[5][1] = -0.1840967F;
      object_track.errcov[5][2] = 0.4057486F;
      object_track.errcov[5][3] = -0.4093297F;
      object_track.errcov[5][4] = -0.2768350F;
      object_track.errcov[5][5] = 6.9573606F;

      object_track.cca_pnt_filter_cov[0][0] = 2.6967668F;
      object_track.cca_pnt_filter_cov[0][1] = 1.3053692F;
      object_track.cca_pnt_filter_cov[1][0] = object_track.cca_pnt_filter_cov[0][1];
      object_track.cca_pnt_filter_cov[0][0] = 1.3959639F;

      object_track.pseudo_vcs_position.x = object_track.vcs_position.x + 0.1F;
      object_track.pseudo_vcs_position.y = object_track.vcs_position.y + 0.1F;

      object_track.meascov[0][0] = 0.6283736F;
      object_track.meascov[0][1] = 0.3861097F;
      object_track.meascov[1][0] = object_track.meascov[0][1];
      object_track.meascov[1][1] = 0.2461389F;

      object_track.num_updates_since_init = 255U;

      object_track.f_moving = true;

      object_track.ndets = 3U;

      // Detection 1
      uint32_t idx = 3;
      raw_detection_list.detections[idx].processed.vcs_az = 0.0F;
      raw_detection_list.detections[idx].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[idx].processed.vcs_az);
      raw_detection_list.detections[idx].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[idx].processed.vcs_az);
      det_props[idx].range_rate_compensated = object_track.vcs_velocity.longitudinal * raw_detection_list.detections[idx].processed.cos_vcs_az + object_track.vcs_velocity.lateral * raw_detection_list.detections[idx].processed.sin_vcs_az  + 0.1F;

      // Detection 2
      idx = 17;
      raw_detection_list.detections[idx].processed.vcs_az = F360_DEG2RAD(10.0F);
      raw_detection_list.detections[idx].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[idx].processed.vcs_az);
      raw_detection_list.detections[idx].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[idx].processed.vcs_az);
      det_props[idx].range_rate_compensated = object_track.vcs_velocity.longitudinal * raw_detection_list.detections[idx].processed.cos_vcs_az + object_track.vcs_velocity.lateral * raw_detection_list.detections[idx].processed.sin_vcs_az;

      // Detection 3
      idx = 55;
      raw_detection_list.detections[idx].processed.vcs_az = F360_DEG2RAD(5.0F);
      raw_detection_list.detections[idx].processed.cos_vcs_az = F360_Cosf(raw_detection_list.detections[idx].processed.vcs_az);
      raw_detection_list.detections[idx].processed.sin_vcs_az = F360_Sinf(raw_detection_list.detections[idx].processed.vcs_az);
      det_props[idx].range_rate_compensated = object_track.vcs_velocity.longitudinal * raw_detection_list.detections[idx].processed.cos_vcs_az + object_track.vcs_velocity.lateral * raw_detection_list.detections[idx].processed.sin_vcs_az - 0.1F;
   
      // Selected detections
      selected_dets_idx[0] = 3;
      selected_dets_idx[1] = 17;
      selected_dets_idx[2] = 55;
      selected_dets_num = 3U;

   }
};

/** \purpose  
 * Verify that no measurement update is done when there are no associated eligable detections
 * \req
 * NA
 */
TEST(f360_msmt_update_obj_trks_cca_moveable, Msmt_Update_Obj_Trks_CCA_No_Dets)
{
   /** \precond
    * Use default setup form test group except for:
    *    - Set number of selected detections for msmnt update to zero.
    * Copy object before call to function to be alble to compare afterwards that object is unchanged.
    */
   selected_dets_num = 0U;
   const F360_Object_Track_T copy_obj = object_track;

   /** \action
    * Call function Msmt_Update_Obj_Trks_CCA_Moveable()
    */
   Msmt_Update_Obj_Trks_CCA_Moveable(det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
    * Verify that object states haven't changed
    */
   DOUBLES_EQUAL(copy_obj.vcs_position.x, object_track.vcs_position.x, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_velocity.longitudinal, object_track.vcs_velocity.longitudinal, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_accel.longitudinal, object_track.vcs_accel.longitudinal, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_position.y, object_track.vcs_position.y, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_velocity.lateral, object_track.vcs_velocity.lateral, F360_EPSILON);
   DOUBLES_EQUAL(copy_obj.vcs_accel.lateral, object_track.vcs_accel.lateral, F360_EPSILON);
   for(uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx ++)
   {
      for(uint32_t col_idx = 0U; col_idx < STATE_DIMENSION; col_idx ++)
      {
         DOUBLES_EQUAL(copy_obj.errcov[row_idx][col_idx], object_track.errcov[row_idx][col_idx], F360_EPSILON);
      }
   }
}


/** \purpose
 * Verify that object states are updated correctly when there are associated eligable detections
 * \req NA
 */
TEST(f360_msmt_update_obj_trks_cca_moveable, Msmt_Update_Obj_Trks_CCA_default)
{
   /** \precond
    * Use default setup from test group
    */

   /** \action
    * Call function Msmt_Update_Obj_Trks_CCA_Moveable()
    * Extract object pointing before function call so that it is possible to compare with afterwards.
    */
   float32_t pnt_before = object_track.bbox.Get_Orientation().Value();
   Msmt_Update_Obj_Trks_CCA_Moveable(det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
    * Expected output is:
    *    - vcs_position: [10.085811428468116, 0.091158385978730]
    *    - vcs_velocity: [5.002059334855990, 1.980379592815318]
    *    - speed: length of updated vcs_velocity vector
    *    - vcs_heading: direction of updated vcs_velocity vector
    *    - vcs_accel: [0.098941016521831, 0.096558496272075]
    *    - tang_accel: scalar product of updated vcs_accel with unit vector in the same direction as updated vcs_velocity
    *    - errcov: [0.552309098094765, 0.002822719097295, -0.006220049282493, 0.338893491038036, 0.003852976625816, -0.013003917964762;
    *               0.002822719097295, 0.829903546572940, 0.017245996483160, 0.001767580812805, -0.418050182876877, -0.002183527111271;
    *              -0.006220049282493, 0.017245996483160, 6.569864014276908, -0.003881852642069, -0.058291180241595, 0.406220628539916;
    *               0.338893491038036, 0.001767580812805, -0.003881852642069, 0.216821994650829, 0.002256556717415, -0.008491185474361;
    *               0.003852976625816, -0.418050182876877, -0.058291180241595, 0.002256556717415, 6.100872837580059, -0.247858432974155;
    *              -0.013003917964762, -0.002183527111271, 0.406220628539916, -0.008491185474361, -0.247858432974155, 6.927180503823093F]
    * 
    *    - pointing yaw rate filter has run => pointing has changed
    *    - curvature: updated yaw_rate / updated speed
    */
   const float32_t exp_vcs_position_x = 10.0858114F;
   const float32_t exp_vcs_position_y = 0.0911583F;
   const float32_t exp_vcs_vel_x = 5.00205933F;
   const float32_t exp_vcs_vel_y = 1.9803795F;
   const float32_t exp_speed = F360_Get_Hypotenuse(exp_vcs_vel_x, exp_vcs_vel_y);
   const float32_t exp_vcs_hdg = F360_Atan2f(exp_vcs_vel_y, exp_vcs_vel_x);
   const float32_t exp_vcs_accel_x = 0.0989410F;
   const float32_t exp_vcs_accel_y = 0.0965584F;
   const float32_t exp_tang_accel = exp_vcs_accel_x * F360_Cosf(exp_vcs_hdg) + exp_vcs_accel_y * F360_Sinf(exp_vcs_hdg);
   const float32_t exp_curv = object_track.heading_rate / object_track.speed;
   const float32_t exp_errcov[STATE_DIMENSION][STATE_DIMENSION] = {{0.5523090F, 0.0028227F, -0.0062200F, 0.3388934F, 0.0038529F, -0.0130039F},
                                                                  {0.0028227F, 0.8299035F, 0.0172459F, 0.0017675F, -0.4180501F, -0.0021835F},
                                                                  {-0.0062200F, 0.0172459F, 6.5698640F, -0.0038818F, -0.0582911F, 0.4062206F},
                                                                  {0.3388934F, 0.0017675F, -0.0038818F, 0.2168219F, 0.0022565F, -0.0084911F},
                                                                  {0.0038529F, -0.4180501F, -0.0582911F, 0.0022565F, 6.1008728F, -0.2478584F},
                                                                  {-0.0130039F, -0.0021835F, 0.4062206F, -0.0084911F, -0.2478584F, 6.9271805F}};



   DOUBLES_EQUAL_TEXT(exp_vcs_position_x, object_track.vcs_position.x, test_pass_th, "VCS x postion is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_position_y, object_track.vcs_position.y, test_pass_th, "VCS y postion is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_vel_x, object_track.vcs_velocity.longitudinal, test_pass_th, "VCS x velocity is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_vel_y, object_track.vcs_velocity.lateral, test_pass_th, "VCS y velocity is unexpected");
   DOUBLES_EQUAL_TEXT(exp_speed, object_track.speed, test_pass_th, "Speed is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_hdg, object_track.vcs_heading.Value(), test_pass_th, "VCS heading is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_accel_x, object_track.vcs_accel.longitudinal, test_pass_th, "VCS x acceleration is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_accel_y, object_track.vcs_accel.lateral, test_pass_th, "VCS y acceleration is unexpected");
   DOUBLES_EQUAL_TEXT(exp_tang_accel, object_track.tang_accel, test_pass_acc_th, "Tangential acceleration is unexpected");
   CHECK_TRUE_TEXT(std::abs(pnt_before - object_track.bbox.Get_Orientation().Value()) > test_pass_th, "Object pointing has not been updated");
   DOUBLES_EQUAL_TEXT(exp_curv, object_track.curvature, test_pass_th, "Curvature is unexpected");
   for (uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx ++)
   {
      for (uint32_t col_idx = 0U; col_idx < STATE_DIMENSION; col_idx ++)
      {
         DOUBLES_EQUAL_TEXT(exp_errcov[row_idx][col_idx], object_track.errcov[row_idx][col_idx], test_pass_th, "Errcov is unexpected");
      }
   }
}


/** \purpose
 * Verify that object states are updated correctly when there are associated eligable detections  but the object is stationary
 * \req NA
 */
TEST(f360_msmt_update_obj_trks_cca_moveable, Msmt_Update_Obj_Trks_CCA_stationary)
{
   /** \precond
    * Use default setup from test group except for
    *    - object f_moving is set to false
    */
   object_track.f_moving = false;

   /** \action
    * Call function Msmt_Update_Obj_Trks_CCA_Moveable()
    * Extract object pointing and curvature before function call so that it is possible to compare with afterwards.
    */
   float32_t pnt_before = object_track.bbox.Get_Orientation().Value();
   float32_t hdg_before = object_track.vcs_heading.Value();
   Msmt_Update_Obj_Trks_CCA_Moveable(det_props, raw_detection_list, calib, selected_dets_idx, selected_dets_num, object_track, timing_info);

   /** \result
    * Expected output is (note this is very similar to in Msmt_Update_Obj_Trks_CCA_default):
    *    - vcs_position: [10.085811428468116, 0.091158385978730]
    *    - vcs_velocity: [5.002059334855990, 1.980379592815318]
    *    - speed: length of updated vcs_velocity vector
    *    - vcs_heading: not updated
    *    - vcs_accel: [0.098941016521831, 0.096558496272075]
    *    - tang_accel: scalar product of updated vcs_accel with unit vector in the same direction as updated vcs_velocity
    *    - errcov: [0.552309098094765, 0.002822719097295, -0.006220049282493, 0.338893491038036, 0.003852976625816, -0.013003917964762;
    *               0.002822719097295, 0.829903546572940, 0.017245996483160, 0.001767580812805, -0.418050182876877, -0.002183527111271;
    *              -0.006220049282493, 0.017245996483160, 6.569864014276908, -0.003881852642069, -0.058291180241595, 0.406220628539916;
    *               0.338893491038036, 0.001767580812805, -0.003881852642069, 0.216821994650829, 0.002256556717415, -0.008491185474361;
    *               0.003852976625816, -0.418050182876877, -0.058291180241595, 0.002256556717415, 6.100872837580059, -0.247858432974155;
    *              -0.013003917964762, -0.002183527111271, 0.406220628539916, -0.008491185474361, -0.247858432974155, 6.927180503823093F]
    * 
    *    - pointing yaw rate filter has not run => pointing is unchanged
    *    - yaw_rate: 0
    *    - curvature: not updated
    */
   const float32_t exp_vcs_position_x = 10.085811428468116F;
   const float32_t exp_vcs_position_y = 0.091158385978730F;
   const float32_t exp_vcs_vel_x = 5.002059334855990F;
   const float32_t exp_vcs_vel_y = 1.980379592815318F;
   const float32_t exp_speed = F360_Get_Hypotenuse(exp_vcs_vel_x, exp_vcs_vel_y);
   const float32_t exp_vcs_accel_x = 0.0989410F;
   const float32_t exp_vcs_accel_y = 0.0965584F;
   const float32_t exp_tang_accel = exp_vcs_accel_x * F360_Cosf(hdg_before) + exp_vcs_accel_y * F360_Sinf(hdg_before);
   const float32_t exp_yaw_rate = 0.0F;
   const float32_t exp_curv = 0.0F;
   Angle exp_hdg;
   exp_hdg.Value(F360_Atan2f(exp_vcs_vel_y, exp_vcs_vel_x)).Normalize();
   const float32_t exp_errcov[STATE_DIMENSION][STATE_DIMENSION] = {{0.5523090F, 0.0028227F, -0.0062200F, 0.3388934F, 0.0038529F, -0.0130039F},
                                                                  {0.0028227F, 0.8299035F, 0.0172459F, 0.0017675F, -0.4180501F, -0.0021835F},
                                                                  {-0.0062200F, 0.0172459F, 6.5698640F, -0.0038818F, -0.0582911F, 0.4062206F},
                                                                  {0.3388934F, 0.0017675F, -0.0038818F, 0.2168219F, 0.0022565F, -0.0084911F},
                                                                  {0.0038529F, -0.4180501F, -0.0582911F, 0.0022565F, 6.1008728F, -0.2478584F},
                                                                  {-0.0130039F, -0.0021835F, 0.4062206F, -0.0084911F, -0.2478584F, 6.9271805F}};


   DOUBLES_EQUAL_TEXT(exp_vcs_position_x, object_track.vcs_position.x, test_pass_th, "VCS x postion is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_position_y, object_track.vcs_position.y, test_pass_th, "VCS y postion is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_vel_x, object_track.vcs_velocity.longitudinal, test_pass_th, "VCS x velocity is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_vel_y, object_track.vcs_velocity.lateral, test_pass_th, "VCS y velocity is unexpected");
   DOUBLES_EQUAL_TEXT(exp_speed, object_track.speed, test_pass_th, "Speed is unexpected");
   DOUBLES_EQUAL_TEXT(exp_hdg.Value(), object_track.vcs_heading.Value(), test_pass_th, "VCS heading is unexpectedly updated");
   DOUBLES_EQUAL_TEXT(exp_vcs_accel_x, object_track.vcs_accel.longitudinal, test_pass_th, "VCS x acceleration is unexpected");
   DOUBLES_EQUAL_TEXT(exp_vcs_accel_y, object_track.vcs_accel.lateral, test_pass_th, "VCS y acceleration is unexpected");
   DOUBLES_EQUAL_TEXT(exp_tang_accel, object_track.tang_accel, test_pass_th, "Tangential acceleration is unexpected");
   DOUBLES_EQUAL_TEXT(pnt_before, object_track.bbox.Get_Orientation().Value(), F360_EPSILON, "Object pointing is unexpectedly updated");
   DOUBLES_EQUAL_TEXT(exp_yaw_rate, object_track.heading_rate, F360_EPSILON, "Object yaw_rate is not zero");
   DOUBLES_EQUAL_TEXT(exp_curv, object_track.curvature, test_pass_th, "Curvature is unexpectedly updated");
   for (uint32_t row_idx = 0U; row_idx < STATE_DIMENSION; row_idx ++)
   {
      for (uint32_t col_idx = 0U; col_idx < STATE_DIMENSION; col_idx ++)
      {
         DOUBLES_EQUAL_TEXT(exp_errcov[row_idx][col_idx], object_track.errcov[row_idx][col_idx], test_pass_th, "Errcov is unexpected");
      }
   }

}
/** @}*/