#ifndef F360_INTERNAL_OBJECT_H
#define F360_INTERNAL_OBJECT_H

// Add pragmas to throw error if struct is padded
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(error : 4820)
#elif 0
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wpadded"
#endif

#include <f360_reuse.h>

static const int32_t F360_Internal_Object_Stream_Num = 150;
static const int32_t F360_Internal_Object_Stream_Ver = 9;
static const int32_t F360_Internal_Object_Max_Payload_Size = 2000;

typedef struct F360_Internal_Object_Tag
{
   float other_state_covariance[12];       /* states in error cov that is not part of logged state_variance[6] or supplemental_state_covariance[3]. Note: For stream version 11 and above tracking is done in VCS instead of in WCS */
   float orth_delta_filtered;              /* low pass filtered max orthogonal delta of associated detections */
   float orth_gap_filtered;                /* low pass filtered max orthogonal gap of associated detections */
   float filtered_pos_diff_heading;        /* low pass filtered heading solely based on position delta between tracker iterations */
   float time_since_initialization;        /* time since object initialization (based on tracker execution time) [s] */
   float time_since_vehicle_init;          /* time since f_vehicular_trk flag set (based on tracker execution time) [s] */
   float time_since_last_stop;             /* time since last time the object comes to a stop from moving motion classification [s] */
   float filtered_dets;                    /* filtered over the time number of detection (can be fraction) [-] */
   float prev_avrg_conf_level;             /* average confidence level calculated based on previous and current confidence  [-] */
   float innovation_length;                /* length innovation [m] [m] */
   float innovation_width;                 /* width innovation [m] [m] */
   float length_uncertainty;
   float width_uncertainty;
   float mirror_prob;                      /* Probability (0-1) that this track is a mirror of multi path detections (where mirror is guardrail or other object) [-] */
   float average_rcs;                      /* low-pass filtered RCS value based on average rcs of associated detections [dB/m^2] */
   float hdg_ptng_disagmt;                 /* filtered difference between heading and pointing angles */
   float cca_pnt_filter_cov[3];            /*  covariance matrix used in object cca pointing filter */
   float filtered_hist_assoc_det_rr_err_mean; /* adjusted mean difference between speeds of a detectionandand object it is assigned to, accumulated over object's lifetime. */
   float filtered_hist_assoc_det_rr_err_var;  /* adjusted variance difference between speeds of a detection and and object it is assigned to, accumulated over object's lifetime. */
   float filtered_hist_assoc_n_dets;          /* adjusted total number of detections associated to the object over time */
   float ud_mov_historic_height_mean;      /* height mean of the object, accumulated over object's lifetime. The height mean is weighted with forgetting factor and decays by 3% if there are no detections assigned to the object [m] */
   float ud_mov_historic_ndets;            /* total number of detections used to calculate historic height mean, associated to the object over time[-] */
   float time_since_split;                 /* time since object was involved in a split[s] */
   int32_t cnt_error_in_predicted_speed;   /* counter of updates with  speed inaccuracy estimation detection counter [tracker execution cycles] */
   int32_t cntConsecutiveAmbiguous;        /* counter of consecutive tracked object update by ambiguous object motion status (incremented once per tracker cycle) [tracker execution cycles] */
   int32_t cntConsecutiveMoving;           /* counter of consecutive tracked object update by moving object motion status (incremented once per tracker cycle) [tracker execution cycles] */
   int32_t cntConsecutiveStopped;          /* counter of consecutive tracked object update by stopped object motion status (incremented once per tracker cycle) [tracker execution cycles] */
   int32_t total_reduced_dets;             /* number of down-selected detection associated to the track through its lifespan [-] */
   uint32_t ud_mov_cnt_underdrivable;      /* number of scans during which object historic height mean was above the threshold determining ud status[-] */
   uint16_t id;
   uint8_t num_updates_since_init;         /* number of times an upbject has been KF measurement updated since it was first born */
   uint8_t min_projection_reference_point; /* Enumeration of which object point (corners, side midpoints or center) that is most likely seen from center of host regardless of visiblity */
   uint8_t behind_sep_id;                  /* id of highest prioritized SEP that this object is behind. Tracks that are "on" will never be flagged as "behind". Only CTCA and fast moving CCA tracks are flagged as behind whereas slow moving CCA tracks have behind_lsc_id = F360_INVALID_UNSIGNED_ID by default */
   uint8_t on_sep_id;                      /* id of highest prioritized SEP which this object is "on" */
   uint8_t conf_longitudinal_position;     /* [CONF9_T] Internal state confidence of vcs_position longitudinal used for determining overall confidence */
   uint8_t conf_lateral_position;          /* [CONF9_T] Internal state confidence of vcs_position lateral used for determining overall confidence */
   uint8_t conf_longitudinal_velocity;     /* [CONF9_T] Internal state confidence of vcs_velocity longitudinal used for determining overall confidence */
   uint8_t conf_lateral_velocity;          /* [CONF9_T] Internal state confidence of vcs_velocity lateral used for determining overall confidence */
   uint8_t conf_speed;                     /* [CONF9_T] Internal state confidence of speed used for determining overall confidence */
   uint8_t conf_overall;                   /* [CONF3_T] Overall state confidence based on "smoothness" of all states */
   uint8_t low_rcs_dets_cnt;               /* Counter for blocking overall confidence increase */
   uint8_t f_ghost_NU_2_C;                 /* flag indicating change of object status from NEW to COASTED [-] */
   uint8_t f_veh_trk_near_stat_host;       /* flag indicating that host is stationary and moving track is in a short distance [-] */
   uint8_t f_overlapping_with_object;      /* flag indicating that object bounding box overlaps with different bounding box [-] */
   uint8_t f_valid_for_liberal_tracking;   /* Process this track with more liberal conditions within association and earlier downselection */
   uint8_t f_stopped;                      /* flag indicating that object has stopped [-] */
   uint8_t padding[2];
} F360_Internal_Object_T;

// Restore MSVC and GCC warning settings
#if defined _MSC_VER
#pragma warning(pop)
#elif 0
#pragma GCC diagnostic pop
#endif
#endif
