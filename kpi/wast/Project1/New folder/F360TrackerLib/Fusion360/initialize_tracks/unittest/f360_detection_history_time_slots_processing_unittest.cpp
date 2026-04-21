/** \file
   This file contains unit test to Split_Dets_Into_Time_Slots function.
*/

#include "f360_detection_history_time_slots_processing.h"
#include <CppUTest/TestHarness.h>


using namespace f360_variant_A;

/** \defgroup  f360_detection_history_time_slots_processing
 *  @{
 */

/** \brief
*  This test group is testing Split_Dets_Into_Time_Slots function
**/

TEST_GROUP(f360_detection_history_time_slots_processing)
{
   F360_NEES_CFMI_Information_T nees_cfmi_information = {};
   F360_Detection_Time_Slots_T time_slots = {};
   float32_t dt_for_split = 1.0F;

   TEST_SETUP()
   {
      nees_cfmi_information = {};
      time_slots = {};
   }

   void Add_Detection(float32_t time_since_meas, uint32_t det_index, bool f_historical)
   {
      uint32_t& nees_det_index = nees_cfmi_information.dets_num;
      F360_NEES_CFMI_Detection_T nees_det = {};
      nees_det.time_since_meas = time_since_meas;
      nees_det.idx = {det_index, nees_det_index, f_historical};

      nees_cfmi_information.detections[nees_det_index] = nees_det;
      nees_det_index++;
      if (f_historical)
      {
         nees_cfmi_information.old_dets_count++;
      }
      else
      {
         nees_cfmi_information.current_dets_count++;
      }
   }
};

TEST(f360_detection_history_time_slots_processing, whenClusterAndDetectionHistoryAreEmptyTimeSlotsShouldBeEmpty)
{
   /** \precond
    cluster and detection history are empty
    **/
    /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    *time slots are empty
    **/
   CHECK_EQUAL(0, time_slots.number_of_active_slots);
   CHECK_EQUAL(0, time_slots.number_of_dets);
}

TEST(f360_detection_history_time_slots_processing, WhenClusterHasOneOldDetectionTimeSlotsShouldHaveOneTimeSlot)
{
   /** \precond
   one detection in the cluster
    **/
   float32_t time_since_measurement = 17.0;
   int32_t detection_index_in_history = 5;
   bool det_is_historical = true;
   Add_Detection(time_since_measurement, detection_index_in_history, det_is_historical);

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    *one time slot is created
    **/
   CHECK_EQUAL(1, time_slots.number_of_active_slots);
   CHECK_EQUAL(1, time_slots.number_of_dets);
}

TEST(f360_detection_history_time_slots_processing, WhenClusterHasOneOldDetectionItShouldBeRecordedInFirstSlotOfTimeSlots)
{
   /** \precond
     *one detection in the cluster
     **/
   float32_t time_since_measurement = 12.0;
   int32_t detection_index_in_history = 3;
   bool det_is_historical = true;
   Add_Detection(time_since_measurement, detection_index_in_history, det_is_historical);

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    *detection is recorded in the first time slot
    **/
   F360_Detection_Time_Slot_T& first_slot = time_slots.time_since_meas_slots[0];
   CHECK_EQUAL(1, first_slot.number_of_active_detections);
   CHECK_EQUAL(0, first_slot.hist_dets_idx[0]);
   CHECK_EQUAL(time_since_measurement, first_slot.max_time_since_meas);
   CHECK_EQUAL(time_since_measurement, first_slot.min_time_since_meas);
}

