/*=========================================================================
*  FILE: f360_safety_control_logic.cpp
*=========================================================================
* Copyright (C) 2021 Aptiv. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains SafetyControlLogic class definition.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
*========================================================================*/

/******************************
* Includes
*******************************/

#include "f360_safety_control_logic.h"

namespace f360_variant_A
{
   static void Update_Non_Critical_Fault_Counter(const bool f_fault,
                                                 const uint8_t max_non_critical_fault_count,
                                                 const uint8_t max_fault_status_count,
                                                 uint8_t& r_fault_counter,
                                                 uint8_t& r_fault_remain_counter);

   static void Update_Fault_Status(const bool f_fault,
                                   const uint8_t max_fault_status_count,
                                   uint8_t& r_fault_remain_counter,
                                   SafetyControlLogic::CYCLE_FAULT_STATUS& fault_status);

   static bool check_fault_status_any(const SafetyControlLogic::SCL_Output_T& r_tracker_fault_status,
                                      const uint8_t check_fault_status);

   static bool check_fault_status_all(const SafetyControlLogic::SCL_Output_T& r_tracker_fault_status,
                                      const uint8_t check_fault_status);
   /*=========================================================================
   * Method         SafetyControlLogic::SafetyControlLogic
   *
   * Description    Constructor of SafetyControlLogic.
   *
   * Parameters     Input_Diagnostics& input_diagnostics
   *                Output_Diagnostics& output_diagnostics
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   SafetyControlLogic::SafetyControlLogic(Input_Diagnostics& IN, Output_Diagnostics& OUT) :
      inp_diag(IN),
      out_diag(OUT),
      scl_status(),
      input_faults(),
      output_faults(),
      core_info_fault_counter(0U),
      core_info_fault_remain_counter(0U),
      vehicle_index_fault_counter(0U),
      vehicle_index_fault_remain_counter(0U),
      host_speed_fault_counter(0U),
      host_speed_fault_remain_counter(0U),
      host_yawrate_fault_counter(0U),
      host_yawrate_fault_remain_counter(0U),
      host_longitudinal_acceleration_fault_counter(0U),
      host_longitudinal_acceleration_fault_remain_counter(0U),
      host_lateral_acceleration_fault_counter(0U),
      host_lateral_acceleration_fault_remain_counter(0U),
      mounting_pos_fault_counter(),
      mounting_pos_fault_remain_counter(),
      polarity_fault_counter(),
      polarity_fault_remain_counter(),
      boresight_angle_fault_counter(),
      boresight_angle_fault_remain_counter(),
      look_index_fault_counter(),
      look_index_fault_remain_counter(),
      sensor_trk_ts_divergence_fault_counter(),
      sensor_trk_ts_divergence_fault_remain_counter(),
      object_track_fault_remain_counter(0U)
   {}

   /*=========================================================================
   * Method         SafetyControlLogic::get_max_non_critical_fault_count
   *
   * Description    Function to get max_non_critical_fault_count.
   *
   * Parameters     None.
   *
   * Returns        uint8_t
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   uint8_t SafetyControlLogic::SCL_Calibrations_T::get_max_non_critical_fault_count() const
   {
      return max_non_critical_fault_count;
   }

   /*=========================================================================
   * Method         SafetyControlLogic::get_max_fault_status_count
   *
   * Description    Function to get max_fault_status_count.
   *
   * Parameters     None.
   *
   * Returns        uint8_t
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   uint8_t SafetyControlLogic::SCL_Calibrations_T::get_max_fault_status_count() const
   {
      return max_fault_status_count;
   }

   /*=========================================================================
   * Method         SafetyControlLogic::evaluate_cycle
   *
   * Description    Function evaluates the fault status from Input Diagnostcis
   *
   * Parameters     const F360_Core_Info_T& core_info - pointer to core info struct
   *                const F360_Host_T &host - reference to host info struct
   *                const F360_Radar_Sensor_Calib_T (&sensor_calibs)[MAX_NUMBER_OF_SENSORS] - reference to sensor calibrations struct
   *                const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS] - reference to sensors information struct
   *                const All_Objects_Log_T& obj_log - Reference to object tracks information log struct
   *
   * Returns        SCL_Output_T fault_status
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   SafetyControlLogic::SCL_Output_T SafetyControlLogic::evaluate_cycle(
      const F360_Core_Info_T& core_info,
      const F360_Host_T& host,
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const F360_Object_Log_Output_T& obj_log
   )
   {
      //Receive faults from Input_Diagnostics
      input_faults = inp_diag.Execute(core_info, host, sensors);

      //Receive faults from Output_Diagnostics
      output_faults = out_diag.Execute(obj_log);

      //Evaluates faults from Input_Diagnostics and Output_Diagnostics
      determine_faults_status(input_faults, output_faults);

      return this->scl_status;
   }

   /*=========================================================================
    * Method         Update_Non_Critical_Fault_Counter
    *
    * Description    Updates the non critical fault counter r_fault_counter and saturates
    *                r_fault_remain_counter with max_fault_status_count
    *                if r_fault_counter >= max_non_critical_fault_count.
    *
    * Parameters     const bool f_fault
    *                const uint8_t max_non_critical_fault_count
    *                const uint8_t max_fault_status_count
    *                uint8_t& r_fault_counter
    *                uint8_t& r_fault_remain_counter
    *
    * Returns        None
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   static void Update_Non_Critical_Fault_Counter(const bool f_fault,
                                                 const uint8_t max_non_critical_fault_count,
                                                 const uint8_t max_fault_status_count,
                                                 uint8_t& r_fault_counter,
                                                 uint8_t& r_fault_remain_counter)
   {
      // Update fault counter
      if (f_fault)
      {
         r_fault_counter++;
      }
      else
      {
         r_fault_counter = 0U;
      }

      // Check if to many faults in a row
      if (r_fault_counter >= max_non_critical_fault_count)
      {
         r_fault_remain_counter = max_fault_status_count;
      }
   }

   /*=========================================================================
    * Method         Update_Fault_Status
    *
    * Description    Updates the fault status and r_fault_remain_counter
    *
    * Parameters     const bool f_fault
    *                const uint8_t max_fault_status_count
    *                uint8_t& r_fault_remain_counter
    *                SafetyControlLogic::CYCLE_FAULT_STATUS& fault_status
    *
    * Returns        None
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   static void Update_Fault_Status(const bool f_fault,
                                   const uint8_t max_fault_status_count,
                                   uint8_t& r_fault_remain_counter,
                                   SafetyControlLogic::CYCLE_FAULT_STATUS& fault_status)
   {
      // Keep fault high for max_fault_status_count after no faults. Reset to max if fault present
      if (r_fault_remain_counter > 0U)
      {
         if (!f_fault)
         {
            r_fault_remain_counter--;

            if (0U < r_fault_remain_counter)
            {
               fault_status = SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PRESENT_STATUS;
            }
         }
         else
         {
            r_fault_remain_counter = max_fault_status_count;
            fault_status = SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PRESENT_STATUS;
         }
      }
   }

   /*=========================================================================
    * Method         Update_Non_Critical_Fault_Info
    *
    * Description    Updates the fault info for non critical errors
    *
    * Parameters     const bool f_fault,
    *                uint8_t& r_fault_counter,
    *                uint8_t& r_fault_remain_counter,
    *                SafetyControlLogic::CYCLE_FAULT_STATUS& r_fault_status
    *
    * Returns        None
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void SafetyControlLogic::update_non_critical_fault_info(
      const bool f_fault,
      uint8_t& r_fault_counter,
      uint8_t& r_fault_remain_counter,
      CYCLE_FAULT_STATUS& r_fault_status) const
   {
      Update_Non_Critical_Fault_Counter(f_fault,
                                        calib.get_max_non_critical_fault_count(),
                                        calib.get_max_fault_status_count(),
                                        r_fault_counter,
                                        r_fault_remain_counter);
      Update_Fault_Status(f_fault,
                          calib.get_max_fault_status_count(),
                          r_fault_remain_counter,
                          r_fault_status);
   }

   /*=========================================================================
    * Method         Update_Critical_Fault_Info
    *
    * Description    Updates the fault status for critical errors
    *
    * Parameters     const bool f_fault,
    *                uint8_T& r_fault_remain_counter,
    *                SCL_Output_T& r_output
    *
    * Returns        None
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   void SafetyControlLogic::update_critical_fault_info(
      const bool f_fault,
      uint8_t& r_fault_remain_counter,
      CYCLE_FAULT_STATUS& r_fault_status,
      bool& r_critical_fault_present) const
   {
      //Check critical fault
      if (f_fault)
      {
         r_fault_remain_counter = calib.get_max_fault_status_count();
         r_critical_fault_present = true;
      }
      else
      {
         r_critical_fault_present = false;
      }

      Update_Fault_Status(f_fault,
                          calib.get_max_fault_status_count(),
                          r_fault_remain_counter,
                          r_fault_status);
   }

   /*=========================================================================
   * Method         SafetyControlLogic::determine_faults_status
   *
   * Description    Function determines critical fault status.
   *
   * Parameters     Input_Faults_T &input_status
   *                Output_Faults_T &output_status
   *
   * Returns        SCL_Output_T scl_status
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void SafetyControlLogic::determine_faults_status(Input_Faults_T const& input_status, Output_Faults_T const& output_status)
   {
      // Core Info Fault
      check_core_info_faults(input_status.core_info);

      // Host Info Fault
      check_host_info_faults(input_status.host_info);

      // Radar Sensor Calib Fault
      check_radar_sensor_calib_faults(input_status.sensors_calibs);

      // Radar Sensor Fault
      check_radar_sensor_faults(input_status.sensors);

      // Object Tracks Fault
      check_object_tracks_faults(output_status);

      // Compute Overall Fault Status
      compute_overall_fault_status();
   }

   /*=========================================================================
   * Method         SafetyControlLogic::Check_Core_Info_Faults
   *
   * Description    Function determines critical fault status of Core_Info_Faults_T.
   *
   * Parameters     Core_Info_Faults_T const& r_core_info
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           For input diagnostics fault SCL should never set should_reset flag
   *========================================================================*/
   void SafetyControlLogic::check_core_info_faults(Core_Info_Faults_T const& r_core_info)
   {
      // Set fault status to not preset
      this->scl_status.core_info_fault_status = FAULT_NOT_PRESENT_STATUS;

      // Check core_info faults
      const bool f_core_info_fault = (
         r_core_info.time_us_no_increase ||
         r_core_info.cnt_loops_no_increase ||
         r_core_info.elapsed_time_below_lower_limit ||
         r_core_info.elapsed_time_above_upper_limit);
      update_non_critical_fault_info(f_core_info_fault,
                                     core_info_fault_counter,
                                     core_info_fault_remain_counter,
                                     this->scl_status.core_info_fault_status);
   }

