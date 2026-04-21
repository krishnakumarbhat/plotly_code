/** \file
 * This file contains unit tests for content of f360_msmt_update_support_functions_cca_non_moveable.cpp file
 */

#include "f360_msmt_update_support_functions_cca_non_moveable.h"
#include "f360_trk_fltr_cca_states.h"
#include <CppUTest/TestHarness.h>

//#include "headerfile_needed.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;
constexpr uint8_t msmt_size = 3;

/** \defgroup  f360_msmt_update_support_functions_cca_non_moveable
 *  @{
 */

/** \brief
 * This test group sets up test data and expected output for Non Moveable CCA update in the general case.
 */
TEST_GROUP(f360_msmt_update_support_functions_cca_non_moveable)
{	
   uint32_t nr_dets;
   uint32_t nr_total_msnmts;
   float32_t h_mat[msmt_size][STATE_DIMENSION] = {};
   float32_t p_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t r_mat[msmt_size][msmt_size] = {};
   float32_t z_mat[msmt_size] = {};
   float32_t zhat_mat[msmt_size] = {};
   float32_t state[STATE_DIMENSION];

   float32_t s_mat[msmt_size][msmt_size] = {};
   float32_t k_mat[STATE_DIMENSION][msmt_size] = {};

   float32_t exp_s_mat[msmt_size][msmt_size] = {};
   float32_t exp_k_mat[STATE_DIMENSION][msmt_size] = {};
   float32_t exp_p_mat[STATE_DIMENSION][STATE_DIMENSION] = {};
   float32_t exp_state[STATE_DIMENSION] = {};

   float32_t rdot_comp[3]; // Must be the size of number of detections
   float32_t azimuth[3]; // Must be the size of number of detections

   const float32_t threshold = 0.0001F; // Threshold for comparing calculated and expected data
   

   F360_Calibrations_T calibrations =  {};

   /** \setup
   * Create data for a CCA update
   */
   TEST_SETUP()
   {
      // Setup arbitrary reasonable state vector
      state[F360_TRK_FLTR_CCA_STATE_X] = 10.0F;
      state[F360_TRK_FLTR_CCA_STATE_VX] = 5.0F;
      state[F360_TRK_FLTR_CCA_STATE_AX] = 0.1F;
      state[F360_TRK_FLTR_CCA_STATE_Y] = 0.0F;
      state[F360_TRK_FLTR_CCA_STATE_VY] = 2.0F;
      state[F360_TRK_FLTR_CCA_STATE_AY] = 0.1F;

      // Three detections
      nr_dets = 1U;
      azimuth[0] = F360_DEG2RAD(0.0F);
      rdot_comp[0] = state[F360_TRK_FLTR_CCA_STATE_VX] * F360_Cosf(azimuth[0]) + state[F360_TRK_FLTR_CCA_STATE_VY] * F360_Sinf(azimuth[0]) + 0.1F; // Add some value to force state update
  
                                                            
      // Set up H, Z and Zhat matrix
      //Two pseudo position measurements
      nr_total_msnmts = msmt_size;
      z_mat[0] = state[F360_TRK_FLTR_CCA_STATE_X] + 0.1F; // Pseudo position, add some value to force state update
      z_mat[1] = state[F360_TRK_FLTR_CCA_STATE_Y] + 0.1F; // Pseudo position (Yevhenii added to replicate the same thing in MATLAB file)

      h_mat[0][F360_TRK_FLTR_CCA_STATE_X] = 1.0F;
      h_mat[1][F360_TRK_FLTR_CCA_STATE_Y] = 1.0F;
      
      zhat_mat[0] = state[F360_TRK_FLTR_CCA_STATE_X];
      zhat_mat[1] = state[F360_TRK_FLTR_CCA_STATE_Y];
      for (uint32_t i = 2U; i < nr_total_msnmts; i++)
      {
         float32_t az = azimuth[i - 2];
         float32_t rdot = rdot_comp[i - 2];

         h_mat[i][F360_TRK_FLTR_CCA_STATE_VX] = F360_Cosf(az);
         h_mat[i][F360_TRK_FLTR_CCA_STATE_VY] = F360_Sinf(az);

         z_mat[i] = rdot;
         zhat_mat[i] = state[F360_TRK_FLTR_CCA_STATE_VX] * F360_Cosf(az) + state[F360_TRK_FLTR_CCA_STATE_VY] * F360_Sinf(az);
      }

      Initialize_Tracker_Calibrations(calibrations);


      // Set up arbitrary symmetric P matrix
      // Fill upper triangle

      p_mat[0][0] = 6.5169020258062931F;
      p_mat[0][1] = 0.1945124907734175F;
      p_mat[0][2] = -0.0384556107300637F;
      p_mat[0][3] = -0.3850330754532219F;
      p_mat[0][4] = 0.1285258329421008F;
      p_mat[0][5] = 0.0938525644826565F;
      p_mat[1][0] = 0.1945124907734175F;
      p_mat[1][1] = 6.6638225932428696F;
      p_mat[1][2] = 0.1043554630575398F;
      p_mat[1][3] = 0.1661297090399247F;
      p_mat[1][4] = 0.4279077173738897F;
      p_mat[1][5] = -0.1840967228468995F;
      p_mat[2][0] = -0.0384556107300637F;
      p_mat[2][1] = 0.1043554630575398F;
      p_mat[2][2] = 6.5717516167530929F;
      p_mat[2][3] = -0.0463124667591884F;
      p_mat[2][4] = -0.0474636529957669F;
      p_mat[2][5] = 0.4057486204994333F;
      p_mat[3][0] = -0.3850330754532219F;
      p_mat[3][1] = 0.1661297090399247F;
      p_mat[3][2] = -0.0463124667591884F;
      p_mat[3][3] = 6.9483854104727634F;
      p_mat[3][4] = -0.0724591341868679F;
      p_mat[3][5] = -0.4093296840042222F;
      p_mat[4][0] = 0.1285258329421008F;
      p_mat[4][1] = 0.4279077173738897F;
      p_mat[4][2] = -0.0474636529957669F;
      p_mat[4][3] = -0.0724591341868679F;
      p_mat[4][4] = 6.4492365170366686F;
      p_mat[4][5] = -0.2768349930633138F;
      p_mat[5][0] = 0.0938525644826565F;
      p_mat[5][1] = -0.1840967228468995F;
      p_mat[5][2] = 0.4057486204994333F;
      p_mat[5][3] = -0.4093296840042222F;
      p_mat[5][4] = -0.2768349930633138F;
      p_mat[5][5] = 6.9573606188351498F;



      // Set up R matrix

      r_mat[0][0] = 0.6283736714856673F;
      r_mat[0][1] = 0.3861097256837295F;
      r_mat[0][2] = 0.0000000000000000F;
      r_mat[0][3] = 0.0000000000000000F;
      r_mat[1][0] = 0.3861097256837295F;
      r_mat[1][1] = 0.2461389364340652F;
      r_mat[1][2] = 0.0000000000000000F;
      r_mat[1][3] = 0.0000000000000000F;
      r_mat[2][0] = 0.0000000000000000F;
      r_mat[2][1] = 0.0000000000000000F;
      r_mat[2][2] = 0.9029219420985610F;
      r_mat[2][3] = 0.0000000000000000F;
      r_mat[3][0] = 0.0000000000000000F;
      r_mat[3][1] = 0.0000000000000000F;
      r_mat[3][2] = 0.0000000000000000F;
      r_mat[3][3] = 0.0011865324620129F;

   }

};

