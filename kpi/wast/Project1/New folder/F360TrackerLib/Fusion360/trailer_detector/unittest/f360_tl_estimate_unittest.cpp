/** \file
 * This file contains unit tests for content of tl_estimate.cpp file
 */
#include "f360_trailer_detector_TL.h"
#include <CppUTest/TestHarness.h>

//#include "headerfile_needed.h"

// Unit testing guidelines: https://confluence.asux.aptiv.com/display/F360Core/Unit+testing+guidelines

using namespace f360_variant_A;

// Mock Trailer_Detector_TL in order to be able to access private members to make unit testing easier
class Trailer_Detector_TLmock : public Trailer_Detector_TL
{
   public:

   void call_norm_detection_row(const int32_t (&detection_row)[ROW_NUMBER], float32_t (&norm_sample)[ROW_NUMBER])
   {
      Trailer_Detector_TL::Norm_Detection_Row(detection_row, norm_sample);
   }

   void call_SVM_classification(const float32_t (&norm_sample)[ROW_NUMBER], int32_t& predict_class, float32_t& max_score)
   {
      Trailer_Detector_TL::SVM_Classification(norm_sample, predict_class, max_score);
   }

   void set_trailer_detection_row_struct(TL_Trailer_Detection_Row trailer_detection_row_struct_set_val)
   {
      trailer_detection_row_struct = trailer_detection_row_struct_set_val;
   }

   void set_window_timer(uint32_t windor_timer_set_val)
   {
      window_timer = windor_timer_set_val;
   }

   void call_estimate()
   {
      Trailer_Detector_TL::Estimate();
   }

   bool get_f_estimation_done()
   {
      return f_estimation_done;
   }

   void set_f_estimation_done(bool f_estimation_done_set_val)
   {
      f_estimation_done = f_estimation_done_set_val;
   }

   float32_t trailer_length;
   float32_t axel_trailer_length;
   
};

/** \defgroup  tl_SVM_classification
 *  @{
 */

/** \brief
 * Tests that Norm_Detection_Row works as intended
 */
TEST_GROUP(f360_SVM_classification)
{	
   // Create an instance of the Trailer_Detector_TP class
   Trailer_Detector_TLmock trailer_detector_TL;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-9F;

   // Define predict class default state and zero the max_score
   int32_t predict_class = -1;
   float32_t max_score = 0.0F;
   float32_t trailer_len = 4.0F;
   float32_t axel_trailer_len = 2.9F;

   // SVM model parameters (from f360_trailer_detector_TL_SVM.h)
   const float32_t length_array[CLASS_NUMBER] = {4.0F, 4.5F, 5.5F, 5.5F, 5.5F, 6.0F, 8.0F, 7.5F, 12.0F, 5.0F, 9.0F, 4.0F, 4.5F, 4.5F, 6.0F, 5.0F, 4.5F, 8.0F, 12.0F, 6.0F};
   const float32_t axel_length_array[CLASS_NUMBER] = { 2.9F, 3.15F, 3.85F, 3.85F, 3.85F, 4.2F, 5.6F, 5.25F, 8.4F, 3.5F, 6.3F, 2.9F, 3.15F, 3.15F, 4.2F, 3.5F, 3.15F, 5.6F, 8.4F, 4.2F };
 

   /** \setup
    * Initialize the Trailer_Detector_TP class instance with some default values
    */
   TEST_SETUP()
   {
      trailer_detector_TL.Initialize();
   }
};

/** \purpose  
 * Test that Norm_Detection_Row correctly normalizes the input array.
 */
TEST(f360_SVM_classification, Test_norm_detection_row_normal_input)
{
   /** \precond
    * Set up a detection row with non-zero elements in 4 bins.
    * Fill expected output array with normalized values.
    */
   float32_t norm_sample[ROW_NUMBER] = {};
   int32_t detection_row[ROW_NUMBER] = {};
   detection_row[0U] = 2;
   detection_row[4U] = 1;
   detection_row[26] = 4;
   detection_row[ROW_NUMBER - 1] = 5;

   float32_t exp_norm_sample[ROW_NUMBER] = {}; 
   exp_norm_sample[0U] = 0.294883912309794F;
   exp_norm_sample[4U] = 0.147441956154897;
   exp_norm_sample[26] = 0.589767824619589;
   exp_norm_sample[ROW_NUMBER - 1] = 0.737209780774486;
	
   /** \action
    * Call Trailer_Detector_TL::Norm_Detection_Row() from the mocked class.
    */
   trailer_detector_TL.call_norm_detection_row(detection_row, norm_sample);

   /** \result
    * Check that all elements of norm_sample are filled correctly.
    */
   for (uint32_t i = 0U; i < ROW_NUMBER; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_norm_sample[i], norm_sample[i], test_pass_th, "all elements of norm_sample are not filled correctly.")
   }	
}

/** \purpose  
 * Test that Norm_Detection_Row correctly handles an input array when all elements are zero.
 */
