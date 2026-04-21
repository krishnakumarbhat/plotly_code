/*===========================================================================*\
 * FILE: f360_trailer_detector_core.h
 *============================================================================
 * Copyright (C) 2022 Aptiv Advanced Safety and User Experience. All rights reserved.
 * Confidential - Restricted Aptiv information. Do not disclose.
 *-----------------------------------------------------------------------------------------
 * DESCRIPTION:
 *   This file defines trailer detector class which is the entry to trailer detector feature
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
 *     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
 *
\*==========================================================================================*/
#ifndef TRAILER_DETECTOR_CORE_H
#define TRAILER_DETECTOR_CORE_H

#include "f360_host.h"
#include "f360_constants.h"
#include "f360_detection_props.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_trailer_detector_TP.h"
#include "f360_trailer_detector_TL.h"
#include "f360_trailer_detector_TW.h"
#include "f360_trailer_detector_TA.h"
#include "f360_trailer_detector_flt_fus_output.h"
#include "f360_trailer_detector_outputs.h"
#include "f360_timing_info.h"

namespace f360_variant_A
{
   class Trailer_Detector_Core
   {
   public:
      Trailer_Detector_Core():
         trailer_detector_output{},
         output{}
      {
         Initialize();
      };

      ~Trailer_Detector_Core() {};

      void Initialize();

      void Run_Trailer_Detector(const F360_Host_T &vehicle_data, 
         const rspp_variant_A::RSPP_Detection_List_T &raw_detect_list,
         const F360_Detection_Props_T (&all_detections)[MAX_NUMBER_OF_DETECTIONS],
         const F360_Radar_Sensor_T(&sensors)[MAX_NUMBER_OF_SENSORS],
         const float32_t elapsed_time_s,
         F360_TRKR_TIMING_INFO_T& timing_info);

      Trailer_Detector_Flt_Fus_Output Get_Trailer_Detector_Output() const;

   protected:

      void Post_Processing_Of_Output(const F360_Host_T& vehicle_data);

      Trailer_Detector_Flt_Fus_Output trailer_detector_output;
      Trailer_Detector_Outputs output;
   
   private:
      Trailer_Detector_TP trailer_presence_obj;
      Trailer_Detector_TL trailer_length_obj;
      Trailer_Detector_TW trailer_width_obj;
      Trailer_Detector_TA trailer_angle_obj;

   // debugging purpose only
   public:
      const Trailer_Detector_TP& Get_Trailer_Presence_Instance() const {return this->trailer_presence_obj;};
      const Trailer_Detector_TL& Get_Trailer_Length_Instance() const {return this->trailer_length_obj;};
      const Trailer_Detector_TW& Get_Trailer_Width_Instance() const {return this->trailer_width_obj;};
      const Trailer_Detector_TA& Get_Trailer_Angle_Instance() const {return this->trailer_angle_obj;};
      const Trailer_Detector_Outputs& Get_Trailer_Output_Instance() const {return this->output;};
      void Set_Trailer_Presence_Instance(const Trailer_Detector_TP& trailer_presence_obj_input) { this->trailer_presence_obj = trailer_presence_obj_input; };
      void Set_Trailer_Length_Instance(const Trailer_Detector_TL& trailer_length_obj_input) { this->trailer_length_obj = trailer_length_obj_input; };
      void Set_Trailer_Width_Instance(const Trailer_Detector_TW& trailer_width_obj_input) { this->trailer_width_obj = trailer_width_obj_input; };
      void Set_Trailer_Angle_Instance(const Trailer_Detector_TA& trailer_angle_obj_input) { this->trailer_angle_obj = trailer_angle_obj_input; };
      void Set_Trailer_Output_Instance(const Trailer_Detector_Outputs& trailer_output_obj_input) { this->output = trailer_output_obj_input; };
      
   };
}
#endif
