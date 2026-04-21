#ifndef REDUCED_OBJECT_LOG_T_H
#define REDUCED_OBJECT_LOG_T_H
/*===========================================================================*\
 * FILE: ReducedObjectsLog.h
 *===========================================================================
 * Copyright 2024 Aptiv Technologies, Inc., All Rights Reserved.
 * Aptiv Confidential
 *---------------------------------------------------------------------------
 *
 * DESCRIPTION:
 *   This file contains the F360 Tracker Reduced Objects Information
 *
 * ABBREVIATIONS:
 *   TODO: List of abbreviations used, or reference(s) to external document(s)
 *
 * TRACEABILITY INFO:
 *   Design Document(s):
 *   Requirements Document(s): PDD-10024333-012_(CADS4_VFP_Ethernet_Communication).doc
 *   (Design & Requirements)
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     SW REF 264.15D "Aptiv C Coding Standards" [12-Mar-2006]
 *
 * CHANGE SYNERGY INFO:
 * %full_filespec:   ReducedObjectsLog.h %
 * %version:         1 %
 * %date_modified:   Jan 5, 2017 %
 *
 * DEVIATIONS FROM STANDARDS:
 *   None
 *
\*===========================================================================*/

#include "f360_reuse.h"
/*===========================================================================*\
* Other Header Files
\*===========================================================================*/
#include "emblib_T360_Types.h"

/*===========================================================================*\
* Exported local (file scope) Constants
\*===========================================================================*/
#define REDUCED_OBJECTS_LOG_STREAM_NUM     6
#define REDUCED_OBJECTS_LOG_STREAM_VERSION 21

// #ifndef __TASKING__
//    #pragma pack(push, save_pack_obj, 4)
// #else
// #endif

