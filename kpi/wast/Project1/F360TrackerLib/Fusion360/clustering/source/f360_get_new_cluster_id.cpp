/*===================================================================================*\
* FILE: f360_get_new_cluster_id.cpp
*====================================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose."
*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* The file contains the definition of Get_New_Cluster_ID function 
*
*
* Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]
*     ESGW_4-2_PE-SWX_00-01-A02_EN "APTIV C Coding Standards" [12-Mar-2006]
***/

/*===========================================================================*\
* Includes
\*===========================================================================*/

#include "f360_get_new_cluster_id.h"
#include "f360_constants.h"

namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: Get_New_Object_Track_ID
   *===========================================================================
   * RETURN VALUE:
   * None
   *
   * PARAMETERS:
   * int32_t &new_id - ID (1-based) of new cluster
   * bool &f_success - indication if new ID can be get
   * F360_Tracker_Info_T &tracker_info
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
   * Function is trying to get new cluster id. In case of fail 
   * (full active_cluster_ids array) function should return new_id equal to 0.
   * tracker_info will be modified as well with respect to 
   *   -tracker_info.active_cluster_ids
   *   -tracker_info.inactive_cluster_ids
   *   -tracker_info.num_active_clusters
   *
   * PRECONDITIONS:
   * Assumed consistency of 
   *   -tracker_info.active_cluster_ids
   *   -tracker_info.inactive_cluster_ids
   *   -tracker_info.num_active_clusters
   * which means that:
   *   - number of non-zero values in tracker_info.active_cluster_ids should be 
   *     equal to tracker_info.num_active_clusters
   *   - sum of nonzero values in tracker_info.inactive_cluster_ids and 
   *     tracker_info.active_cluster_ids should be equal to total 
   *     possible number of clusters
   *
   * POSTCONDITIONS:
   * None
   *
   \*===========================================================================*/
   void Get_New_Cluster_ID(
      int16_t & new_id,
      F360_Tracker_Info_T & tracker_info)
   {
      const int16_t num_active = tracker_info.num_active_clusters;
      const int16_t num_inactive = (static_cast<int16_t>(tracker_info.variant.num_clusters)) - num_active;

      const bool f_success = (num_active < (static_cast<int16_t>(tracker_info.variant.num_clusters)));

      if (f_success)
      {
         //  Get new object track ID from start of inactive IDs list (to re-use oldest IDs first). 
         new_id = tracker_info.inactive_cluster_ids[0];

         //  Shift inactive list one position towards start of array
         for (int16_t i = 0; i < num_inactive - 1; i++)
         {
            tracker_info.inactive_cluster_ids[i] = tracker_info.inactive_cluster_ids[i + 1];
         }
         tracker_info.inactive_cluster_ids[num_inactive - 1] = 0;

         //  Increment and save number of active object tracks.  Add new ID to list of active tracks. 
         tracker_info.active_cluster_ids[num_active] = new_id;
         tracker_info.num_active_clusters++;
      }
      else
      {
         new_id = 0;
      }
   }
}