/** \purpose  
 * Verify that acceleration driven by measurements is prevented when initialization 
 *
 * \req
 * NA.
 */
TEST(f360_msmt_update_support_functions_cca_non_moveable, Test_Kalman_Gain_Update_NM_CCA_Large_Num_Times_Since_Init)
{
 /** \precond
    * Set required input
    */
   const uint8_t num_updates_since_init = calibrations.k_max_num_cca_updates_since_init_to_limit_acc;
   float32_t h_mat[3][6] = {
      {1.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.0000000F, 1.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 1.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F}
   };

   float32_t p_mat[6][6] = {
      {6.5169020F, 0.1945125F, -0.0384556F, -0.3850331F, 0.1285258F, 0.0938526F},
      {0.1945125F, 6.6638226F, 0.1043555F, 0.1661297F, 0.4279077F, -0.1840967F},
      {-0.0384556F, 0.1043555F, 6.5717516F, -0.0463125F, -0.0474637F, 0.4057486F},
      {-0.3850331F, 0.1661297F, -0.0463125F, 6.9483854F, -0.0724591F, -0.4093297F},
      {0.1285258F, 0.4279077F, -0.0474637F, -0.0724591F, 6.4492365F, -0.2768350F},
      {0.0938526F, -0.1840967F, 0.4057486F, -0.4093297F, -0.2768350F, 6.9573606F}
   };

   float32_t r_mat[3][3] =  {
      {0.4839733F, 0.0348570F, 0.0000000F},
      {0.0348570F, 0.1017386F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.9000000F}
   };


   float32_t exp_s_mat[3][3] = {
      {7.0008754F, -0.3501761F, 0.1945125F},
      {-0.3501761F, 7.0501240F, 0.1661297F},
      {0.1945125F, 0.1661297F, 7.5638226F}
   };

   float32_t exp_k_mat[6][3] = {
      {0.9303921F, -0.0084481F, 0.0019756F},
      {0.0034593F, 0.0029793F, 0.8808582F},
      {-0.0062459F, -0.0072119F, 0.0141157F},
      {-0.0057286F, 0.9852736F, 0.0004708F},
      {0.0162516F, -0.0107993F, 0.0563922F},
      {0.0112066F, -0.0569525F, -0.0233764F}
   };


   /** \action
    * Call function
    */
   Kalman_Gain_Update_CCA_Non_Moveable(
      h_mat,
      p_mat,
      r_mat,
      calibrations,
      num_updates_since_init,
      k_mat,
      s_mat);


/** \result
   * Compare computed against expected data
   */

   // S-matrix
   for (uint32_t i = 0U; i < nr_total_msnmts; i++)
   {
      for (uint32_t j = 0U; j < nr_total_msnmts; j++)
      {
         DOUBLES_EQUAL(s_mat[i][j], exp_s_mat[i][j], threshold);
      }
   }

   // K matrix
   for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
   {
      for (uint32_t j = 0U; j < nr_total_msnmts; j++)
      {
         DOUBLES_EQUAL(k_mat[i][j], exp_k_mat[i][j], threshold);
      }
   }

}