typedef struct Object_Log_Tag
{
   uint32_t unique_id; //!< [0 - 2^32-1] unique ID of object track

   float vcs_xposn;    //!< [m] x-position of object track's centroid in VCS coordinate.
   float vcs_yposn;    //!< [m] y-position of object track's centroid in VCS coordinate.
   float vcs_xvel;     //!< [m/s] over-the-ground velocity of object track's centroid along x direction (in VCS coordinate).
   float vcs_yvel;     //!< [m/s] over-the-ground velocity of object track's centroid along y direction (in VCS coordinate).
   float vcs_xaccel;   //!< [m/s^2] over-the-ground track acceleration along x direction (in VCS coordinate).
   float vcs_yaccel;   //!< [m/s^2] over-the-ground track acceleration along y direction (in VCS coordinate).
   float vcs_heading;  //!< [rad] heading angle of object track's centroid in VCS coordinate.
   float vcs_pointing; //!< [rad] orientation of track's bounding box in VCS coordinate.

   float speed;      //!< [m/s, (-10)-100] signed speed of object track's centroid. Negative value indicates track backing up.
   float curvature;  //!< [rad/m] trajectory curvature of object track's centroid in world coordinate.
   float tang_accel; //!< [m/s^2] over-the-ground tangential acceleration of object's centroid.

   float state_variance[6]; //!< variance of the state vector consisting of [vcs_xposn, vcs_yposn, vcs_heading, speed, curvature,
                            //!< and tangential accelaration] when track filter is CTCA and [vcs_xposn, vcs_xvel, vcs_yposn,
                            //!< vcs_yvel, unused_1, unused_2] when tracker filter is CCV

   float supplemental_state_covariance[3]; //!< covariance of [vcs_xposn & vcs_yposn, speed & vcs_heading, tangential acceleration
                                           //!< & curvature] when track filter is CTCA and [vcs_xposn & vcs_yposn, vcs_xvel &
                                           //!< vcs_yvel, unused] when track filter is CCV

   float time_since_measurement; //!< [seconds] time since the current track was updated (it can be time of measurement or in case
                                 //!< of coasting the time increased by cumulated time of time-update step).
   float time_since_cluster_created; //!< [seconds] time since the creation of current track.
   float time_since_track_updated;   //!< [seconds] time since most recent detections were associated to current track.

   float len1;                   //!< [m] longitudinal distance from centroid to rear edge of bounding box.
   float len2;                   //!< [m] longitudinal distance from centroid to front edge of bounding box.
   float wid1;                   //!< [m] lateral distance from centroid to left edge of bounding box.
   float wid2;                   //!< [m] lateral distance from centroid to right edge of bounding box.
   float confidenceLevel;        //!< [0-1] confidence level of current track.
   float time_since_stage_start; //!< [seconds] time since start of current track status.

   float existence_probability; //!< [0-1] Probability that the target is an actual target

   float accuracy_length;
   float accuracy_width;

   /* Probability vectors */
   float probability_pedestrian;
   float probability_car;
   float probability_motorcycle;
   float probability_bicycle;
   float probability_truck;
   float probability_undet;
   uint32_t padding_t;
   uint16_t trkID; //!< [0-250] ID of object tracks, zero-ID represents an inactive track.
   uint16_t ndets; //!< [0-200] number of sensor detections associated to current object track.
   uint16_t
      num_reduced_dets; //!< [0-200] number of downselected detections among all detections associated to current object track.

   uint8_t reducedID;       //!< [0-125] ID of downselected object tracks, zero-ID means current track is not downselected.
   uint8_t trk_fltr_type;   //!< [0-2] (enum F360_Trk_Fltr_Type_T) track filter type, 0-INVALID, 1-CCV, 2-CTCA.
   uint8_t reference_point; //!< [0-8] (enum F360_Reference_Point_T) Specifies where on the object the (x,y)-posn is located. 0 =
                            //!< center, 1 = front left, 2 = front mid, 3 = front right, 4 = right mid, 5 = rear right, 6 = rear
                            //!< mid, 7 = rear left, 8 = rear left.
   uint8_t
      status; //!< [0-5] (enum F360_OBJ_STATUS) status of object track in current frame, 0-INVALID, 1-NEW (unconfirmed), 2-NEW
              //!< UPDATED (track initialized),
              //!  3-NEW COASTED (track just coasted), 4-UPDATED (has detection association), 5-COASTED (no detection association).
   uint8_t reducedStatus; //!< [0-5] (enum F360_OBJ_STATUS) status of track downselection. 0-current track not downselected,
                          //!< 1-current track newly downselected, 3-5 current track downselected, same meaning as variable status.

   uint8_t init_scheme; //!< [(-1)-3] (enum F360_TRK_INIT_TYPE) type of track initialization, (-1)-invalid, 0-first generation of
                        //!< cloud algorithm, 1-second generation of cloud algorithm, 2-radial motion, 3-longitudinal motion.

   uint8_t
      object_class; //!< [0-10] Determines the type of Object detected. 0=Object Undetermined, 1=Car, 2=MotorCycle,  3=Truck ,
                    //!< 4=Pedestrain,
                    //!         5=Pole, 6=Tree, 7=Animal, 8=General on-road Object Detected, 9=Bicycle, 10=Vehicle Unidentified.

   uint8_t f_crossing; //!< [bool] indicates if current track has cross radial motion with respect to the sensor illuminating this
                       //!< object.

   uint8_t f_moving;   //!< [bool] indicates if current track is moving at the current time.
   uint8_t f_moveable; //!< [bool] indicates if current track has ever been seen to move.
   uint8_t f_oncoming; //!< [bool] indicates if current track is on-coming with respect to host vehicle.

   uint8_t f_vehicular_trk; //!< [bool] indicates if current track is a vehicle.
   uint8_t f_onguardrail;   //!< [bool] indicates if the current track is on guard rail.

   uint8_t f_fast_moving; //!< [bool] indicates if the current track's speed is above 3 m/s

   uint8_t underdrivable_status; //!< [0-3] (enum Underdrivable_Status_T) track underdrivability status, 0 - host can pass under
                                 //!< track, 1 - host is likely to pass under track, 2 - host can not pass under track, 3 - track
                                 //!< is not in area of interest.

   uint8_t current_msmt_type; //!< [0-1] (enum F360_Current_Msmt_Type_T) current measurement type, intermediate unification of
                              //!< interfaces, to be removed after DEX-1436.

   uint8_t padding[2];
} Object_Log_T;

typedef struct Reduced_Objects_Log_Tag
{
   uint64_t object_list_timestamp; //!< [us] object list timestamp related to measurement timestamp
   Object_Log_T obj[MAX_REDUCED_OBJECTS];
   uint32_t tracker_index; //!< index of the current tracker iteration.
} Reduced_Objects_Log_T;

//  **********************************************************************************************************
//  ************************ WARNING!!!!!! *******************************************************************
//  **********************************************************************************************************
//  The following compile-time assertion fails if the size of the log stream type does not equal the expected
//  size.  If it fails, then the size must be corrected AND the Stream LogVersion must changed.
//  If the version in this Stream LogVersion is NOT changed, then DV tool will not be able to decode the stream!
//  **********************************************************************************************************

// LOGSIZE_ASSERT(sizeof(Reduced_Objects_Log_T) == (22512));

// #ifndef __TASKING__
//    #pragma pack(pop, save_pack_obj)
// #else
// #endif

#endif /* REDUCED_OBJECT_LOG_T_H */