   /*=========================================================================
   * Method         SafetyControlLogic::check_host_info_faults
   *
   * Description    Function determines critical fault status of Host_Info_Faults_T.
   *
   * Parameters     Host_Info_Faults_T &r_host_info
   *
   * Returns        None
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           For input diagnostics fault SCL should never set should_reset flag
   *========================================================================*/
   void SafetyControlLogic::check_host_info_faults(Host_Info_Faults_T const& r_host_info)
   {
      // Set fault status to not preset
      this->scl_status.host_info_fault_status = FAULT_NOT_PRESENT_STATUS;

      // Check vehicle index faults
      const bool f_index_fault = r_host_info.vehicle_index_no_increase;
      update_non_critical_fault_info(f_index_fault,
                                     vehicle_index_fault_counter,
                                     vehicle_index_fault_remain_counter,
                                     this->scl_status.host_info_fault_status);

      // Check host speed faults. Temporary workaround - Force f_speed_fault to false. TODO: remove in DFF-958
      const bool f_speed_fault = (false && r_host_info.host_speed_invalid);
      update_non_critical_fault_info(f_speed_fault,
                                     host_speed_fault_counter,
                                     host_speed_fault_remain_counter,
                                     this->scl_status.host_info_fault_status);

      // Check host yawrate faults. DFF-1725: Temporary workaround - host_yawrate_invalid changed to check for faulty curvature_rear or vcs_sideslip values
      const bool f_yawrate_fault = r_host_info.host_yawrate_invalid;
      update_non_critical_fault_info(f_yawrate_fault,
                                     host_yawrate_fault_counter,
                                     host_yawrate_fault_remain_counter,
                                     this->scl_status.host_info_fault_status);

      // Check host longitudinal acceleration faults. Temporary workaround - Force f_long_accel_fault to false. TODO: remove in DFF-958
      const bool f_long_accel_fault = (false && r_host_info.host_longitudinal_acceleration_invalid);
      update_non_critical_fault_info(f_long_accel_fault,
                                     host_longitudinal_acceleration_fault_counter,
                                     host_longitudinal_acceleration_fault_remain_counter,
                                     this->scl_status.host_info_fault_status);

      // Check host lateral acceleration faults. Temporary workaround - Force f_lat_accel_fault to false. TODO: remove in DFF-958
      const bool f_lat_accel_fault = (false && r_host_info.host_lateral_acceleration_invalid);
      update_non_critical_fault_info(f_lat_accel_fault,
                                     host_lateral_acceleration_fault_counter,
                                     host_lateral_acceleration_fault_remain_counter,
                                     this->scl_status.host_info_fault_status);
   }