TEST(f360_SVM_classification, Test_norm_detection_row_empty_input_array)
{
   /** \precond
    * Set up a detection row with only zeros.
    * Fill expected output array with zeros.
    */
   float32_t norm_sample[ROW_NUMBER] = {};
   int32_t detection_row[ROW_NUMBER] = {};

   float32_t exp_norm_sample[ROW_NUMBER] = {}; 

   /** \action
    * Call Trailer_Detector_TL::Norm_Detection_Row() from the mocked class.
    */
   trailer_detector_TL.call_norm_detection_row(detection_row, norm_sample);

   /** \result
    * Check that all elements of norm_sample are filled correctly.
    */
   for (uint32_t i = 0U; i < ROW_NUMBER; i++)
   {
      DOUBLES_EQUAL_TEXT(exp_norm_sample[i], norm_sample[i], test_pass_th, "all elements of norm_sample are not filled correctly.")
   }	
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 2 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_vnose)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 2
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */
   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.0911948681, 0.994114399, 0.000451459753, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00902919471, 0.0446945168, 0.0365682393, 0.00180583901, 0.000451459753, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000 };
   int32_t exp_class_idx = 2;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
   
   	
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 18 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_utility)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 18
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */
   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.0122404806, 0.713773012, 0.166394040, 0.0244809613, 0.000765030039, 0.000382515020, 0.000765030039, 0.0325137749, 0.117432110, 0.0221858714, 0.0221858714, 0.0103279054, 0.00344263529, 0.00726778526, 0.271585673, 0.0107104201, 0.000765030039, 0.00382515020, 0.000765030039, 0.00191257510, 0.000765030039, 0.00229509012, 0.00229509012, 0.00000000, 0.00573772518, 0.0673226416, 0.507597446, 0.299891770, 0.0665576160, 0.0413116217, 0.0218033567, 0.0221858714, 0.0634974912, 0.0700002462, 0.0126229953, 0.00956287514, 0.0114754504, 0.00497269537, 0.00726778526, 0.0229509007, 0.0401640758, 0.0191257503, 0.00114754506, 0.00114754506, 0.000382515020, 0.00497269537, 0.0248634759, 0.00306012016, 0.00267760502, 0.00918036047, 0.0110929357, 0.0103279054, 0.00535521004, 0.00459018024 };

   int32_t exp_class_idx = 18;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
   
   	
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 12 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_jetski)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 12
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */
   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.000502012263, 0.00301207369, 0.318275779, 0.135041296, 0.0170684177, 0.0466871411, 0.0717877522, 0.0727917776, 0.00000000, 0.0100402450, 0.0346388444, 0.620487154, 0.692776918, 0.0346388444, 0.00803219620, 0.00251006125, 0.00401609810, 0.000502012263, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000 };
   int32_t exp_class_idx = 12;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
   
   	
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 8 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_horse)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 8
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */
   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00306748366, 0.704644799, 0.323400408, 0.0271691419, 0.00000000, 0.00744960317, 0.153374180, 0.434268057, 0.00569675537, 0.00613496732, 0.0153374188, 0.00219105976, 0.00306748366, 0.184049025, 0.0955302045, 0.0499561615, 0.0674846396, 0.0442594066, 0.0648553669, 0.233128756, 0.155565247, 0.0613496751, 0.0368098021, 0.147677422, 0.139789611, 0.0113935107, 0.00525854342, 0.00394390756, 0.00525854342, 0.00876423903, 0.00131463585, 0.00131463585, 0.00394390756, 0.000438211951, 0.000438211951, 0.000438211951, 0.00350569561, 0.00525854342, 0.00657317927, 0.0127081461, 0.0258545056, 0.0113935107, 0.0113935107, 0.0350569561, 0.0205959622, 0.00613496732, 0.00131463585, 0.00131463585, 0.00262927171, 0.00701139122, 0.0271691419, 0.0210341737, 0.00394390756, 0.00131463585, 0.00131463585 };
   int32_t exp_class_idx = 8;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
   
   	
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 9 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_clamshell)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 9
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */
   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.0255685542, 0.905803680, 0.374504119, 0.0323367007, 0.0131602855, 0.00488810614, 0.0195524246, 0.189132109, 0.0334647261, 0.00639213854, 0.00413608970, 0.00300806528, 0.00188004086, 0.00338407350, 0.00263205706, 0.00263205706, 0.000752016320, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.000376008160, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.000376008160, 0.00000000, 0.00000000, 0.000376008160, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000 };
   int32_t exp_class_idx = 9;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
   
   	
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 4 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_box)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 4
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */
   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.199623674, 0.713844121, 0.0501164906, 0.0109498212, 0.00547491061, 0.00421146955, 0.0109498212, 0.0303225815, 0.0240053777, 0.120448038, 0.0193727612, 0.0412724055, 0.0240053777, 0.0113709681, 0.00421146955, 0.0164247323, 0.0446415804, 0.185304672, 0.409776002, 0.0332706124, 0.0122132627, 0.00294802897, 0.00463261688, 0.00168458791, 0.0155824386, 0.0450627282, 0.212679222, 0.0699103996, 0.0277957004, 0.00800179224, 0.00463261688, 0.00210573478, 0.00673835166, 0.0311648771, 0.194148764, 0.357974946, 0.0437992848, 0.0109498212, 0.00715949852, 0.00126344094, 0.00210573478, 0.0151612908, 0.0172670260, 0.0193727612, 0.00547491061, 0.00210573478, 0.00168458791, 0.00168458791, 0.000421146979, 0.000842293957, 0.00336917583, 0.00168458791 };
   int32_t exp_class_idx = 4;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
   
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** \purpose  
 * Test that SVM_classification correctly classifies trailer as class 11 by checking that the corresponding assigned length
 * and axel length is correct given the normalized input array.
 */