TEST(f360_detection_history_time_slots_processing, WhenClusterHasTwoOldDetectionsAndDiffInTimeIsSmallerThanTresholdTimeSlotsShouldHaveOneSlot)
{
   /** \precond
     *two detections in the clusted with difference in time smaller than treshold
     **/
   float32_t first_time_since_measurement = 27.4;
   int32_t first_detection_index_in_history = 3;  
   bool det_is_historical = true;
   Add_Detection(first_time_since_measurement, first_detection_index_in_history, det_is_historical);

   float32_t second_time_since_measurement = 30.0;
   int32_t second_detection_index_in_history = 2;
   Add_Detection(second_time_since_measurement, second_detection_index_in_history, det_is_historical);

   dt_for_split = 3.0;

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    *one time slot with two detections is created
    **/
   CHECK_EQUAL(1, time_slots.number_of_active_slots);
   CHECK_EQUAL(2, time_slots.number_of_dets);
}

TEST(f360_detection_history_time_slots_processing, WhenClusterHasTwoOldDetectionsAndDiffInTimeIsGreaterThanTresholdTimeSlotsShouldHaveTwoSlots)
{
   /** \precond
     *two detections in the clusted with difference in time greater than treshold
     **/
   float32_t first_time_since_measurement = 14.0;
   int32_t first_detection_index_in_history = 17;
   bool det_is_historical = true;
   Add_Detection(first_time_since_measurement, first_detection_index_in_history, det_is_historical);

   float32_t second_time_since_measurement = 21.1;
   int32_t second_detection_index_in_history = 0;
   Add_Detection(second_time_since_measurement, second_detection_index_in_history, det_is_historical);

   dt_for_split = 7.0;

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    *two time slots are created
    **/
   CHECK_EQUAL(2, time_slots.number_of_active_slots);
   CHECK_EQUAL(2, time_slots.number_of_dets);
}

TEST(f360_detection_history_time_slots_processing, ClusterShouldOnlyHoldEightDetectionsAndAdditionalDetectionsShouldBeDiscarded)
{
   /** \precond
     *ten detections with time difference smaller than threshold between every two consecutive detections
     **/
   dt_for_split = 1.1;

   float32_t time_since_measurement_1 = 1.0;
   float32_t time_since_measurement_2 = 2.0;
   float32_t time_since_measurement_3 = 3.0;
   float32_t time_since_measurement_4 = 4.0;
   float32_t time_since_measurement_5 = 5.0;
   float32_t time_since_measurement_6 = 6.0;
   float32_t time_since_measurement_7 = 7.0;
   float32_t time_since_measurement_8 = 8.0;
   float32_t time_since_measurement_9 = 9.0;
   float32_t time_since_measurement_10 = 10.0;

   int32_t detection_index_in_history_1 = 1;
   int32_t detection_index_in_history_2 = 2;
   int32_t detection_index_in_history_3 = 3;
   int32_t detection_index_in_history_4 = 4;
   int32_t detection_index_in_history_5 = 5;
   int32_t detection_index_in_history_6 = 6;
   int32_t detection_index_in_history_7 = 7;
   int32_t detection_index_in_history_8 = 8; 
   int32_t detection_index_in_history_9 = 9;
   int32_t detection_index_in_history_10 = 12;

   bool det_is_historical = true;

   Add_Detection(time_since_measurement_1, detection_index_in_history_1, det_is_historical);
   Add_Detection(time_since_measurement_2, detection_index_in_history_2, det_is_historical);
   Add_Detection(time_since_measurement_3, detection_index_in_history_3, det_is_historical);
   Add_Detection(time_since_measurement_4, detection_index_in_history_4, det_is_historical);
   Add_Detection(time_since_measurement_5, detection_index_in_history_5, det_is_historical);
   Add_Detection(time_since_measurement_6, detection_index_in_history_6, det_is_historical);
   Add_Detection(time_since_measurement_7, detection_index_in_history_7, det_is_historical);
   Add_Detection(time_since_measurement_8, detection_index_in_history_8, det_is_historical);
   Add_Detection(time_since_measurement_9, detection_index_in_history_9, det_is_historical);
   Add_Detection(time_since_measurement_10, detection_index_in_history_10, det_is_historical);

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    *one time slot is created with only eight detections
    **/
   CHECK_EQUAL(1, time_slots.number_of_active_slots);
   CHECK_EQUAL(8, time_slots.number_of_dets);

   CHECK_EQUAL(8, time_slots.time_since_meas_slots[0].number_of_active_detections);
}