   /*=========================================================================
   * Method         SafetyControlLogic::check_radar_sensor_calib_faults
   *
   * Description    Function determines critical fault status of Radar_Sensor_Calib_Faults_T.
   *
   * Parameters     Radar_Sensor_Calib_Faults_T const (&r_sensors_calibs)[MAX_NUMBER_OF_SENSORS]
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           For input diagnostics fault SCL should never set should_reset flag
   *========================================================================*/
   void SafetyControlLogic::check_radar_sensor_calib_faults(
      Radar_Sensor_Calib_Faults_T const (&r_sensors_calibs)[MAX_NUMBER_OF_SENSORS])
   {
      for (uint32_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         // Set fault to not preset
         this->scl_status.sensors_calibs_fault_status[i] = FAULT_NOT_PRESENT_STATUS;

         // Check radar mounting position fault
         const bool f_mounting_pos_fault = r_sensors_calibs[i].mounting_pos_is_invalid;
         update_non_critical_fault_info(f_mounting_pos_fault,
                                        mounting_pos_fault_counter[i],
                                        mounting_pos_fault_remain_counter[i],
                                        this->scl_status.sensors_calibs_fault_status[i]);

         // Check radar polarity faults
         const bool f_polarity_fault = r_sensors_calibs[i].polarity_is_invalid;
         update_non_critical_fault_info(f_polarity_fault,
                                        polarity_fault_counter[i],
                                        polarity_fault_remain_counter[i],
                                        this->scl_status.sensors_calibs_fault_status[i]);

         // Check radar boresight faults
         const bool f_boresight_angle_fault = r_sensors_calibs[i].boresight_angle_is_invalid;
         update_non_critical_fault_info(f_boresight_angle_fault,
                                        boresight_angle_fault_counter[i],
                                        boresight_angle_fault_remain_counter[i],
                                        this->scl_status.sensors_calibs_fault_status[i]);
      }
   }

