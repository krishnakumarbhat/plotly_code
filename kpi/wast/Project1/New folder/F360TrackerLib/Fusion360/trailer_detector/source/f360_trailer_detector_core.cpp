/*===========================================================================*\
 * FILE: trailer_detector_core.cpp
 *============================================================================
 * Copyright (C) 2023 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailerDetectionCore class implementation.
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#include "f360_trailer_detector_core.h"
#include "f360_get_wall_time.h"

namespace f360_variant_A
{
   /*=========================================================================
    * Method         Trailer_Detector_Core::Initialize
    *
    * Description    Public method to run trailer detector core initialization
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
   void Trailer_Detector_Core::Initialize()
   {
      trailer_presence_obj.Initialize();
      trailer_length_obj.Initialize();
      trailer_width_obj.Initialize();
      trailer_angle_obj.Initialize();
   }

   /*=========================================================================
    * Method         Trailer_Detector_Core::Run_Trailer_Detector
    *
    * Description    Public method to run trailer detector main logic which
    *                calls trailer presence, length, width and angle in order,
    *                and fuse submodule outputs into a single one
    *
    * Parameters     const F360_Host_T &vehicle_data, 
                     const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
                     const F360_Detection_Props_T (&all_detections)[MAX_NUMBER_OF_DETECTIONS]
                     const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS]
                     const float32_t elapsed_time_s
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
   void Trailer_Detector_Core::Run_Trailer_Detector(const F360_Host_T &vehicle_data, 
      const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
      const F360_Detection_Props_T (&all_detections)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
      const float32_t elapsed_time_s,
      F360_TRKR_TIMING_INFO_T& timing_info)
   {
      const float32_t start_time = get_wall_time();
      /*trailer presence*/
      trailer_presence_obj.Run_Trailer_Detector(vehicle_data, raw_detect_list, all_detections, sensors);
      trailer_presence_obj.Get_Output(output.tp_outputs);

      /*trailer length*/
      trailer_length_obj.Run_Trailer_Detector(vehicle_data, raw_detect_list, all_detections, sensors);
      trailer_length_obj.Get_Output(output.tl_outputs);

      /*trailer width*/
      trailer_width_obj.Run_Trailer_Detector(vehicle_data, raw_detect_list, all_detections, sensors);
      trailer_width_obj.Get_Output(output.tw_outputs);

      /*trailer angle*/
      trailer_angle_obj.Set_Trailer_Axle_Length(output.tl_outputs.axle_trailer_length);
      trailer_angle_obj.Run_Trailer_Detector(vehicle_data,elapsed_time_s);
      trailer_angle_obj.Get_Output(output.ta_outputs);

      Post_Processing_Of_Output(vehicle_data);

      timing_info.trailer_detector = get_wall_time() - start_time;
   }

   /*=========================================================================
    * Method         Trailer_Detector_Core::Post_Proessing_Of_Output
    *
    * Description    Privte method to post process the output from the trailer 
                     precence, trailer length, trailer width and trailer angle 
                     estimators
    *
    * Parameters     const F360_Host_T& vehicle_data
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
   void Trailer_Detector_Core::Post_Processing_Of_Output(const F360_Host_T& vehicle_data)
   {
      const bool f_trailer_presence_hardware = vehicle_data.f_trailer_presence_hardware;
      // Clear length, width and angle data if trailer is not present
      if (((TRAILER_PRESENCE_STATE_NOT_DETECTED == output.tp_outputs.trailer_presence) ||
         (TRAILER_PRESENCE_STATE_UNKNOWN == output.tp_outputs.trailer_presence)) &&
         (!f_trailer_presence_hardware))
      {
         output.ta_outputs.trailer_angle_rad = 0.0F;
         output.ta_outputs.trailer_angle_rate_rad = 0.0F;
         output.ta_outputs.confidence = TRAILER_DETECTOR_CONF_UNKNOWN;

         output.tl_outputs.trailer_length = 0.0F;
         output.tl_outputs.confidence = TRAILER_DETECTOR_CONF_UNKNOWN;

         output.tw_outputs.trailer_width = 0.0F;
         output.tw_outputs.confidence = TRAILER_DETECTOR_CONF_UNKNOWN;
      }
      else
      {
         // do nothing
      }

      // Determine trailer detection status
      if (((TRAILER_PRESENCE_STATE_NOT_DETECTED == output.tp_outputs.trailer_presence) && (!f_trailer_presence_hardware))
          || (((TRAILER_PRESENCE_STATE_DETECTED == output.tp_outputs.trailer_presence) || f_trailer_presence_hardware)
              && (output.tl_outputs.f_estimation_done) && (output.tw_outputs.f_estimation_done)))
      {
         output.trailer_detection_status = TRAILER_DETECTOR_STATUS_NOT_RUNNING;
         output.radar_detection_timer = 0U;
      }
      else
      {
         output.trailer_detection_status = TRAILER_DETECTOR_STATUS_RUNNING;
         output.radar_detection_timer += 1U;
      }

      /*assignment*/
      trailer_detector_output.trailer_presence = output.tp_outputs.trailer_presence;
      trailer_detector_output.trailer_presence_conf = output.tp_outputs.confidence;

      trailer_detector_output.trailer_HV_gap = 0.0F;

      trailer_detector_output.trailer_angle = output.ta_outputs.trailer_angle_rad;
      trailer_detector_output.trailer_angle_rate = output.ta_outputs.trailer_angle_rate_rad;
      trailer_detector_output.trailer_angle_conf = TRAILER_DETECTOR_CONF_HIGH;
      trailer_detector_output.trailer_angle_rate_conf = TRAILER_DETECTOR_CONF_HIGH;

      trailer_detector_output.trailer_length = output.tl_outputs.trailer_length;
      trailer_detector_output.trailer_length_conf = TRAILER_DETECTOR_CONF_HIGH;

      trailer_detector_output.trailer_width = output.tw_outputs.trailer_width;
      trailer_detector_output.trailer_width_conf = output.tw_outputs.confidence;

      trailer_detector_output.trailer_detection_status = output.trailer_detection_status;
      trailer_detector_output.radar_detection_timer = output.radar_detection_timer;
      trailer_detector_output.stationary_timer = output.tp_outputs.stationary_timer;
   }

   /*=========================================================================
    * Method         Trailer_Detector_Core::Get_Trailer_Detector_Output
    *
    * Description    Returns the trailer detector output.
    *
    * Parameters     None.
    *
    * Returns        this->trailer_detector_output
    *
    * Externals:     None.
    *
    * Precondition   None.
    *
    * Postcondition  None.
    *
    * Note           None.
    *========================================================================*/
   Trailer_Detector_Flt_Fus_Output Trailer_Detector_Core::Get_Trailer_Detector_Output() const{
      return this->trailer_detector_output;
   }
}