TEST(f360_detection_history_time_slots_processing, ItShouldBePossibleForOnlyTenClustersToBeCreatedAdditionalClustersShouldBeDiscarded)
{
   /** \precond
     *Fifteen detections with time difference rather than threshold between every two consecutive detections
     **/
   dt_for_split = 0.5;

   float32_t time_since_measurement_1 = 1.0;
   float32_t time_since_measurement_2 = 2.0;
   float32_t time_since_measurement_3 = 3.0;
   float32_t time_since_measurement_4 = 4.0;
   float32_t time_since_measurement_5 = 5.0;
   float32_t time_since_measurement_6 = 6.0;
   float32_t time_since_measurement_7 = 7.0;
   float32_t time_since_measurement_8 = 8.0;
   float32_t time_since_measurement_9 = 9.0;
   float32_t time_since_measurement_10 = 10.0;
   float32_t time_since_measurement_11 = 11.0;
   float32_t time_since_measurement_12 = 12.0;
   float32_t time_since_measurement_13 = 13.0;
   float32_t time_since_measurement_14 = 14.0;
   float32_t time_since_measurement_15 = 15.0;

   int32_t detection_index_in_history_1 = 1;
   int32_t detection_index_in_history_2 = 2;
   int32_t detection_index_in_history_3 = 3;
   int32_t detection_index_in_history_4 = 4;
   int32_t detection_index_in_history_5 = 5;
   int32_t detection_index_in_history_6 = 6;
   int32_t detection_index_in_history_7 = 7;
   int32_t detection_index_in_history_8 = 8;
   int32_t detection_index_in_history_9 = 9;
   int32_t detection_index_in_history_10 = 10;
   int32_t detection_index_in_history_11 = 11;
   int32_t detection_index_in_history_12 = 12;
   int32_t detection_index_in_history_13 = 13;
   int32_t detection_index_in_history_14 = 14;
   int32_t detection_index_in_history_15 = 15;

   bool det_is_historical = true;

   Add_Detection(time_since_measurement_1, detection_index_in_history_1, det_is_historical);
   Add_Detection(time_since_measurement_2, detection_index_in_history_2, det_is_historical);
   Add_Detection(time_since_measurement_3, detection_index_in_history_3, det_is_historical);
   Add_Detection(time_since_measurement_4, detection_index_in_history_4, det_is_historical);
   Add_Detection(time_since_measurement_5, detection_index_in_history_5, det_is_historical);
   Add_Detection(time_since_measurement_6, detection_index_in_history_6, det_is_historical);
   Add_Detection(time_since_measurement_7, detection_index_in_history_7, det_is_historical);
   Add_Detection(time_since_measurement_8, detection_index_in_history_8, det_is_historical);
   Add_Detection(time_since_measurement_9, detection_index_in_history_9, det_is_historical);
   Add_Detection(time_since_measurement_10, detection_index_in_history_10, det_is_historical);
   Add_Detection(time_since_measurement_11, detection_index_in_history_11, det_is_historical);
   Add_Detection(time_since_measurement_12, detection_index_in_history_12, det_is_historical);
   Add_Detection(time_since_measurement_13, detection_index_in_history_13, det_is_historical);
   Add_Detection(time_since_measurement_14, detection_index_in_history_14, det_is_historical);
   Add_Detection(time_since_measurement_15, detection_index_in_history_15, det_is_historical);

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    * ten time slots are created. Each slot with one detection. 
    **/
   CHECK_EQUAL(10, time_slots.number_of_active_slots);
   CHECK_EQUAL(10, time_slots.number_of_dets);


}