   /*=========================================================================
   * Method         SafetyControlLogic::check_radar_sensor_faults
   *
   * Description    Function determines critical fault status of Radar_Sensor_Faults_T.
   *
   * Parameters     Radar_Sensor_Faults_T const (&r_sensors)[MAX_NUMBER_OF_SENSORS]
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           For input diagnostics fault SCL should never set should_reset flag
   *========================================================================*/
   void SafetyControlLogic::check_radar_sensor_faults(Radar_Sensor_Faults_T const (&r_sensors)[MAX_NUMBER_OF_SENSORS])
   {
      for (uint32_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         // Set fault to not preset
         this->scl_status.sensors_fault_status[i] = FAULT_NOT_PRESENT_STATUS;

         // Check radar look index fault
         const bool f_look_index_fault = r_sensors[i].look_index_no_increase;
         update_non_critical_fault_info(f_look_index_fault,
                                        look_index_fault_counter[i],
                                        look_index_fault_remain_counter[i],
                                        this->scl_status.sensors_fault_status[i]);

         // Check radar sensor vs tracker timestamp divergence faults
         const bool f_sensor_trk_ts_divergence_fault = r_sensors[i].sensor_vs_tracker_timestamp_divergence;
         update_non_critical_fault_info(f_sensor_trk_ts_divergence_fault,
                                        sensor_trk_ts_divergence_fault_counter[i],
                                        sensor_trk_ts_divergence_fault_remain_counter[i],
                                        this->scl_status.sensors_fault_status[i]);
      }
   }

