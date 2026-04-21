/*===========================================================================*\
* FILE: f360_sensor_capability.cpp
*============================================================================
* Copyright (C) 2019-2022 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*----------------------------------------------------------------------------
* DESCRIPTION:
*   This file contains function defenitions for computing and propagating sensor uncertainties
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "Aptiv C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*===========================================================================*/
#include "f360_math.h"
#include <cstring>
#include "f360_sensor_capability.h"
#include "f360_math_func.h"
#include "f360_constants.h"
#include "rspp_detection.h"
#include "f360_detection_props.h"
#include "f360_radar_sensor.h"
#include "f360_radar_sensor_props.h"
#include "f360_det_cross_covariances.h"
#include "f360_get_wall_time.h"
namespace f360_variant_A
{
   static void Polar_To_Cartesian_Linearization_Transform(
      const float32_t magnitude,
      const float32_t cos_angle,
      const float32_t sin_angle,
      const float32_t var_magnitude,
      const float32_t var_angle,
      float32_t (&cov_xy)[2][2]
   );

   /*===========================================================================*\
   * FUNCTION: Get_Uncertainty_Of_Detection_Position()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t det_rng
   * const float32_t cos_det_az
   * const float32_t sin_det_az
   * const float32_t var_det_rng
   * const float32_t var_det_az
   * float32_t (&cov_det_pos)[2][2])
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
   * This function propagates uncertainty of detection, range, detection azimuth
   * and sensor mounting position into uncertainty of detection VCS position.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Uncertainty_Of_Detection_Position(
      const float32_t det_rng,
      const float32_t cos_det_az,
      const float32_t sin_det_az,
      const float32_t var_det_rng,
      const float32_t var_det_az,
      float32_t (&cov_det_pos)[2][2])
   {
      Polar_To_Cartesian_Linearization_Transform(det_rng, cos_det_az, sin_det_az, var_det_rng, var_det_az, cov_det_pos);
   }

   /*===========================================================================*\
   * FUNCTION: Polar_To_Cartesian_Linearization_Transform()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t magnitude
   * const float32_t cos_angle
   * const float32_t sin_angle
   * const float32_t var_magnitude
   * const float32_t var_angle
   * float32_t cov_xy[2][2])
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
   * This function takes detection range and detection azimuth and corresponding
   * variance measures and computes covariance matrix using linearization.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   static void Polar_To_Cartesian_Linearization_Transform(
      const float32_t magnitude,
      const float32_t cos_angle,
      const float32_t sin_angle,
      const float32_t var_magnitude,
      const float32_t var_angle,
      float32_t (&cov_xy)[2][2])
   {
      float32_t orig_cov[2][2] = {};
      float32_t jacobian[2][2] = {};
      float32_t temp_mat[2][2] = {};

      orig_cov[0][0] = var_magnitude;
      orig_cov[0][1] = 0.0F;
      orig_cov[1][0] = 0.0F;
      orig_cov[1][1] = var_angle;

      jacobian[0][0] = cos_angle; // (d/(d magnitude)) magnitude*cos(angle)
      jacobian[0][1] = -magnitude * sin_angle; // (d/(d angle)) magnitude*cos(angle)
      jacobian[1][0] = sin_angle; // (d/(d magnitude)) magnitude*sin(angle)
      jacobian[1][1] = magnitude * cos_angle; // (d/(d angle)) magnitude*sin(angle)

      F360_Matmul_MxN_NxP(jacobian, orig_cov, temp_mat);
      F360_Matmul_MxN_PxN_Transpose(temp_mat, jacobian, cov_xy);
   }

   /*===========================================================================*\
   * FUNCTION: Get_Uncertainty_Of_Compensated_Range_Rate()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t cos_det_az
   * const float32_t sin_det_az
   * const float32_t (&sens_vel)[2]
   * const float32_t var_det_rng_rate
   * const float32_t var_det_az
   * const float32_t (&cov_sens_vel)[2][2]
   * float32_t & var_comp_rng_rate
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
   * This function propagates uncertainty of sensor velocity, detection range
   * rate and detection azimuth angle into uncertainty of detection compensated
   * range rate.
   *
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Uncertainty_Of_Compensated_Range_Rate(
      const float32_t cos_det_az,
      const float32_t sin_det_az,
      const float32_t (&sens_vel)[2],
      const float32_t var_det_rng_rate,
      const float32_t var_det_az,
      const float32_t (&cov_sens_vel)[2][2],
      float32_t &var_comp_rng_rate)
   {
      float32_t det_cov[4][4] = { 0.0F };
      float32_t jacobian[1][4];
      float32_t temp_mat[1][4];
      float32_t temp_var_comp_rng_rate[1][1];

      /* Equation for computing compensated range rate is:
      comp_rng_rate = rng_rate + sens_vel_x*cos(az) + sens_vel_y*sin(az)
      Assume uncertainty in rng_rate, sens_vel and az. Linearize the above
      equation and assume normal distributed variables to get the uncertainty
      in comp_range_rate */