TEST(f360_detection_history_time_slots_processing, ScenarioWithFourteenDetectionsThatShouldBeDividetToSevenClusters)
{
   /** \precond
     *detections that should be divided in to five time slots
     * dt_for_split = 0.025
      * time slot 1 {12345.681, 12345.701, 12345.7125634, 12345.737333}
      * time slot 2 {0.003, 0.000001, 0.014, 0.02}
      * time slot 3 {4.050, 4.0251, 4.04}
      * time slot 4 {4.07501, 4.079001}
      * time slot 5 {5.14}
     **/
   dt_for_split = 0.025;

   float32_t time_since_measurement_1 = 12345.681;      //time slot 1
   float32_t time_since_measurement_2 = 0.003;          //time slot 2
   float32_t time_since_measurement_3 = 4.050;          //time slot 3
   float32_t time_since_measurement_4 = 4.07501;        //time slot 4
   float32_t time_since_measurement_5 = 5.14;           //time slot 5
   float32_t time_since_measurement_6 = 12345.701;      //time slot 1
   float32_t time_since_measurement_7 = 0.000001;       //time slot 2
   float32_t time_since_measurement_8 = 4.0251;         //time slot 3
   float32_t time_since_measurement_9 = 4.079001;       //time slot 4
   float32_t time_since_measurement_10 = 4.04;          //time slot 3
   float32_t time_since_measurement_11 = 0.014;         //time slot 2
   float32_t time_since_measurement_12 = 12345.7125634; //time slot 1
   float32_t time_since_measurement_13 = 0.02;          //time slot 2
   float32_t time_since_measurement_14 = 12345.737333;  //time slot 1

   int32_t detection_index_in_history_1 = 3;    //time slot 1
   int32_t detection_index_in_history_2 = 5;    //time slot 2
   int32_t detection_index_in_history_3 = 7;    //time slot 3
   int32_t detection_index_in_history_4 = 0;    //time slot 4
   int32_t detection_index_in_history_5 = 1;    //time slot 5
   int32_t detection_index_in_history_6 = 2;    //time slot 1
   int32_t detection_index_in_history_7 = 4;    //time slot 2
   int32_t detection_index_in_history_8 = 6;    //time slot 3
   int32_t detection_index_in_history_9 = 9;    //time slot 4
   int32_t detection_index_in_history_10 = 12;  //time slot 3
   int32_t detection_index_in_history_11 = 10;  //time slot 2
   int32_t detection_index_in_history_12 = 11;  //time slot 1
   int32_t detection_index_in_history_13 = 13;  //time slot 2
   int32_t detection_index_in_history_14 = 14;  //time slot 1

   bool det_is_historical = true;

   Add_Detection(time_since_measurement_1, detection_index_in_history_1, det_is_historical);
   Add_Detection(time_since_measurement_2, detection_index_in_history_2, det_is_historical);
   Add_Detection(time_since_measurement_3, detection_index_in_history_3, det_is_historical);
   Add_Detection(time_since_measurement_4, detection_index_in_history_4, det_is_historical);
   Add_Detection(time_since_measurement_5, detection_index_in_history_5, det_is_historical);
   Add_Detection(time_since_measurement_6, detection_index_in_history_6, det_is_historical);
   Add_Detection(time_since_measurement_7, detection_index_in_history_7, det_is_historical);
   Add_Detection(time_since_measurement_8, detection_index_in_history_8, det_is_historical);
   Add_Detection(time_since_measurement_9, detection_index_in_history_9, det_is_historical);
   Add_Detection(time_since_measurement_10, detection_index_in_history_10, det_is_historical);
   Add_Detection(time_since_measurement_11, detection_index_in_history_11, det_is_historical);
   Add_Detection(time_since_measurement_12, detection_index_in_history_12, det_is_historical);
   Add_Detection(time_since_measurement_13, detection_index_in_history_13, det_is_historical);
   Add_Detection(time_since_measurement_14, detection_index_in_history_14, det_is_historical);

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    * detections are divded in to 5 time slots
    **/
   CHECK_EQUAL(5, time_slots.number_of_active_slots);
   CHECK_EQUAL(14, time_slots.number_of_dets);

   CHECK_EQUAL(4, time_slots.time_since_meas_slots[0].number_of_active_detections); // time slot 2
   CHECK_EQUAL(6, time_slots.time_since_meas_slots[0].hist_dets_idx[0]);
   CHECK_EQUAL(1, time_slots.time_since_meas_slots[0].hist_dets_idx[1]);
   CHECK_EQUAL(10, time_slots.time_since_meas_slots[0].hist_dets_idx[2]);
   CHECK_EQUAL(12, time_slots.time_since_meas_slots[0].hist_dets_idx[3]);

   CHECK_EQUAL(3, time_slots.time_since_meas_slots[1].number_of_active_detections); // time slot 3
   CHECK_EQUAL(7, time_slots.time_since_meas_slots[1].hist_dets_idx[0]);
   CHECK_EQUAL(9, time_slots.time_since_meas_slots[1].hist_dets_idx[1]);
   CHECK_EQUAL(2, time_slots.time_since_meas_slots[1].hist_dets_idx[2]);

   CHECK_EQUAL(2, time_slots.time_since_meas_slots[2].number_of_active_detections); // time slot 4
   CHECK_EQUAL(3, time_slots.time_since_meas_slots[2].hist_dets_idx[0]);
   CHECK_EQUAL(8, time_slots.time_since_meas_slots[2].hist_dets_idx[1]);


   CHECK_EQUAL(1, time_slots.time_since_meas_slots[3].number_of_active_detections); // time slot 5
   CHECK_EQUAL(4, time_slots.time_since_meas_slots[3].hist_dets_idx[0]);


   CHECK_EQUAL(4, time_slots.time_since_meas_slots[4].number_of_active_detections); // time slot 1
   CHECK_EQUAL(0, time_slots.time_since_meas_slots[4].hist_dets_idx[0]);
   CHECK_EQUAL(5, time_slots.time_since_meas_slots[4].hist_dets_idx[1]);
   CHECK_EQUAL(11, time_slots.time_since_meas_slots[4].hist_dets_idx[2]);
   CHECK_EQUAL(13, time_slots.time_since_meas_slots[4].hist_dets_idx[3]);
}

