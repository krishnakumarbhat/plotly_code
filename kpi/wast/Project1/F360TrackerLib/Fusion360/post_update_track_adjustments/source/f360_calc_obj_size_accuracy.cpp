/*===================================================================================*\
* FILE: f360_calc_obj_size_accuracy.cpp
*====================================================================================
* Copyright 2018 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file calculates the innovation and accuracy of an object's length and width
*
* ABBREVIATIONS:
*   
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*
*
* DEVIATIONS FROM STANDARDS:
*
*
\*==========================================================================================*/

#include "f360_calc_obj_size_accuracy.h"
#include "f360_math.h"
#include "f360_math_func.h"
namespace f360_variant_A
{
   /*===========================================================================*\
    * FUNCTION: Calc_Obj_Size_Accuracy
    *===========================================================================
    * RETURN VALUE:
    * None
    *
    * PARAMETERS:
    * const float32_t measured_len
    * const float32_t measured_wid
    * const F360_Calibrations_T & calib
    * F360_Object_Track_T & obj
    *
    * EXTERNAL REFERENCES:
    * None.
    *
    * DEVIATIONS FROM STANDARDS:
    * None.
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * Calculates the accuracy of the object size, based on innovation.
    *
    * PRECONDITIONS:
    * None
    *
    * POSTCONDITIONS:
    * None
    *
    \*===========================================================================*/
   void Calc_Obj_Size_Accuracy(
      const float32_t measured_len,
      const float32_t measured_wid,
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & obj)
   {
      if (obj.f_moveable)
      {
         Calc_Obj_Size_Innovation(measured_len, measured_wid, calib, obj);

         const float32_t eclipse_penalty_factor = 1.0F; // In gdsr, this value is calculated based on number of reference points
         const float32_t filt_coef_acy = 0.3F;          // as in gdsr tracker

         // Penalize if occluded, eclipse_penalty_factor currently set to 1
         const float32_t raw_accuracy_length = obj.innovation_length * eclipse_penalty_factor;
         const float32_t raw_accuracy_width = obj.innovation_width * eclipse_penalty_factor;

         obj.accuracy_length = filt_coef_acy * obj.accuracy_length + (1.0F - filt_coef_acy) * raw_accuracy_length;
         obj.accuracy_width = filt_coef_acy * obj.accuracy_width + (1.0F - filt_coef_acy) * raw_accuracy_width;
      }
      else
      {
         // For non-movable objects, we don't have size measurements. Instead, accuracy is approximately the extension where object can associate to detections.
         const float32_t assoc_gate_diameter = 2.0F * calib.k_max_assoc_gate_radius_non_moveable;
         const float32_t accuracy = assoc_gate_diameter - calib.k_nonmoveable_target_diameter;
         obj.accuracy_length = accuracy;
         obj.accuracy_width = accuracy;
      }
   }

   /*===========================================================================*\
   * FUNCTION: Calc_Obj_Size_Innovation
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t measured_len
   * const float32_t measured_wid
   * const F360_Calibrations_T & calib
   * F360_Object_Track_T & obj
   *
   * EXTERNAL REFERENCES:
   * None.
   *
   * DEVIATIONS FROM STANDARDS:
   * None.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Calculates the innovation of the object size.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calc_Obj_Size_Innovation(
      const float32_t measured_len,
      const float32_t measured_wid,
      const F360_Calibrations_T & calib,
      F360_Object_Track_T & obj)
   {
      if (obj.status > F360_OBJECT_STATUS_INVALID)
      {

         if ((obj.status == F360_OBJECT_STATUS_NEW) ||
            (obj.status == F360_OBJECT_STATUS_NEW_UPDATED) ||
            (obj.status == F360_OBJECT_STATUS_NEW_COASTED))
         {
            obj.innovation_length = calib.k_puta_obj_size_acc_innov_no_update_length;
            obj.innovation_width = calib.k_puta_obj_size_acc_innov_no_update_width;
         }
         else if ((obj.status == F360_OBJECT_STATUS_COASTED) && (obj.time_since_track_updated >= calib.k_hyst_time_for_coasted_objects))
         {
            // Coast towards a constant error
            obj.innovation_length = calib.k_puta_obj_size_acc_filt_coef_innov_coasting_obj * obj.innovation_length + (1.0F - calib.k_puta_obj_size_acc_filt_coef_innov_coasting_obj) * calib.k_puta_obj_size_acc_innov_no_update_length;
            obj.innovation_width = calib.k_puta_obj_size_acc_filt_coef_innov_coasting_obj * obj.innovation_width + (1.0F - calib.k_puta_obj_size_acc_filt_coef_innov_coasting_obj) * calib.k_puta_obj_size_acc_innov_no_update_width;
         }
         else
         {           
            const float32_t error_length = std::abs(measured_len - obj.bbox.Get_Length());
            const float32_t error_width = std::abs(measured_wid - obj.bbox.Get_Width());

            obj.innovation_length = calib.k_puta_obj_size_acc_filt_coef_innov_updated_obj * obj.innovation_length + (1.0F - calib.k_puta_obj_size_acc_filt_coef_innov_updated_obj) * error_length;
            obj.innovation_width = calib.k_puta_obj_size_acc_filt_coef_innov_updated_obj * obj.innovation_width + (1.0F - calib.k_puta_obj_size_acc_filt_coef_innov_updated_obj) * error_width;
         }

      }
   }
}
