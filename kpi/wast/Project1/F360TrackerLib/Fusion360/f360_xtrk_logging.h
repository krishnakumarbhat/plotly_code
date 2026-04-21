#ifndef F360_XTRK_LOGGING_H
#define F360_XTRK_LOGGING_H
#ifdef _DEBUG

#include "f360_host_calib.h"
#include "f360_host_raw.h"
#include "f360_host.h"
#include "rspp_detection_list.h"
#include "f360_radar_sensor.h"
#include "f360_radar_sensor_props.h"
#include "f360_detection_props.h"
#include "f360_detection_hist.h"
#include "f360_cluster.h"
#include "f360_object_track.h"
#include "f360_tracker_info.h"
#include "f360_host_props.h"
#include "f360_static_env_poly_types.h"
#include "f360_trailer_detector_flt_fus_output.h"
#include "f360_tracker_version.h"
#include "ocg_occupancy_grid_types.h"

namespace f360_variant_A
{
   bool open_xtrk_log(const char * filename_base);
   void close_xtrk_log(void);

   int write_xtrk_log(
      const F360_Host_Calib_T* const host_calib,
      const F360_Host_Raw_T* const host_raw,
      const F360_Host_T* const host,
      const rspp_variant_A::RSPP_Detection_List_T* const raw_detect_list,
      const F360_Radar_Sensor_T sensors[MAX_NUMBER_OF_SENSORS],
      const F360_Radar_Sensor_Props_T(&sensor_props)[MAX_NUMBER_OF_SENSORS],
      const F360_Detection_Props_T(&det_props)[MAX_NUMBER_OF_DETECTIONS],
      const F360_Detection_Hist_T& det_hist,
      const F360_Cluster_T(&clusters)[NUMBER_OF_CLUSTERS],
      const F360_Object_Track_T(&object_tracks)[NUMBER_OF_OBJECT_TRACKS],
      const F360_Tracker_Info_T& tracker_info,
      const F360_Host_Props_T& host_props,
      const Static_Env_Poly_T(&static_env_polys)[F360_NUM_OF_STATIC_ENV_POLYS],
      const Trailer_Detector_Flt_Fus_Output& trailer_detector_output,
      const ocg::OCG_Outputs_T* occupancy_grid
   );
}
#endif
#endif