TEST(f360_detection_history_time_slots_processing, EachClusterShouldHaveMinAndMaxTimeSinceMeas)
{
   /** \precond
     *detections that should be divided in to four time slots
     * dt_for_split = 5.0
      * time slot 1 {75.0, 73.0}
      * time slot 2 {30.0, 31.0, 30.5, 29.0}
      * time slot 3 {90.0}
      * time slot 4 {10528, 10529}
     **/
   dt_for_split = 5.0;

   float32_t time_since_measurement_1 = 75.0;      //time slot 1 max
   float32_t time_since_measurement_2 = 30.0;      //time slot 2
   float32_t time_since_measurement_3 = 10528.0;   //time slot 4 min
   float32_t time_since_measurement_4 = 73.0;      //time slot 1 min
   float32_t time_since_measurement_5 = 31.0;      //time slot 2 max
   float32_t time_since_measurement_6 = 90.0;      //time slot 3 min max
   float32_t time_since_measurement_7 = 10529.0;   //time slot 4 max
   float32_t time_since_measurement_8 = 30.5;      //time slot 2
   float32_t time_since_measurement_9 = 29.0;      //time slot 2 min

   int32_t detection_index_in_history_1 = 3;
   int32_t detection_index_in_history_2 = 0;
   int32_t detection_index_in_history_3 = 4;
   int32_t detection_index_in_history_4 = 1;
   int32_t detection_index_in_history_5 = 2;
   int32_t detection_index_in_history_6 = 6;
   int32_t detection_index_in_history_7 = 9;
   int32_t detection_index_in_history_8 = 11;
   int32_t detection_index_in_history_9 = 10;

   bool det_is_historical = true;

   Add_Detection(time_since_measurement_1, detection_index_in_history_1, det_is_historical);
   Add_Detection(time_since_measurement_2, detection_index_in_history_2, det_is_historical);
   Add_Detection(time_since_measurement_3, detection_index_in_history_3, det_is_historical);
   Add_Detection(time_since_measurement_4, detection_index_in_history_4, det_is_historical);
   Add_Detection(time_since_measurement_5, detection_index_in_history_5, det_is_historical);
   Add_Detection(time_since_measurement_6, detection_index_in_history_6, det_is_historical);
   Add_Detection(time_since_measurement_7, detection_index_in_history_7, det_is_historical);
   Add_Detection(time_since_measurement_8, detection_index_in_history_8, det_is_historical);
   Add_Detection(time_since_measurement_9, detection_index_in_history_9, det_is_historical);

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    * correct min_time_since_meas and max_time_since_meas are assigned to each time slot
    **/
   CHECK_EQUAL(time_since_measurement_9, time_slots.time_since_meas_slots[0].min_time_since_meas); // time slot 2
   CHECK_EQUAL(time_since_measurement_5, time_slots.time_since_meas_slots[0].max_time_since_meas); // time slot 2

   CHECK_EQUAL(time_since_measurement_4, time_slots.time_since_meas_slots[1].min_time_since_meas); // time slot 1
   CHECK_EQUAL(time_since_measurement_1, time_slots.time_since_meas_slots[1].max_time_since_meas); // time slot 1

   CHECK_EQUAL(time_since_measurement_6, time_slots.time_since_meas_slots[2].min_time_since_meas); // time slot 3
   CHECK_EQUAL(time_since_measurement_6, time_slots.time_since_meas_slots[2].max_time_since_meas); // time slot 3

   CHECK_EQUAL(time_since_measurement_3, time_slots.time_since_meas_slots[3].min_time_since_meas); // time slot 4
   CHECK_EQUAL(time_since_measurement_7, time_slots.time_since_meas_slots[3].max_time_since_meas); // time slot 5
}