/** \purpose  
 * Verify that acceleration driven by measurements is not allowed when object is newly initialized, and the relate K Gain will be set to 0 
 *
 * \req
 * NA.
 */
TEST(f360_msmt_update_support_functions_cca_non_moveable, Test_Kalman_Gain_Update_CCA_Small_Num_Times_Since_Init)
{
 /** \precond
    * Set num_updates_since_init to 0
    */
   const uint8_t num_updates_since_init = 0;
   /** \action
    * Call function
    */
   Kalman_Gain_Update_CCA_Non_Moveable(
      h_mat,
      p_mat,
      r_mat,
      calibrations,
      num_updates_since_init,
      k_mat,
      s_mat);

   /** \result
   * Compare computed against expected data
   */
   // K matrix

   for (uint32_t j = 0U; j < nr_total_msnmts; j++)
   {
      DOUBLES_EQUAL(0.0F, k_mat[F360_TRK_FLTR_CCA_STATE_AX][j], threshold);
   }

}


/** \purpose
* Verify that error covariance matrix is calculated as expected.
* \req
* NA
*/
TEST(f360_msmt_update_support_functions_cca_non_moveable, Test_Error_Cov_Update_NM_CCA)
{
   /** \precond
    */
   float32_t k_mat[6][3] = {
      {0.9303921F, -0.0084481F, 0.0019756F},
      {0.0034593F, 0.0029793F, 0.8808582F},
      {-0.0062459F, -0.0072119F, 0.0141157F},
      {-0.0057286F, 0.9852736F, 0.0004708F},
      {0.0162516F, -0.0107993F, 0.0563922F},
      {0.0112066F, -0.0569525F, -0.0233764F}
   };

   float32_t r_mat[3][3] =  {
      {0.4839733F, 0.0348570F, 0.0000000F},
      {0.0348570F, 0.1017386F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.9000000F}
   };

   float32_t h_mat[3][6] = {
      {1.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 0.0000000F, 0.0000000F, 1.0000000F, 0.0000000F, 0.0000000F},
      {0.0000000F, 1.0000000F, 0.0000000F, 0.0000000F, 0.0000000F, 0.0000000F}
   };

   float32_t exp_p_mat[6][6] = {
      {0.4499905F, 0.0017780F, -0.0032742F, 0.0315711F, 0.0074889F, 0.0034385F},
      {0.0017780F, 0.7927723F, 0.0127041F, 0.0004237F, 0.0507530F, -0.0210388F},
      {-0.0032742F, 0.0127041F, 6.5697044F, -0.0009514F, -0.0532237F, 0.4059814F},
      {0.0315711F, 0.0004237F, -0.0009514F, 0.1000407F, -0.0005322F, -0.0054036F},
      {0.0074889F, 0.0507530F, -0.0532237F, -0.0005322F, 6.4222346F, -0.2723991F},
      {0.0034385F, -0.0210388F, 0.4059814F, -0.0054036F, -0.2723991F, 6.9286930F}
   };



   /** \action
   * Call function
   */
   Error_Cov_Update_CCA_Non_Moveable(
      k_mat,
      r_mat,
      h_mat,
      p_mat);

   /** \result
   * Compare computed P matrix against expected data
   */
   for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
   {
      for (uint32_t j = 0U; j < STATE_DIMENSION; j++)
      {
         DOUBLES_EQUAL(exp_p_mat[i][j], p_mat[i][j], threshold);
      }
   }
}

/** \purpose
* Verify that state vector is calculated as expected.
* \req
* NA
*/
TEST(f360_msmt_update_support_functions_cca_non_moveable, Test_State_Update_NM_CCA)
{
   
   /** \precond
    * 
    */
   float32_t k_mat[6][3] = {
      {0.9303921F, -0.0084481F, 0.0019756F},
      {0.0034593F, 0.0029793F, 0.8808582F},
      {-0.0062459F, -0.0072119F, 0.0141157F},
      {-0.0057286F, 0.9852736F, 0.0004708F},
      {0.0162516F, -0.0107993F, 0.0563922F},
      {0.0112066F, -0.0569525F, -0.0233764F}
   };

   float32_t exp_state[6] = {
      9.9890181F,
      0.5957051F,
      0.0294290F,
      -0.0023470F,
      1.7180197F,
      0.2168688F
   };
      
   /** \action
   * Call function
   */
   State_Update_CCA_Non_Moveable(
      z_mat,
      zhat_mat,
      k_mat,
      state);

   /** \result
   * Compare computed State vector against expected data
   */
   for (uint32_t i = 0U; i < STATE_DIMENSION; i++)
   {
      DOUBLES_EQUAL(exp_state[i], state[i], threshold);
   }
}
/** @}*/