   /*=========================================================================
   * Method         SafetyControlLogic::check_object_tracks_faults
   *
   * Description    Function determines critical fault status of F360_Object_Track_T.
   *
   * Parameters     Output_Faults_T &output_status
   *
   * Returns        SCL_Output_T object_track_faults_status
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           If there is fault from output_diagnostics, tracker will be reset
   *========================================================================*/
   void SafetyControlLogic::check_object_tracks_faults(Output_Faults_T const& r_object_tracks)
   {
      // Set fault to not preset
      this->scl_status.object_track_fault_status = FAULT_NOT_PRESENT_STATUS;
      this->scl_status.should_reset = false;

      // Check object tracks faults
      const bool f_object_state_fault = (
         r_object_tracks.f_track_positions_faulty ||
         r_object_tracks.f_track_velocities_faulty ||
         r_object_tracks.f_track_accelerations_faulty);

      update_critical_fault_info(
         f_object_state_fault,
         object_track_fault_remain_counter,
         this->scl_status.object_track_fault_status,
         this->scl_status.should_reset);
   }

   /*===========================================================================*\
    * FUNCTION: check_fault_status_any()
    *===========================================================================
    * RETURN VALUE:
    * bool f_has_fault_status
    *
    * PARAMETERS:
    * const SCL_Output_Log_T& r_tracker_fault_status
    * const uint8_t check_fault_status
    *
    * EXTERNAL REFERENCES:
    * NONE
    *
    * DEVIATIONS FROM STANDARDS:
    * NONE
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * Returns true if any of the fault statuses has the check fault value
    *
    * PRECONDITIONS:
    * NONE
    *
    * POSTCONDITIONS:
    * NONE
    *
   \*===========================================================================*/
   static bool check_fault_status_any(const SafetyControlLogic::SCL_Output_T& r_tracker_fault_status,
                                      const uint8_t check_fault_status)
   {
      bool f_has_fault_status = (
         (r_tracker_fault_status.core_info_fault_status    == check_fault_status) ||
         (r_tracker_fault_status.host_info_fault_status    == check_fault_status) ||
         (r_tracker_fault_status.object_track_fault_status == check_fault_status)
         );

      for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         f_has_fault_status = f_has_fault_status || (
            (r_tracker_fault_status.sensors_calibs_fault_status[i] == check_fault_status) ||
            (r_tracker_fault_status.sensors_fault_status[i]        == check_fault_status)
            );
      }

