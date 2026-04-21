/*===========================================================================*\
* FILE: f360_update_longi_stat_curves.cpp
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential - Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function definition of Update_Longi_Stat_Curves() and subfunctions
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/

#include "f360_update_longi_stat_curves.h"
#include "f360_math.h"
#include "f360_matrix_vector_Init_real32_T.h"
#include "f360_math_func.h"
#include "f360_longi_stat_curve_init.h"
#include "f360_cluster_objects_for_lsc.h"
#include "f360_downselect_longi_stat_clusters.h"
#include "f360_post_process_longi_stat_clusters.h"
#include "f360_get_wall_time.h"
#include "f360_check_if_point_is_inside_box.h"


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Update_Longi_Stat_Curves()
   * ===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   *   const F360_Tracker_Info_T& tracker_info,
   *   const F360_Calibrations_T& calibs,
   *   const F360_Host_T& host,
   *   F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
   *   F360_Longi_Stat_Curve_T(&longi_stat_curves)[MAX_NR_OF_LONGI_STAT_CURVES],
   *   F360_TRKR_TIMING_INFO_T& timing_info
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
   * This function clusters slow moving CCA objects and fits polynomial to clusters 
   * that fulfill the requirements to become longi stat curves
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Update_Longi_Stat_Curves(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calibs,
      const F360_Host_T& host,
      F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
      F360_Longi_Stat_Curve_T(&longi_stat_curves)[MAX_NR_OF_LONGI_STAT_CURVES],
      F360_TRKR_TIMING_INFO_T& timing_info
      )
   {
      const float32_t start_time = get_wall_time();

      // Reset all curves from previous tracker iteration, all curves are instantaneous estimates and not filtered in time
      F360_Longi_Stat_Curve_Init(longi_stat_curves);

      for (int32_t i = 0; i < tracker_info.num_active_objs; i++)
      {
         const int32_t obj_idx = tracker_info.active_obj_ids[i] - 1;

         objects[obj_idx].lsc_next_in_cluster = NULL;
         objects[obj_idx].lsc_prev_in_cluster = NULL;
      }

      // Init first iteration. Prepare data for start of clustering algo
      uint16_t nr_next_ids_of_interest;
      uint16_t next_ids_of_interest[NUMBER_OF_OBJECT_TRACKS] = {};
      const bool f_is_data_available = Arrange_First_Iteration(
         tracker_info,
         calibs,
         host,
         nr_next_ids_of_interest,
         next_ids_of_interest
         );

      if (f_is_data_available)
      {
         // Valid cluster array. Array of clusters that are candidates to be fitted to polynomial in the end of this function
         uint16_t nr_valid_clusters = 0U;
         F360_Longi_Stat_Cluster_T valid_clusters[NR_LONGI_STAT_CLUSTERS] = {};

         Cluster_Objects_For_LSC(
            calibs,
            tracker_info,
            host,
            nr_next_ids_of_interest,
            next_ids_of_interest,
            objects,
            nr_valid_clusters,
            valid_clusters);

         Post_Process_Longi_Stat_Clusters(
            calibs,
            nr_valid_clusters,
            valid_clusters);

         uint16_t nr_downselected_clusters;
         F360_Longi_Stat_Cluster_T downselected_clusters[MAX_NR_OF_LONGI_STAT_CURVES] = {};
         Downselect_Longi_Stat_Clusters(
            nr_valid_clusters,
            valid_clusters,
            calibs,
            nr_downselected_clusters,
            downselected_clusters);

         F360_Longi_Stat_Curve_T new_longi_stat_curves[MAX_NR_OF_LONGI_STAT_CURVES] = {};
         Fit_Second_Order_Polynomials_To_Clusters(
            nr_downselected_clusters,
            downselected_clusters,
            new_longi_stat_curves);

         Sanity_Check_And_Populate_LSC_Output(
            calibs,
            nr_downselected_clusters,
            new_longi_stat_curves,
            longi_stat_curves);
      }

      timing_info.lsc_module = get_wall_time() - start_time;
   }

   /*===========================================================================*\
   * FUNCTION: Arrange_First_Iteration()
   * ===========================================================================
   * RETURN VALUE:
   * bool f_is_data_remaining
   *
   * PARAMETERS:
   *   const F360_Tracker_Info_T& tracker_info,
   *   const F360_Calibrations_T& calibs,
   *   const F360_Host_T& host,
   *   F360_Object_Track_T(&objects)[NUMBER_OF_OBJECT_TRACKS],
   *   uint16_t& nr_next_ids_of_interest,
   *   uint16_t(&next_ids_of_interest)[NUMBER_OF_OBJECT_TRACKS]
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
   * This function arranges the data for the first clustering iteration.
   * It copies the longitudinal sorted objects array and does a sanity check that 
   * enough objects exists for clustering to be possbile.
   *
   * PRECONDITIONS:
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   bool Arrange_First_Iteration(
      const F360_Tracker_Info_T& tracker_info,
      const F360_Calibrations_T& calibs,
      const F360_Host_T& host,
      uint16_t& nr_next_ids_of_interest,
      uint16_t(&next_ids_of_interest)[NUMBER_OF_OBJECT_TRACKS]
      )
   {
      // Fill sorted array of objects
      nr_next_ids_of_interest = 0U;
      const float32_t host_turn_radius = std::abs(host.curvature_rear) > F360_EPSILON ? 1.0F / host.curvature_rear : INFTY;
      const F360_Object_Track_T* curr_trk = tracker_info.vcslong_sorted_start; // Start with first object in list
      for (uint32_t i = 0U; i < static_cast<uint32_t>(tracker_info.num_active_objs); i++)
      {
         if (curr_trk->bbox.Get_Center().x < calibs.k_lsc_min_long_pos)
         {
            // Do nothing
         }
         else if (curr_trk->bbox.Get_Center().x > calibs.k_lsc_max_long_pos)
         {
            // Stop filling relevant objects
            break;
         }
         else
         {
            if (Is_Object_Valid_For_LSC_Cluster(*curr_trk, calibs, host_turn_radius))
            {
               next_ids_of_interest[nr_next_ids_of_interest] = static_cast<uint16_t>(curr_trk->id);
               nr_next_ids_of_interest++;
            }
         }

         // take next object from list
         curr_trk = tracker_info.vcslong_sorted_next_track[curr_trk->id - 1];
      }

      // Check that there are enough objects to form at least one cluster
      bool f_is_data_remaining;
      if (nr_next_ids_of_interest >= calibs.k_lsc_min_points_in_cluster)
      {
         f_is_data_remaining = true;
      }
      else
      {
         f_is_data_remaining = false;
      }

      return f_is_data_remaining;
   }

   /*===========================================================================*\
    * FUNCTION: Is_Object_Valid_For_LSC_Cluster()
    * ===========================================================================
    * RETURN VALUE:
    * bool f_is_valid
    *
    * PARAMETERS:
    *   const F360_Object_Track_T& obj
    *   const F360_Calibrations_T& calibs
    *   const float host_turn_radius
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
    * This function checks if the current object is eligible to be tried for clustering
    *
    * PRECONDITIONS:
    *
    * POSTCONDITIONS:              
    * None
    *
    \*===========================================================================*/
   bool Is_Object_Valid_For_LSC_Cluster(
      const F360_Object_Track_T& obj,
      const F360_Calibrations_T& calibs,
      const float host_turn_radius)
   {
       
       float dist_to_circle;
       if (host_turn_radius < INFTY)
       {
           dist_to_circle = std::abs(F360_Sqrtf((obj.vcs_position.y - host_turn_radius) * (obj.vcs_position.y - host_turn_radius) + obj.vcs_position.x * obj.vcs_position.x) - std::abs(host_turn_radius));
       }
       else
       {
           dist_to_circle = std::abs(obj.vcs_position.y);
       }
       const bool f_in_allowed_zone = (dist_to_circle > calibs.k_distance_to_circle_thr);
       const bool f_is_valid = ((F360_TRACKER_TRKFLTR_CCA == obj.trk_fltr_type) && (!obj.f_moveable) 
           && f_in_allowed_zone);

      return f_is_valid;
   }

 /*===========================================================================*\
 * FUNCTION: Fit_Second_Order_Polynomials_To_Clusters()
 * ===========================================================================
 * RETURN VALUE:
 * None
 *
 * PARAMETERS:
 *   const uint16_t nr_valid_clusters
 *   const F360_Longi_Stat_Cluster_T(&valid_clusters)[MAX_NR_OF_LONGI_STAT_CURVES]
 *   F360_Longi_Stat_Curve_T(&all_curves)[MAX_NR_OF_LONGI_STAT_CURVES]
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
 * This function fits a second degree polynomial to valid clusters and populates the
 * longi stat curve structure.
 *
 * PRECONDITIONS:
 *
 * POSTCONDITIONS:
 * None
 *
 \*===========================================================================*/
   void Fit_Second_Order_Polynomials_To_Clusters(
      const uint16_t nr_valid_clusters,
      const F360_Longi_Stat_Cluster_T(&valid_clusters)[MAX_NR_OF_LONGI_STAT_CURVES],
      F360_Longi_Stat_Curve_T(&all_curves)[MAX_NR_OF_LONGI_STAT_CURVES])
   {

      for (uint32_t i = 0U; i < nr_valid_clusters; i++)
      {
         // Arrange matrix for least square fit
         const uint32_t nr_clustered_ids = valid_clusters[i].nr_objects;
         const int32_t nr_clustered_ids_signed = static_cast<int32_t>(nr_clustered_ids);

         const int32_t nr_poly_coeff_slots_signed = static_cast<int32_t>(LSC_NR_POLY_COEFF_SLOTS);
         F360_matrix_real32_LSC_T A;
         A.m_size[0] = nr_clustered_ids_signed;
         A.m_size[1] = nr_poly_coeff_slots_signed;
         A.numDimensions = nr_poly_coeff_slots_signed;

         F360_vector_real32_LSC_T B;
         B.m_size = nr_clustered_ids_signed;
         B.numDimensions = 1;

         F360_Object_Track_T* current_obj = valid_clusters[i].first_object;
         for (uint32_t k = 0U; k < nr_clustered_ids; k++)
         {

            // Compensate for center displacement
            const float32_t obj_aspect_angle = F360_Atan2f(current_obj->bbox.Get_Center().y, current_obj->bbox.Get_Center().x);
            const float32_t obj_radius = current_obj->bbox.Get_Length() * 0.5F;   // nonmoveable objects are in circular shape
            const float32_t x_vcs = current_obj->bbox.Get_Center().x - obj_radius * F360_Cosf(obj_aspect_angle);
            const float32_t y_vcs = current_obj->bbox.Get_Center().y - obj_radius * F360_Sinf(obj_aspect_angle);

            // Fill matrices for polynomial fit
            A.data[k][0] = x_vcs * x_vcs;
            A.data[k][1] = x_vcs;
            A.data[k][2] = 1.0F;

            B.data[k] = y_vcs;

            current_obj = current_obj->lsc_next_in_cluster;
         }

         float32_t x1; // Coefficient a 
         float32_t x2; // Coefficient b
         float32_t x3; // Coefficient c
         const bool f_poly_fit_ok = F360_Fit_Second_Degree_Polynomial(A, B, x1, x2, x3);

         if (f_poly_fit_ok)
         {
            all_curves[i].f_valid = true;
            all_curves[i].x_min = Get_Cluster_Min_Long_Pos(valid_clusters[i]);
            all_curves[i].x_max = Get_Cluster_Max_Long_Pos(valid_clusters[i]);
            all_curves[i].a = x1;
            all_curves[i].b = x2;
            all_curves[i].c = x3;
            all_curves[i].mean_lat_pos = valid_clusters[i].lat_mean;
         }
         else
         {
            all_curves[i].f_valid = false;
            all_curves[i].x_min = 0.0F;
            all_curves[i].x_max = 0.0F;
            all_curves[i].a = 0.0F;
            all_curves[i].b = 0.0F;
            all_curves[i].c = 0.0F;
            all_curves[i].mean_lat_pos = 0.0F;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: Sanity_Check_And_Populate_LSC_Output()
   * ===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   *   const F360_Calibrations_T & calibs
   *   const uint16_t nr_downselected_clusters
   *   const F360_Longi_Stat_Curve_T(new_longi_stat_curves)[MAX_NR_OF_LONGI_STAT_CURVES]
   *   F360_Longi_Stat_Curve_T(longi_stat_curves)[MAX_NR_OF_LONGI_STAT_CURVES]
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
   * This function sanity checks the found curves and populates the LSC
   * structure if the curve is valid.
   *
   * PRECONDITIONS:
   * None
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Sanity_Check_And_Populate_LSC_Output(
      const F360_Calibrations_T & calibs,
      const uint16_t nr_downselected_clusters,
      const F360_Longi_Stat_Curve_T(&new_longi_stat_curves)[MAX_NR_OF_LONGI_STAT_CURVES],
      F360_Longi_Stat_Curve_T(&longi_stat_curves)[MAX_NR_OF_LONGI_STAT_CURVES])
   {
      // Sanity check found polynomials and fill output structure
      for (uint32_t i = 0U; i < nr_downselected_clusters; i++)
      {
         longi_stat_curves[i] = new_longi_stat_curves[i];

         if (std::abs(longi_stat_curves[i].a) > calibs.k_lsc_max_a_coeff)
         {
            longi_stat_curves[i].f_valid = false;
         }
      }
   }

}