TEST(f360_detection_history_time_slots_processing, CurrentDetectionSlotSaturation)
{
   /** \precond
     * Create N + 1 current where N is maximum count for time slot storage.
     **/

   const float32_t time_since_measurement = 1.0F;      //time slot 1 max
   const bool det_is_historical = false;

   for (uint16_t det_index = 0U; det_index < F360_NUMBER_OF_DETECTIONS_PER_HIST_IDX_TIME_SLOT + 1U; det_index++)
   {
      Add_Detection(time_since_measurement, det_index, det_is_historical);
   }

   /** \action
     *call Split_Dets_Into_Time_Slots function
     **/
   Split_Dets_Into_Time_Slots(nees_cfmi_information, time_slots, dt_for_split);

   /** \result
    * Check indexes of detections within first time slot.
    **/
   CHECK_EQUAL(0U, time_slots.time_since_meas_slots[0].hist_dets_idx[0]);
   CHECK_EQUAL(1U, time_slots.time_since_meas_slots[0].hist_dets_idx[1]);
   CHECK_EQUAL(2U, time_slots.time_since_meas_slots[0].hist_dets_idx[2]);
   CHECK_EQUAL(3U, time_slots.time_since_meas_slots[0].hist_dets_idx[3]);
   CHECK_EQUAL(4U, time_slots.time_since_meas_slots[0].hist_dets_idx[4]);
   CHECK_EQUAL(5U, time_slots.time_since_meas_slots[0].hist_dets_idx[5]);
   CHECK_EQUAL(6U, time_slots.time_since_meas_slots[0].hist_dets_idx[6]);
   CHECK_EQUAL(7U, time_slots.time_since_meas_slots[0].hist_dets_idx[7]);
}

/** @}*/