      // Setup covariance of [rng_rate, az, sens_vel_x, sens_vel_y]^T
      det_cov[0][0] = var_det_rng_rate;
      det_cov[1][1] = var_det_az;
      det_cov[2][2] = cov_sens_vel[0][0];
      det_cov[2][3] = cov_sens_vel[0][1];
      det_cov[3][2] = cov_sens_vel[1][0];
      det_cov[3][3] = cov_sens_vel[1][1];

      // Compute jacobian (linearization)
      jacobian[0][0] = 1.0F; // d comp_rng_rate / d rng_rate
      jacobian[0][1] = -sens_vel[0] * sin_det_az + sens_vel[1] * cos_det_az; // d comp_rng_rate / d az
      jacobian[0][2] = cos_det_az; // d comp_rng_rate / d sens_vel_x
      jacobian[0][3] = sin_det_az; // d comp_rng_rate / d sens_vel_x

      // Propagate uncertainty
      F360_Matmul_MxN_NxP(jacobian, det_cov, temp_mat);
      F360_Matmul_MxN_PxN_Transpose(temp_mat, jacobian, temp_var_comp_rng_rate);

      var_comp_rng_rate = temp_var_comp_rng_rate[0][0];
   }

   /*===========================================================================*\
   * FUNCTION: Compute_Raw_Host_Speed_Uncertainty()
   *===========================================================================
   * RETURN VALUE:
   * float32_t host_speed_var - variance of host speed signal
   *
   * PARAMETERS:
   * const F360_Host_T &host
   * const F360_Globals_T &globals
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
   * This function models the uncertainties of the following two host signals:
   *    - host speed at center of rear axle
   *
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Compute_Raw_Host_Speed_Uncertainty(
      const F360_Host_T &host,
      const float32_t max_otg_speed)
   {
      /* Use the following equations and assume uncertainty in speed_correction_factor,
         raw_speed and raw_yaw_rate:
            - host->speed = (speed_correction_factor)*raw_speed
         Propagate the uncertainties to host->speed by
         linearizing the above equations and assuming normal distributed variables. */
      float32_t raw_speed;
      const float32_t speed_correction_factor = host.speed_correction_factor;
      const float32_t speed_compensation_factor = speed_correction_factor;

      if (std::abs(speed_compensation_factor) < F360_EPSILON)
      {
         raw_speed = max_otg_speed;
      }
      else
      {
         raw_speed = host.speed / speed_compensation_factor;
      }

      /* Modeled uncertainties of raw measured host signals.
         Can be considered to be tuning variables of the sensor capability
         module. */
      const float32_t raw_speed_var = 1e-4F; // [(m/s)^]
      const float32_t speed_correction_factor_var = 1e-5F;

      /* Propagate raw uncertainties */
      const float32_t host_speed_var = speed_compensation_factor * speed_compensation_factor*raw_speed_var +
         raw_speed * raw_speed*speed_correction_factor_var;
      return host_speed_var;
   }

   /*===========================================================================*\
   * FUNCTION: Compute_Raw_Detection_Uncertainty()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &det
   * const F360_Radar_Sensor_T &sensor
   * const F360_Radar_Sensor_Props_T &sensor_props
   * const RSPP_Calibrations_T &rspp_calibrations
   * float32_t &range_var
   * float32_t &azimuth_var
   * float32_t &range_rate_var
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
   * This function models the uncertainties of the radar measurements:
   *    - range
   *    - range rate
   *    - azimuth
   *
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_Raw_Detection_Uncertainty(
      const rspp_variant_A::RSPP_Detection_T &det,
      const F360_Radar_Sensor_T &sensor,
      const RSPP_Calibrations_T & rspp_calibrations,
      float32_t &range_var,
      float32_t &azimuth_var,
      float32_t &range_rate_var)
   {
      const F360_Det_Look_ID_T look_id = sensor.variable.look_id;

      // Variables for modeling azimuth variance
      const float32_t frac_az = 0.04F;
      const float32_t az_safety_margin = F360_DEG2RAD(5.0F);
      float32_t std_vec[7];           // NUM_AZ_BREAKPOINTS
      float32_t az_breakpoint_vec[7]; // NUM_AZ_BREAKPOINTS
      float32_t max_interior_fov;
      float32_t min_interior_fov;
      const float32_t det_az = det.raw.azimuth;
      float32_t azimuth_std;

      // Variables for modeling range and range rate variance
      const float32_t range_std = rspp_calibrations.k_range_std;
      const float32_t range_rate_std = rspp_calibrations.k_range_rate_std;

      // Get limits for interior FOV and maximum possible FOV.
      Get_Limits_For_FOV(Get_Range_Type(look_id), sensor, min_interior_fov, max_interior_fov);

      // Set model parameters depending on sensor type
      Compute_Azimuth_Breakpoints(sensor.constant.fov_min_az_rad[look_id], az_safety_margin, min_interior_fov, frac_az, max_interior_fov, sensor.constant.fov_max_az_rad[look_id], az_breakpoint_vec);

      Compute_Azimuth_Std_Vec_Based_On_Sensor_Type(sensor.constant.sensor_type, std_vec);

      // Compute azimuth std
      azimuth_std = F360_Piecewise_Linear_Equation(det_az, az_breakpoint_vec, std_vec);

      // Compute variances
      range_var = range_std * range_std;
      azimuth_var = azimuth_std * azimuth_std;
      range_rate_var = range_rate_std * range_rate_std;
   }


   /*===========================================================================*\
   * FUNCTION: Get_Limits_For_FOV()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Det_Range_Type_T range_type,
   * const F360_Radar_Sensor_Props_T &sensor_props,
   * float32_t &min_interior_fov,
   * float32_t &max_interior_fov
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
   * This function gets limits for sensor field of view.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Limits_For_FOV(
      const F360_Det_Range_Type_T range_type,
      const F360_Radar_Sensor_T &sensor,
      float32_t &min_interior_fov,
      float32_t &max_interior_fov
   )
   {
      if (F360_DET_RANGE_TYPE_MEDIUM == range_type)
      {
         min_interior_fov = sensor.constant.interior_fov[F360_DET_LOOK_ID_2];
         max_interior_fov = sensor.constant.interior_fov[F360_DET_LOOK_ID_3];
      }
      else
      {
         min_interior_fov = sensor.constant.interior_fov[F360_DET_LOOK_ID_0];
         max_interior_fov = sensor.constant.interior_fov[F360_DET_LOOK_ID_1];
      }
   }

   void Compute_Azimuth_Std_Vec_Based_On_Sensor_Type(
      const F360_Sensor_Type_T &sensor_type,
      float32_t (&std_vec)[7])
   {
      if ((F360_SENSOR_TYPE_MRR360_RADAR == sensor_type) ||
         (F360_SENSOR_TYPE_MRR3_RADAR == sensor_type))
      {
         std_vec[0] = F360_DEG2RAD(1.0F);
         std_vec[1] = F360_DEG2RAD(1.0F);
         std_vec[2] = F360_DEG2RAD(0.5F);
         std_vec[3] = F360_DEG2RAD(0.5F);
         std_vec[4] = F360_DEG2RAD(0.5F);
         std_vec[5] = F360_DEG2RAD(1.0F);
         std_vec[6] = F360_DEG2RAD(1.0F);
      }
      else
      {
         std_vec[0] = F360_DEG2RAD(2.0F);
         std_vec[1] = F360_DEG2RAD(2.0F);
         std_vec[2] = F360_DEG2RAD(1.0F);
         std_vec[3] = F360_DEG2RAD(1.0F);
         std_vec[4] = F360_DEG2RAD(1.0F);
         std_vec[5] = F360_DEG2RAD(2.0F);
         std_vec[6] = F360_DEG2RAD(2.0F);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Compute_Azimuth_Breakpoints()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t &min_fov,
   * const float32_t &az_safety_margin,
   * const float32_t &min_interior_fov,
   * const float32_t &frac_az,
   * const float32_t &max_interior_fov,
   * const float32_t &max_fov,
   * float32_t (&az_breakpoint_vec)[7]
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
   * This function computes azimuth breakpoints.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_Azimuth_Breakpoints(
      const float32_t &min_fov,
      const float32_t &az_safety_margin,
      const float32_t &min_interior_fov,
      const float32_t &frac_az,
      const float32_t &max_interior_fov,
      const float32_t &max_fov,
      float32_t (&az_breakpoint_vec)[7])
   {
      az_breakpoint_vec[0] = min_fov - az_safety_margin;
      az_breakpoint_vec[1] = min_interior_fov;
      az_breakpoint_vec[2] = (1.0F - frac_az)*min_interior_fov;
      az_breakpoint_vec[3] = 0.0F;
      az_breakpoint_vec[4] = (1.0F - frac_az)*max_interior_fov;
      az_breakpoint_vec[5] = max_interior_fov;
      az_breakpoint_vec[6] = max_fov + az_safety_margin;
   }

   /*===========================================================================*\
   * FUNCTION: Get_Host_Velocity_Uncertainty()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T &host
   * const float32_t host_speed_var
   * const float32_t host_yaw_rate_var
   * const float32_t (&translation_vec)[2]
   * float32_t (&velocity_cov)[2][2]
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
   * This function propagates uncertainty of host speed and yaw rate at center
   * of rear axle to uncertainty of host velocity vector in any given point of
   * host. The input translation_vec corresponds to the vector from the origin
   * of the VCS coordinate system to the point of interest. As en example the
   * point of interest could for example be the mounting position of a sensor
   * and this function would then compute the uncertainty of the sensor
   * velocity.
   *
   * translation_vec should be given in VCS coordinates and the returned
   * velocity_cov is also in the VCS system.
   *
   * PRECONDITIONS:
   * host_speed_var and host_yaw_rate_var must not be negative.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Host_Velocity_Uncertainty(
      const F360_Host_T &host,
      const float32_t host_speed_var,
      const float32_t host_yaw_rate_var,
      const float32_t (&translation_vec)[2],
      float32_t (&velocity_cov)[2][2])
   {
      /* Use the following rigid body mechanics equations:
            v_B = v_A + w x r_A\B
         where
            v_B = velocity in point B
            v_A = velocity in point A
            w = yaw rate of rigid body
            r_A\B = vector from point A to point B

         For host we have:
            v_A = velocity vector at center of rear axle =
                = speed*[cos(rear_side_slip), sin(rear_sideslip)]^T
         where
            rear_sideslip = -rear_compliance_factor*speed*yaw_rate
         and where
            r_A\B = [dist_rear_axle_to_vcs,]^T + translation_vec
         Assume uncertainty in speed, yawrate and rear_cornering_compliance
         and propagate to host velocity vector. */

         // Extract host properties
      const float32_t speed = host.speed;
      const float32_t speed_sq = speed * speed;
      const float32_t yaw_rate = host.yaw_rate_rad;
      const float32_t dist_rear_axle_to_vcs = host.dist_rear_axle_to_vcs_m;
      const float32_t rear_cornering_compliance = host.rear_cornering_compliance;
      const float32_t rear_sideslip = -rear_cornering_compliance * speed * yaw_rate;
      const float32_t cos_rear_sideslip = F360_Cosf(rear_sideslip);
      const float32_t sin_rear_sideslip = F360_Sinf(rear_sideslip);

      /* Set maximum bias for host rear cornering compliance factor and
         host distance from rear axle to VCS. Can be considered to be
         tuning variables of the sensor capability module. */
      const float32_t max_bias_rear_cornering_complience = 0.005F;

      // Matrices for variance propagation
      float32_t jacobian[2][3];
      float32_t temp_mat[2][3];
      float32_t host_cov_matrix[3][3] = { 0.0F };

      // Set covariance matrix for "raw" host signals
      host_cov_matrix[0][0] = host_speed_var;
      host_cov_matrix[1][1] = host_yaw_rate_var;
      host_cov_matrix[2][2] = max_bias_rear_cornering_complience * max_bias_rear_cornering_complience;

      // Compute the Jacobian
      // Derivative of longitudinal velocity w.r.t. host speed
      jacobian[0][0] = cos_rear_sideslip + rear_cornering_compliance * yaw_rate * speed * sin_rear_sideslip;
      // Derivative of longitudinal velocity w.r.t. yaw_rate
      jacobian[0][1] = rear_cornering_compliance * speed_sq * sin_rear_sideslip - translation_vec[1];
      // Derivative of longitudinal velocity w.r.t. rear cornering compliance factor
      jacobian[0][2] = yaw_rate * speed_sq * sin_rear_sideslip;

      // Derivative of lateral velocity w.r.t. host speed
      jacobian[1][0] = sin_rear_sideslip - rear_cornering_compliance * yaw_rate * speed * cos_rear_sideslip;
      // Derivative of lateral velocity w.r.t. yaw_rate
      jacobian[1][1] = -rear_cornering_compliance * speed_sq * cos_rear_sideslip + dist_rear_axle_to_vcs + translation_vec[0];
      // Derivative of lateral velocity w.r.t. rear cornering compliance factor
      jacobian[1][2] = -yaw_rate * speed_sq * cos_rear_sideslip;

      // Propagate uncertainty of "raw" host signals to uncertainty of host velocity
      F360_Matmul_MxN_NxP(jacobian, host_cov_matrix, temp_mat);
      F360_Matmul_MxN_PxN_Transpose(temp_mat, jacobian, velocity_cov);
   }


   /*===========================================================================*\
   * FUNCTION: Compute_Detection_Cross_Covariances()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t (&sens_vel_vcs)[2]             - vector with velocity of sensor in VCS
   * const float32_t var_azimuth_vcs                - variance of detection VCS azimuth
   * RSPP_Detection_T &detection                     - reference to detection to be processed
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
   * This function computes the following cross covariances and adds the
   * information into the detection property data structure:
   *    - VCS position x and VCS azimuth
   *    - VCS position x and compensated range rate
   *    - VCS position y and VCS azimuth
   *    - VCS position y and compensated range rate
   *    - VCS azimuth and compensated range rate
   *
   * EXTERNAL REFERENCES:
   * Derivation of how cross covariances can be computed can be found in the document
   * Final_cross_covariance_concept.pdf which is attached to the Jira ticket DFD-97,
   * http://jiraprod1.delphiauto.net:8080/browse/DFD-97
   *
   * PRECONDITIONS:
   * Variances must be positive and covariance matrices must be positive definite.
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Compute_Detection_Cross_Covariances(
      const float32_t (&sens_vel_vcs)[2],
      const float32_t var_azimuth_vcs,
      rspp_variant_A::RSPP_Detection_T &detection)
   {
      // Compute cross covariances in VCS coordinate system
      /* Transformation:
      *     z = h(y)
      *     where
      *        y = [range, raw_range_rate, az_vcs, x_sens_vel_vcs, y_sens_vel_vcs]'
      *     and
      *        z = [x_vcs, y_vcs, az_vcs, comp_range_rate]'.
      *     Description of the h() function:
      *        x_vcs = range*cos(az_vcs) + x_translation
      *        y_vcs = range*sin(az_vcs) + y_translation
      *        az_vcs = az_vcs
      *        comp_range_rate = raw_range_rate + x_sens_vel_vcs*cos(az_vcs) + y_sens_vel_vcs*cos(az_vcs)
      *     where x_translation and y_translation are known, non-stochastic translation between VCS origin and sensor
      */

      // The elements of the vector y
      const float32_t range = detection.raw.range;
      const float32_t x_sens_vel_vcs = sens_vel_vcs[0];
      const float32_t y_sens_vel_vcs = sens_vel_vcs[1];

      // The elements of the Jacobian of the transformation w.r.t. az_vcs
      const float32_t d_x_vcs_d_az_vcs = -range * detection.processed.sin_vcs_az; // Derivative of x_vcs w.r.t. az_vcs
      const float32_t d_y_vcs_d_az_vcs = range * detection.processed.cos_vcs_az; // Derivative of y_vcs w.r.t. az_vcs
      const float32_t d_az_vcs_d_az_vcs = 1.0F; // Derivative of az_vcs w.r.t. az_vcs
      const float32_t d_comp_range_rate_d_az_vcs = -x_sens_vel_vcs * detection.processed.sin_vcs_az + y_sens_vel_vcs * detection.processed.cos_vcs_az; // Derivative of comp_range_rate w.r.t. az_vcs

      // Compute cross covariances. This is done according to reference [1] found in function header
      const float32_t cov_x_vcs_az_vcs = d_x_vcs_d_az_vcs * d_az_vcs_d_az_vcs*var_azimuth_vcs;
      const float32_t cov_x_vcs_comp_range_rate = d_x_vcs_d_az_vcs * d_comp_range_rate_d_az_vcs*var_azimuth_vcs;

      const float32_t cov_y_vcs_az_vcs = d_y_vcs_d_az_vcs * d_az_vcs_d_az_vcs*var_azimuth_vcs;
      const float32_t cov_y_vcs_comp_range_rate = d_y_vcs_d_az_vcs * d_comp_range_rate_d_az_vcs*var_azimuth_vcs;

      const float32_t cov_az_vcs_comp_range_rate = d_az_vcs_d_az_vcs * d_comp_range_rate_d_az_vcs*var_azimuth_vcs;

      detection.processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_X_VCS_AZ] = cov_x_vcs_az_vcs;
      detection.processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_X_VCS_COMP_RANGE_RATE] = cov_x_vcs_comp_range_rate;
      detection.processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_Y_VCS_AZ] = cov_y_vcs_az_vcs;
      detection.processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_Y_VCS_COMP_RANGE_RATE] = cov_y_vcs_comp_range_rate;
      detection.processed.vcs_cross_covariances_scm[F360_DET_CROSS_COV_VCS_AZ_VCS_COMP_RANGE_RATE] = cov_az_vcs_comp_range_rate;
   }

   /*===========================================================================*\
   * FUNCTION: Sensor_Capability_Host()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const float32_t elapsed_time
   * const F360_Host_T &host
   * const F360_Globals_T &globals
   * F360_Host_Props_T &host_props
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This functions computes uncertainties for some host signals and stores the
   * information in the host properties data structure. Uncertanties for the
   * following host signals are computed:
   * - Host speed at center of rear axle (corresponding field in host properties
   *   data structure is speed_variance)
   * - Host yaw rate (corresponding field in host properties data structure is
   *   yaw_rate_variance)
   * - Host WCS velocity vector in front center (corresponding field in host
   *   properties data structure is vel_cov_scm)
   * - Host position difference since last tracker iteration (corresponding
   *   field in host properties data structure is position_inc_ov_scm)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Sensor_Capability_Host(
      const float32_t elapsed_time,
      const F360_Host_T &host,
      const float32_t max_otg_speed,
      F360_Host_Props_T &host_props
   )
   {
      const float32_t host_yaw_rate_var = 1e-6F;
      float32_t vcs_origin_pos[2];
      float32_t host_vcs_vel_vec_var[2][2];

      // Compute uncertainty of host signals at center of rear axle
      const float32_t host_speed_var = Compute_Raw_Host_Speed_Uncertainty(host, max_otg_speed);
      host_props.std_speed_scm = F360_Sqrtf(host_speed_var);
      host_props.std_yaw_rate_scm = F360_Sqrtf(host_yaw_rate_var);

      // Compute uncertainty of host velocity at front center (i.e. at VCS origin)
      vcs_origin_pos[0] = 0.0F;
      vcs_origin_pos[1] = 0.0F;
      Get_Host_Velocity_Uncertainty(
         host,
         host_speed_var,
         host_yaw_rate_var,
         vcs_origin_pos,
         host_vcs_vel_vec_var);
      /* Transforming host velocity covariance matrix from VCS to WCS. I we rotate VCS system
      with -host->heading then it will coincide with WCS system. Hence, rotation with
      -host->heading, which is why input to this function is -host_props->sin_heading
      sin(-angle) = -sin(angle) */
      Rotate_2D_Covariance_Matrix(host_props.cos_heading, host_props.sin_heading, host_vcs_vel_vec_var, host_props.vel_cov_scm);

      // Compute uncertainty in host delta position since last iteration.
      // Position increment covariance differs only by a factor delta time from velocity uncertainty
      (void)(memcpy(&host_props.position_inc_cov_scm, &host_props.vel_cov_scm, sizeof(host_props.position_inc_cov_scm)));
      F360_Multiply_2D_Matrix_With_Constant(elapsed_time*elapsed_time, host_props.position_inc_cov_scm); // Account for delta time
   }

   /*===========================================================================*\
   * FUNCTION: Calculate_Detections_Uncertainties()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Host_T &host
   * const F360_Host_Props_T &host_props
   * const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS]
   * const F360_Radar_Sensor_Props_T (&sensor_props)[MAX_NUMBER_OF_SENSORS]
   * const RSPP_Calibrations_T &rspp_calibrations
   * rspp_variant_A::RSPP_Detection_List_T &raw_detection_list,
   *
   * EXTERNAL REFERENCES:
   * None
   *
   * DEVIATIONS FROM STANDARDS:
   * None
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This functions computes uncertainties for some detection signals and stores
   * the information in the detection properties data structure. Uncertanties
   * for the following detection signals are computed:
   * - Detection compensated range rate (corresponding field in detection
   *   properties data structure is std_range_rate_compensated_scm)
   * - Detection VCS position vector (corresponding field in detection properties
   *   data structure is vcs_position_cov_scm)
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Calculate_Detections_Uncertainties(
      const F360_Host_T &host,
      const F360_Host_Props_T &host_props,
      const F360_Radar_Sensor_T (&sensors)[MAX_NUMBER_OF_SENSORS],
      const RSPP_Calibrations_T &rspp_calibrations,
      rspp_variant_A::RSPP_Detection_List_T& raw_detection_list)
   {

      // Split detections according to their parent sensors
      uint32_t ndets_per_sensor[MAX_NUMBER_OF_SENSORS] = { };
      uint32_t det_idx_per_sensor[MAX_NUMBER_OF_SENSORS][MAX_DETS_FOR_SINGLE_SENSOR];

      for (uint32_t det_idx = 0U; det_idx < raw_detection_list.number_of_valid_detections; det_idx++)
      {
         Assign_Detection_Per_Sensor(raw_detection_list.detections[det_idx], det_idx, det_idx_per_sensor, ndets_per_sensor);
      }

      // Loop over all sensors to process the detections from each sensor
      for (uint32_t sens_idx = 0U; sens_idx < MAX_NUMBER_OF_SENSORS; sens_idx++)
      {
         Sensor_Capability(ndets_per_sensor[sens_idx], sensors[sens_idx], host, host_props, det_idx_per_sensor[sens_idx], rspp_calibrations, raw_detection_list);
      }
   }

   /*===========================================================================*\
   * FUNCTION: Sensor_Capability()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const uint32_t sensor_number_of_detections
   * const F360_Radar_Sensor_T &sensor
   * const F360_Host_T &host
   * const F360_Host_Props_T &host_props
   * const uint32_t (&sensor_det_idx)[MAX_DETS_FOR_SINGLE_SENSOR]
   * const F360_Radar_Sensor_Props_T &sensor_prop
   * const RSPP_Calibrations_T &rspp_calibrations
   * rspp_variant_A::RSPP_Detection_List_T &raw_detection_list
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
   * This function computes uncertainties for detections associated with analyzed sensor.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Sensor_Capability(
      const uint32_t sensor_number_of_detections,
      const F360_Radar_Sensor_T &sensor,
      const F360_Host_T &host,
      const F360_Host_Props_T &host_props,
      const uint32_t (&sensor_det_idx)[MAX_DETS_FOR_SINGLE_SENSOR],
      const RSPP_Calibrations_T & rspp_calibrations,
      rspp_variant_A::RSPP_Detection_List_T& raw_detection_list)
   {
      if (sensor.variable.is_valid && (0U < sensor_number_of_detections))
      {
         float32_t sens_vcs_vel_vec[2]{};
         Get_Sensor_VCS_Velocity(sensor, sens_vcs_vel_vec);

         float32_t sens_vcs_mounting_pos[2]{};
         Get_Sensor_VCS_Mounting_Position(sensor, sens_vcs_mounting_pos);

         float32_t sens_vcs_vel_vec_var[2][2]{};
         Get_Host_Velocity_Uncertainty(
            host,
            host_props.std_speed_scm*host_props.std_speed_scm,
            host_props.std_yaw_rate_scm*host_props.std_yaw_rate_scm,
            sens_vcs_mounting_pos,
            sens_vcs_vel_vec_var);

         for (uint32_t det_per_sensor_index = 0U; det_per_sensor_index < sensor_number_of_detections; det_per_sensor_index++)
         {
            const uint32_t det_idx = sensor_det_idx[det_per_sensor_index];
            rspp_variant_A::RSPP_Detection_T &detection = raw_detection_list.detections[det_idx];
            float32_t det_range_var;
            float32_t det_azimuth_var;
            float32_t det_range_rate_var;

            Compute_Raw_Detection_Uncertainty(
               detection,
               sensor,
               rspp_calibrations,
               det_range_var,
               det_azimuth_var,
               det_range_rate_var);

            const float32_t det_vcs_azimuth_var = Compute_Detection_VCS_Azimuth_Variance(det_azimuth_var, rspp_calibrations);
            detection.processed.std_vcs_az_scm = F360_Sqrtf(det_vcs_azimuth_var);

            float32_t det_comp_rng_rate_var;
            Get_Uncertainty_Of_Compensated_Range_Rate(
               detection.processed.cos_vcs_az,
               detection.processed.sin_vcs_az,
               sens_vcs_vel_vec,
               det_range_rate_var,
               det_vcs_azimuth_var,
               sens_vcs_vel_vec_var,
               det_comp_rng_rate_var);

            detection.processed.std_range_rate_compensated_scm = F360_Sqrtf(det_comp_rng_rate_var);

            Get_Uncertainty_Of_Detection_Position(
               detection.raw.range,
               detection.processed.cos_vcs_az,
               detection.processed.sin_vcs_az,
               det_range_var,
               det_vcs_azimuth_var,
               detection.processed.vcs_position_cov_scm);

            Compute_Detection_Cross_Covariances(
               sens_vcs_vel_vec,
               det_vcs_azimuth_var,
               detection);
         }
      }
   }
   
   /*===========================================================================*\
   * FUNCTION: Compute_Detection_VCS_Azimuth_Variance()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - vcs azimuth variance
   *
   * PARAMETERS:
   * const float32_t &det_azimuth_var
   * const RSPP_Calibrations_T &rspp_calibrations
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
   * This function computes detection VCS azimuth variance.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   float32_t Compute_Detection_VCS_Azimuth_Variance(
      const float32_t &det_azimuth_var,
      const RSPP_Calibrations_T & rspp_calibrations)
   {
      return det_azimuth_var + rspp_calibrations.k_sens_vcs_mounting_az_std * rspp_calibrations.k_sens_vcs_mounting_az_std;
   }

   /*===========================================================================*\
   * FUNCTION: Get_Sensor_VCS_Mounting_Position()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T &sensor,
   * float32_t (&sensor_vcs_mounting_pos)[2]
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
   * This function gets sensor VCS mounting position.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Sensor_VCS_Mounting_Position(
      const F360_Radar_Sensor_T &sensor,
      float32_t (&sensor_vcs_mounting_pos)[2])
   {
      sensor_vcs_mounting_pos[0] = sensor.constant.mounting_position.vcs_position.longitudinal;
      sensor_vcs_mounting_pos[1] = sensor.constant.mounting_position.vcs_position.lateral;
   }

   /*===========================================================================*\
   * FUNCTION: Get_Sensor_VCS_Velocity()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const F360_Radar_Sensor_T &sensor,
   * float32_t (&sensor_vcs_vel_vec)[2]
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
   * This function gets sensor VCS velocity.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_Sensor_VCS_Velocity(
      const F360_Radar_Sensor_T &sensor,
      float32_t (&sensor_vcs_vel_vec)[2])
   {
      sensor_vcs_vel_vec[0] = sensor.variable.vcs_velocity.longitudinal;
      sensor_vcs_vel_vec[1] = sensor.variable.vcs_velocity.lateral;
   }

   /*===========================================================================*\
   * FUNCTION: Assign_Detection_Per_Sensor()
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * const rspp_variant_A::RSPP_Detection_T &raw_detection,
   * const uint32_t det_idx,
   * uint32_t (&det_idx_per_sensor)[MAX_NUMBER_OF_SENSORS][MAX_DETS_FOR_SINGLE_SENSOR],
   * uint32_t (&ndets_per_sensor)[MAX_NUMBER_OF_SENSORS]
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
   * This function assigns detection to sensor.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Assign_Detection_Per_Sensor(
      const rspp_variant_A::RSPP_Detection_T &raw_detection,
      const uint32_t det_idx,
      uint32_t (&det_idx_per_sensor)[MAX_NUMBER_OF_SENSORS][MAX_DETS_FOR_SINGLE_SENSOR],
      uint32_t (&ndets_per_sensor)[MAX_NUMBER_OF_SENSORS])
   {
      const int32_t sens_idx = raw_detection.raw.sensor_id - 1;
      det_idx_per_sensor[sens_idx][ndets_per_sensor[sens_idx]] = det_idx;
      ndets_per_sensor[sens_idx]++;
   }
}
