/*===================================================================================*\
* FILE: f360_calculate_object_class_probabilities.cpp
*====================================================================================
* Copyright (C) 2020-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains the function to calculate object class probabilities of a track.
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*===================================================================================*/

#include "f360_math.h"
#include "f360_calculate_object_class_probabilities.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: calcAprioriProbability
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * OBJ_CLASS_A_PRIORI_PROBABILITIES_T& apriori - calculated apriori probabilities
   * const F360_Calibrations_T& calib - tracker calibrations
   * const F360_Host_T& vehicle_data - struct containign information about host vehicle
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
   * Function assings apriori probabilities of object being specific class.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void calcAprioriProbability(
      OBJ_CLASS_A_PRIORI_PROBABILITIES_T& apriori,
      const F360_Calibrations_T& calib,
      const F360_Host_T& vehicle_data)
   {

      const float32_t abs_host_speed = std::abs(vehicle_data.speed);

      if (abs_host_speed <= calib.k_ad_oc_boundary_lowspeed)
      {
         apriori.pedestrian = calib.k_ad_oc_apriori_lowspeed_pedestrian;
         apriori.two_wheel = calib.k_ad_oc_apriori_lowspeed_2wheel;
         apriori.car = calib.k_ad_oc_apriori_lowspeed_car;
         apriori.truck = calib.k_ad_oc_apriori_lowspeed_truck;
      }
      else if (abs_host_speed >= calib.k_ad_oc_boundary_highspeed)
      {
         apriori.pedestrian = calib.k_ad_oc_apriori_highspeed_pedestrian;
         apriori.two_wheel = calib.k_ad_oc_apriori_highspeed_2wheel;
         apriori.car = calib.k_ad_oc_apriori_highspeed_car;
         apriori.truck = calib.k_ad_oc_apriori_highspeed_truck;
      }
      else
      {
         /* is host speed between k_oc_boundary_lowspeed and k_oc_boundary_highspeed: compute apriori values with a linear equation y=mx+c
         *  x-axis host speed, y-axis apriori probability
         *  m=(y2-y1)/(x2-x1) c=y1-m x1 -. f(x)=m (x-x1) +y1
         */
         float32_t m;
         const float32_t k_ad_oc_boundary_inv = 1.0F / ((calib.k_ad_oc_boundary_lowspeed - calib.k_ad_oc_boundary_highspeed));
         const float32_t speed_ad_oc_boundary_lowspeed = (abs_host_speed - calib.k_ad_oc_boundary_lowspeed);

         m = (calib.k_ad_oc_apriori_lowspeed_pedestrian - calib.k_ad_oc_apriori_highspeed_pedestrian) * k_ad_oc_boundary_inv;
         apriori.pedestrian = (m*speed_ad_oc_boundary_lowspeed) + calib.k_ad_oc_apriori_lowspeed_pedestrian;

         m = (calib.k_ad_oc_apriori_lowspeed_2wheel - calib.k_ad_oc_apriori_highspeed_2wheel) * k_ad_oc_boundary_inv;
         apriori.two_wheel = (m*speed_ad_oc_boundary_lowspeed) + calib.k_ad_oc_apriori_lowspeed_2wheel;

         m = (calib.k_ad_oc_apriori_lowspeed_car - calib.k_ad_oc_apriori_highspeed_car) * k_ad_oc_boundary_inv;
         apriori.car = (m*speed_ad_oc_boundary_lowspeed) + calib.k_ad_oc_apriori_lowspeed_car;

         m = (calib.k_ad_oc_apriori_lowspeed_truck - calib.k_ad_oc_apriori_highspeed_truck) * k_ad_oc_boundary_inv;
         apriori.truck = (m*speed_ad_oc_boundary_lowspeed) + calib.k_ad_oc_apriori_lowspeed_truck;
      }
   }

   /*===========================================================================*\
   * FUNCTION: evaluateNormalDistribution
   *===========================================================================
   * RETURN VALUE:
   * float32_t - calculated pdf value
   *
   * PARAMETERS:
   * const float32_t value - tested value
   * const float32_t mean - mean value of tested value
   * float32_t standard_deviation - std of tested value
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
   * Function calculates pdf value using normal distribution
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t evaluateNormalDistribution(const float32_t value,
      const float32_t mean,
      float32_t standard_deviation)
   {
      const float32_t tracker_threshold_is_zero = 1e-10F;

      const  float32_t sqrt2pi = 2.506628274631F;
      if (standard_deviation < tracker_threshold_is_zero)
      {
         standard_deviation = 1.0F;
      }
      const float32_t exponent = (value - mean) / standard_deviation;
      return ((1.0F / (sqrt2pi * standard_deviation)) * F360_Expf(-0.5F * exponent * exponent));
   }


   /*===========================================================================*\
   * FUNCTION: calcProbability_Criteria_Bayes
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const OBJ_CLASS_A_PRIORI_PROBABILITIES_T& apriori - apriori probabilities
   * OBJECT_CLASS_PROBABILITY_T& criteria_Bayes - struct containing information about calculated probabilities
   * const F360_Object_Track_T& object - analysed object
   * const F360_Calibrations_T& calib - tracker calibrations
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
   * Function calculates pdf value using normal distribution
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void calcProbability_Criteria_Bayes(
      const OBJ_CLASS_A_PRIORI_PROBABILITIES_T& apriori,
      OBJECT_CLASS_PROBABILITY_T& criteria_Bayes,
      const F360_Object_Track_T& object,
      const F360_Calibrations_T& calib)
   {
      /****************
      * evaluate the probabiliy density function at the current values for length, width and speed
      * assuming that the random variables length, width and speed are independent p(length, with, speed|c_i) simplifies to
      * p(length, with, speed|c_i)=p(length|c_i)*p(width|c_i)*p(speed|c_i)
      **********************/

      const float32_t pdf_value_pedestrian_length = evaluateNormalDistribution(object.bbox.Get_Length(), calib.k_ad_oc_mean_length_pedestrian, calib.k_ad_oc_standard_deviation_length_pedestrian);
      const float32_t pdf_value_pedestrian_width = evaluateNormalDistribution(object.bbox.Get_Width(), calib.k_ad_oc_mean_width_pedestrian, calib.k_ad_oc_standard_deviation_width_pedestrian);
      const float32_t pdf_value_pedestrian_speed = evaluateNormalDistribution(object.speed, calib.k_ad_oc_mean_speed_pedestrian, calib.k_ad_oc_standard_deviation_speed_pedestrian);

      const float32_t pdf_value_pedestrian = pdf_value_pedestrian_length * pdf_value_pedestrian_width * pdf_value_pedestrian_speed;

      const float32_t pdf_value_2wheel_length = evaluateNormalDistribution(object.bbox.Get_Length(), calib.k_ad_oc_mean_length_2wheel, calib.k_ad_oc_standard_deviation_length_2wheel);
      const float32_t pdf_value_2wheel_width = evaluateNormalDistribution(object.bbox.Get_Width(), calib.k_ad_oc_mean_width_2wheel, calib.k_ad_oc_standard_deviation_width_2wheel);
      const float32_t pdf_value_2wheel_speed = evaluateNormalDistribution(object.speed, calib.k_ad_oc_mean_speed_2wheel, calib.k_ad_oc_standard_deviation_speed_2wheel);

      const float32_t pdf_value_2wheel = pdf_value_2wheel_length * pdf_value_2wheel_width * pdf_value_2wheel_speed;

      const float32_t pdf_value_car_length = evaluateNormalDistribution(object.bbox.Get_Length(), calib.k_ad_oc_mean_length_car, calib.k_ad_oc_standard_deviation_length_car);
      const float32_t pdf_value_car_width = evaluateNormalDistribution(object.bbox.Get_Width(), calib.k_ad_oc_mean_width_car, calib.k_ad_oc_standard_deviation_width_car);
      const float32_t pdf_value_car_speed = evaluateNormalDistribution(object.speed, calib.k_ad_oc_mean_speed_car, calib.k_ad_oc_standard_deviation_speed_car);

      const float32_t pdf_value_car = pdf_value_car_length * pdf_value_car_width * pdf_value_car_speed;

      const float32_t pdf_value_truck_length = evaluateNormalDistribution(object.bbox.Get_Length(), calib.k_ad_oc_mean_length_truck, calib.k_ad_oc_standard_deviation_length_truck);
      const float32_t pdf_value_truck_width = evaluateNormalDistribution(object.bbox.Get_Width(), calib.k_ad_oc_mean_width_truck, calib.k_ad_oc_standard_deviation_width_truck);
      const float32_t pdf_value_truck_speed = evaluateNormalDistribution(object.speed, calib.k_ad_oc_mean_speed_truck, calib.k_ad_oc_standard_deviation_speed_truck);

      const float32_t pdf_value_truck = pdf_value_truck_length * pdf_value_truck_width * pdf_value_truck_speed;

      /*********************************************************
      *  Bayes Rule:
      *  c_i class i,  m current measurement,  N number of classes,
      *  P(c_i|m) a posteriori probability of class c_i, p(m|c_i) probability density function of class c_i, P(c_i) a priori probability of class c_i
      *  P(c_i|m)=p(m|c_i)P(c_i)/(sum j=1:N p(m|c_j)P(c_j))
      ************************************************************/
      const float32_t prob_pedestrian = pdf_value_pedestrian * apriori.pedestrian;
      const float32_t prob_2wheel = pdf_value_2wheel * apriori.two_wheel;
      const float32_t prob_car = pdf_value_car * apriori.car;
      const float32_t prob_truck = pdf_value_truck * apriori.truck;
      float32_t sum = prob_pedestrian + prob_2wheel + prob_car + prob_truck;

      // prevent division by zero if sum == 0.
      if (TRACKER_SUM_OF_PDF_THRESHOLD_IS_ZERO > sum)
      {
         sum = 1.0F;
      }

      criteria_Bayes.probability_pedestrian = prob_pedestrian / sum;
      criteria_Bayes.probability_2wheel = prob_2wheel / sum;
      criteria_Bayes.probability_car = prob_car / sum;
      criteria_Bayes.probability_truck = prob_truck / sum;
      criteria_Bayes.probability_unknown = 0.0F;
   }

   /*===========================================================================*\
   * FUNCTION: calcUndetProb
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * F360_Object_Track_T& object - analysed object
   * const F360_Calibrations_T& cals - tracker calibrations
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
   * function calculates probability that object class is undetermined.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void calcUndetProb(
      F360_Object_Track_T& object,
      const F360_Calibrations_T& cals)
   {
      // if object is new, we don't have enough information to be sure of the classification,
      // decrease the probability for unknown if we have seen the object longer
      if ((F360_OBJECT_STATUS_NEW == object.status) || (F360_OBJECT_STATUS_INVALID == object.status) ||
         (F360_OBJECT_STATUS_NEW_UPDATED == object.status) || (F360_OBJECT_STATUS_NEW_COASTED == object.status))
      {
         object.probability_undet = 1.0F;
      }
      else
      {
         object.probability_undet = object.probability_undet - cals.k_ad_oc_step_decrease_prob_unknown;
      }

      if (0.0F > object.probability_undet)
      {
         object.probability_undet = 0.0F;
      }
      else if (1.0F < object.probability_undet)
      {
         object.probability_undet = 1.0F;
      }
      else
      {
         // do nothing
      }
   }

   /*===========================================================================*\
   * FUNCTION: Split_Motorcycle_Prob_Into_Two_Classes
   *===========================================================================
   * RETURN VALUE:
   * F360_Object_Track_T& object - object with modified parameters
   *
   * PARAMETERS:
   * F360_Object_Track_T& object - analysed object
   * const F360_Calibrations_T& calib - tracker calibrations
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
   * Function splits motorcycle probability into motorcycle and bicycle, basing
   * on calibration values and object speed.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Split_Motorcycle_Prob_Into_Two_Classes(
      F360_Object_Track_T& object,
      const F360_Calibrations_T& calib)
   {
      const float32_t prob_2wheeler = object.probability_motorcycle;
      if (object.object_class != F360_OBJ_CLASS_MOTORCYCLE)
      {
         object.probability_motorcycle = 0.5F * prob_2wheeler;
         object.probability_bicycle = 0.5F * prob_2wheeler;
      }
      else
      {
         bool f_slow_moving;
         if (object.speed < calib.k_ad_oc_min_thres_vel_2wheel)
         {
            object.probability_motorcycle = 0.5F * prob_2wheeler;
            object.probability_bicycle = 0.5F * prob_2wheeler;
            f_slow_moving = true;
         }
         else if ((calib.k_ad_oc_min_thres_vel_2wheel < object.speed) &&
            (object.speed < calib.k_ad_oc_max_thres_vel_2wheel))
         {
            object.probability_bicycle = calib.k_ad_oc_weight_frac_2wheel * prob_2wheeler;
            object.probability_motorcycle = (1.0F - calib.k_ad_oc_weight_frac_2wheel) * prob_2wheeler;
            f_slow_moving = false;
         }
         else
         {
            object.probability_bicycle = 0.0F;
            object.probability_motorcycle = prob_2wheeler;
            f_slow_moving = false;
         }

         if (calib.k_ad_oc_min_prob_winner_class <= object.probability_motorcycle)
         {
            object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
         }
         // Use either regular classification threshold for winner class or lowered threshold to classify low speed 2wheel objects as a bicycle
         // Second condition has been added to allow for bicycle classification without changing probability values, which would trigger a need
         // for classification fusion retraining; more details can be found in DFT-1822  
         else if ((calib.k_ad_oc_min_prob_winner_class <= object.probability_bicycle) ||
            (f_slow_moving && (calib.k_ad_oc_lowered_min_prob_winner_class <= object.probability_bicycle)))
         {
            object.object_class = F360_OBJ_CLASS_BICYCLE;
         }
         else
         {
            object.object_class = F360_OBJ_CLASS_UNDETERMINED;
         }
      }

   }

   /*===========================================================================*\
   * FUNCTION: Set_Object_Class
   *===========================================================================
   * RETURN VALUE:
   * F360_Object_Track_T& object - object with modified parameters
   *
   * PARAMETERS:
   * F360_Object_Track_T& object - analysed object
   * const float32_t min_prob - probability value threshold
   * const float32_t undet_scaling_factor - scaling factor used to inrease undetermined probability value
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
   * Function sets object class basing on calculated probabilities values.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Set_Object_Class(
      F360_Object_Track_T& object,
      const float32_t min_prob)
   {
      if (min_prob <= object.probability_pedestrian)
      {
         object.object_class = F360_OBJ_CLASS_PEDESTRIAN;
      }
      else if (min_prob <= object.probability_motorcycle)
      {
         object.object_class = F360_OBJ_CLASS_MOTORCYCLE;
      }
      else if (min_prob <= object.probability_car)
      {
         object.object_class = F360_OBJ_CLASS_CAR;
      }
      else if (min_prob <= object.probability_truck)
      {
         object.object_class = F360_OBJ_CLASS_TRUCK;
      }
      else 
      {
         object.object_class = F360_OBJ_CLASS_UNDETERMINED;
      }
   }

   /*===========================================================================*\
   * FUNCTION: filteringAndNormalizationOfProbabilities
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * F360_Object_Track_T& object - analysed object
   * const OBJECT_CLASS_PROBABILITY_T& criteria_Bayes - calculated probabilities innovation
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
   * Function filters probabilities using first order filter and normalizes them so they sum up to 1.0F
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void filteringAndNormalizationOfProbabilities(
      F360_Object_Track_T& object,
      const OBJECT_CLASS_PROBABILITY_T& criteria_Bayes)
   {
      const float32_t alpha = 0.98F;

      // use a low pass filter to prevent quick switches between classes. add wheelspin later.
      object.probability_pedestrian = (alpha * object.probability_pedestrian) + ((1.0F - alpha) * criteria_Bayes.probability_pedestrian);
      object.probability_motorcycle = (alpha * object.probability_motorcycle) + ((1.0F - alpha) * criteria_Bayes.probability_2wheel);
      object.probability_car = (alpha * object.probability_car) + ((1.0F - alpha) * criteria_Bayes.probability_car);
      object.probability_truck = (alpha * object.probability_truck) + ((1.0F - alpha) * criteria_Bayes.probability_truck);

      // normalize
      float32_t sum = object.probability_pedestrian + object.probability_motorcycle
         + object.probability_car + object.probability_truck;

      // prevent division by zero if sum == 0.
      if (TRACKER_SUM_OF_PDF_THRESHOLD_IS_ZERO > sum)
      {
         sum = 1.0F;
      }

      /* /sum to make sum(ped, 2wheel, car, truck)=1
      factor (1.0f - object.object_class_probability.reserved_value_5) to decrease (reserved_value_1, 2wheel, car, truck) so that
      sum(reserved_value_1, 2wheel, car, truck, unknown)=1 without changing reserved_value_5*/
      const float32_t normalize_factor = (1.0F - object.probability_undet) / sum;

      object.probability_pedestrian = object.probability_pedestrian * normalize_factor;
      object.probability_motorcycle = object.probability_motorcycle * normalize_factor;
      object.probability_car = object.probability_car * normalize_factor;
      object.probability_truck = object.probability_truck * normalize_factor;
   }

   /*===========================================================================*\
   * FUNCTION: calcProbability
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * F360_Object_Track_T& object - analysed object
   * const OBJ_CLASS_A_PRIORI_PROBABILITIES_T& apriori - apriori probabilities
   * const F360_Calibrations_T& calib - tracker calibrations
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
   * Function calculates probabilities of object belonging to defined class and assigns
   * winner class to object.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void calcProbability(
      F360_Object_Track_T& object,
      const OBJ_CLASS_A_PRIORI_PROBABILITIES_T& apriori,
      const F360_Calibrations_T& calib)
   {
      OBJECT_CLASS_PROBABILITY_T criteria_Bayes{};

      // Set sum probability of motorbike to sum of motorbike and bicycle since the algorithms will treat all two wheelers equaly initially.
      // The probability of motorbike will be split into motorbike and bicycle again towards the end of the algorithm.
      object.probability_motorcycle = object.probability_bicycle + object.probability_motorcycle;
      object.probability_bicycle = 0.0F;

      // Compute probability based on each criteria
      calcProbability_Criteria_Bayes(apriori, criteria_Bayes, object, calib);

      // Set the probability for undetermined class, depening on for how long we have been tracking this object
      calcUndetProb(object, calib);

      // Filter and normalize the probabilities
      filteringAndNormalizationOfProbabilities(object, criteria_Bayes);

      Update_Probability_Undetermined(calib, object);

      Set_Object_Class(object, calib.k_ad_oc_min_prob_winner_class);

      Split_Motorcycle_Prob_Into_Two_Classes(object, calib);
   }


   /*===========================================================================*\
   * FUNCTION: Update_Probability_Undetermined
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const F360_Calibrations_T& calibs - tracker calibrations
   * F360_Object_Track_T& object - analysed object
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
   * Function improves classification of slow moving object performing
   * speed-based recognition of pedestrians and stationary objects.
   * 
   * PRECONDITIONS:
   * The function expects probability of pedestrian not to be greater than 1.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_Probability_Undetermined(
      const F360_Calibrations_T& calib,
      F360_Object_Track_T& object)
   {
      const bool object_speed_in_range_stationary = (calib.k_ad_oc_min_pedestrian_speed < object.speed) && (object.speed < calib.k_ad_oc_max_stationary_speed);

      //increase probability of undetermined
      if ((object_speed_in_range_stationary || (object.speed < calib.k_ad_oc_min_pedestrian_speed)) && (!object.f_moveable)
            && ((F360_OBJ_CLASS_UNDETERMINED == object.object_class) || (F360_OBJ_CLASS_PEDESTRIAN == object.object_class)))
      {
         object.probability_car *= calib.k_ad_oc_prob_decrease;
         object.probability_pedestrian *= calib.k_ad_oc_prob_decrease;
         object.probability_motorcycle *= calib.k_ad_oc_prob_decrease;
         object.probability_bicycle *= calib.k_ad_oc_prob_decrease;
         object.probability_truck *= calib.k_ad_oc_prob_decrease;

         object.probability_undet = 1.0F - (object.probability_pedestrian + object.probability_car + object.probability_motorcycle +
            object.probability_bicycle + object.probability_truck);
      }

      //increase probability of pedestrian
      if ((calib.k_ad_oc_min_prob_pedestrian < object.probability_pedestrian) && (calib.k_ad_oc_min_pedestrian_speed < object.speed) &&
         (object.speed < calib.k_ad_oc_max_pedestrian_speed) && ((F360_OBJ_CLASS_UNDETERMINED == object.object_class) || (F360_OBJ_CLASS_PEDESTRIAN == object.object_class)))
      {
         object.probability_car *= calib.k_ad_oc_prob_decrease;
         object.probability_motorcycle *= calib.k_ad_oc_prob_decrease;
         object.probability_bicycle *= calib.k_ad_oc_prob_decrease;
         object.probability_truck *= calib.k_ad_oc_prob_decrease;
         object.probability_undet *= calib.k_ad_oc_prob_decrease;
      }
   }

   /*===========================================================================*\
   * FUNCTION: run_obj_class
   *===========================================================================
   * RETURN VALUE:
   * F360_Object_Track_T& object - object with modified parameters
   *
   * PARAMETERS:
   * F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS] - object tracks array
   * const F360_Host_T& vehicle_data - struct containing information about host vehicle
   * const F360_Tracker_Info_T& tracker_info - struct containing information about tracker
   * const F360_Calibrations_T& calibs - tracker calibrations
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
   * Function sets object class basing on calculated probabilities values.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void run_obj_class(
      F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Host_T& vehicle_data,
      const F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calibs)
   {
      OBJ_CLASS_A_PRIORI_PROBABILITIES_T apriori;

      calcAprioriProbability(apriori, calibs, vehicle_data);

      for (int32_t num_obj_active = 0; num_obj_active < tracker_info.num_active_objs; num_obj_active++)
      {
         const int32_t obj_trk_idx = tracker_info.active_obj_ids[num_obj_active] - 1;
         calcProbability(object_tracks[obj_trk_idx], apriori, calibs);
      }
   }
}