TEST(f360_SVM_classification, Test_SVM_classification_trailer_class_boat)
{
   /** \precond
    * norm_sample array has been filled with values that indicates trailer is of class 11
    * Expected trailer length and axel length are set to values corresponding to expected class. 
    */

   float32_t norm_sample[ROW_NUMBER] = { 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00269400910, 0.0107760364, 0.00484921644, 0.0264012907, 0.100217141, 0.0231684782, 0.0856694877, 0.329207927, 0.0312505066, 0.0129312444, 0.0210132711, 0.0296341013, 0.0921351165, 0.154636130, 0.197740272, 0.0721994489, 0.0371773280, 0.0808202773, 0.130928844, 0.544189870, 0.223063961, 0.134700462, 0.187503040, 0.166489765, 0.0872858986, 0.0199356675, 0.00862082932, 0.0199356675, 0.0398713350, 0.566819549, 0.100217141, 0.0145476498, 0.00592681998, 0.00377161289, 0.00538801821, 0.00269400910, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000, 0.00000000 };
   
   int32_t exp_class_idx = 11;
   float32_t exp_trailer_len = length_array[exp_class_idx];
   float32_t exp_axel_trailer_len = axel_length_array[exp_class_idx];
	
   /** \action
    * Call Trailer_Detector_TL::SVM_classification() from the mocked class.
    */
   trailer_detector_TL.call_SVM_classification(norm_sample, predict_class, max_score);
   trailer_len = length_array[predict_class];
   axel_trailer_len = axel_length_array[predict_class];

   /** \result
    * Check that trailer_len and axel_trailer_len are assigned the expected values.
    */
   DOUBLES_EQUAL_TEXT(exp_trailer_len, trailer_len, test_pass_th, "Trailer len is not correct.");
   DOUBLES_EQUAL_TEXT(exp_axel_trailer_len, axel_trailer_len, test_pass_th, "axel trailer len is not correct.");
}

/** @}*/

/** \defgroup  tl_estimate
 *  @{
 */

/** \brief
 * Tests that Norm_Detection_Row works as intended
 */
TEST_GROUP(f360_estimate)
{	
   // Create an instance of the Trailer_Detector_TP class
   Trailer_Detector_TLmock trailer_detector_TL;
   
   // Define thresholds for floating number equality comparision
   const float32_t test_pass_th = 1e-9F;

   // SVM model parameters (from f360_trailer_detector_TL_SVM.h)
   const float32_t length_array[CLASS_NUMBER] = { 4.0F, 5.5F, 4.5F, 4.0F, 5.5F, 3.0F, 6.0F, 8.8F, 4.8F, 10.0F };
   const float32_t axel_length_array[CLASS_NUMBER] = { 2.9F, 3.83F, 3.2F, 3.14F, 3.83F, 2.57F, 4.3F, 5.9F, 3.3F, 6.3F };

   TL_Trailer_Detection_Row trailer_det_row_struct_set_val;

   int32_t exp_class_idx;
   float32_t exp_trailer_length = 0.0F;
   float32_t exp_axel_trailer_length = 0.0F;

   /** \setup
    * Initialize the Trailer_Detector_TP class instance with some default values
    */
   TEST_SETUP()
   {
      trailer_detector_TL.Initialize();

      trailer_detector_TL.set_window_timer(800U);

      for (uint32_t i = 0U; i < ROW_NUMBER; i++)
      {
         trailer_det_row_struct_set_val.detection_row[i] = 0;
      }
      trailer_det_row_struct_set_val.detection_row[0U] = 2;
      trailer_det_row_struct_set_val.detection_row[4U] = 1;
      trailer_det_row_struct_set_val.detection_row[26U] = 4;
      trailer_det_row_struct_set_val.detection_row[ROW_NUMBER - 1] = 5;
      trailer_detector_TL.set_trailer_detection_row_struct(trailer_det_row_struct_set_val);
   }
};

/** @}*/