      return (f_has_fault_status);
   }

   /*===========================================================================*\
    * FUNCTION: check_fault_status_all()
    *===========================================================================
    * RETURN VALUE:
    * bool f_all_fault_status
    *
    * PARAMETERS:
    * const SCL_Output_Log_T& r_tracker_fault_status
    * const uint8_t check_fault_status
    *
    * EXTERNAL REFERENCES:
    * NONE
    *
    * DEVIATIONS FROM STANDARDS:
    * NONE
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * Returns true if all of the fault statuses has the check fault value

    * PRECONDITIONS:
    * NONE
    *
    * POSTCONDITIONS:
    * NONE
    *
   \*===========================================================================*/
   static bool check_fault_status_all(const SafetyControlLogic::SCL_Output_T& r_tracker_fault_status,
                                      const uint8_t check_fault_status)
   {
      bool f_all_fault_status = (
         (r_tracker_fault_status.core_info_fault_status    == check_fault_status) &&
         (r_tracker_fault_status.host_info_fault_status    == check_fault_status) &&
         (r_tracker_fault_status.object_track_fault_status == check_fault_status)
         );

      for (uint8_t i = 0U; i < MAX_NUMBER_OF_SENSORS; i++)
      {
         f_all_fault_status = f_all_fault_status && (
            (r_tracker_fault_status.sensors_calibs_fault_status[i] == check_fault_status) &&
            (r_tracker_fault_status.sensors_fault_status[i]        == check_fault_status)
            );
      }

      return (f_all_fault_status);
   }

   /*=========================================================================
   * Method         SafetyControlLogic::compute_overall_fault_status
   *
   * Description    Computes the overall fault status by combining the separate ones
   *                First check if any scl_output_faults is set to FAULT_PRESENT_STATUS (60U). => fault_status = (60U)
   *                If not, then check if any scl_output_faults is set to FAULT_PARTIAL_PRESENT_STATUS (105U). => fault_status = (105U)
   *                If not, then check if all the scl_output_faults are set to FAULT_NOT_PRESENT_STATUS (195U). => fault_status = (195U)
   *                If not, then set fault_status = FAULT_PRESENT_STATUS (60U);
   *
   * Parameters     None.
   *
   * Returns        None.
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   void SafetyControlLogic::compute_overall_fault_status(void)
   {
      const bool f_tracker_fault_present_status = check_fault_status_any(
         this->scl_status, SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PRESENT_STATUS);
      if (f_tracker_fault_present_status)
      {
         this->scl_status.overall_fault_status = SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PRESENT_STATUS;
      }
      else
      {
         const bool f_tracker_fault_partial_present_status = check_fault_status_any(
            this->scl_status, SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PARTIAL_PRESENT_STATUS);
         if (f_tracker_fault_partial_present_status)
         {
            this->scl_status.overall_fault_status = SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PARTIAL_PRESENT_STATUS;
         }
         else
         {
            const bool f_tracker_fault_not_present_status = check_fault_status_all(
               this->scl_status, SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_NOT_PRESENT_STATUS);
            if (f_tracker_fault_not_present_status)
            {
               this->scl_status.overall_fault_status = SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_NOT_PRESENT_STATUS;
            }
            else
            {
               this->scl_status.overall_fault_status = SafetyControlLogic::CYCLE_FAULT_STATUS::FAULT_PRESENT_STATUS;
            }
         }
      }
   }

   /*=========================================================================
   * Method         SafetyControlLogic::get_scl_status
   *
   * Description    Function to get scl_status .
   *
   * Parameters     None.
   *
   * Returns        SCL_Output_T&
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   const SafetyControlLogic::SCL_Output_T& SafetyControlLogic::get_scl_status() const
   {
      return scl_status;
   }

   /*=========================================================================
   * Method         SafetyControlLogic::get_input_status
   *
   * Description    Function to get input_status .
   *
   * Parameters     None.
   *
   * Returns        Input_Faults_T&
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   const Input_Faults_T& SafetyControlLogic::get_input_status() const
   {
      return input_faults;
   }

   /*=========================================================================
   * Method         SafetyControlLogic::get_output_status
   *
   * Description    Function to get output_status .
   *
   * Parameters     None.
   *
   * Returns        Output_Faults_T&
   *
   * Externals:     None.
   *
   * Precondition   None.
   *
   * Postcondition  None.
   *
   * Note           None.
   *========================================================================*/
   const Output_Faults_T& SafetyControlLogic::get_output_status() const
   {
      return output_faults;
   }
}
