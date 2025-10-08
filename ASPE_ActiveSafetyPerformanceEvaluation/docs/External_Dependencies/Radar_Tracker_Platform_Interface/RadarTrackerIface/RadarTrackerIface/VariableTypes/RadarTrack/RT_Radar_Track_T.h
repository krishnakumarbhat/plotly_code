/****************************************************
 *  RT_Radar_Track_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Class RT_Radar_Track_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_D5D3EA09_6434_46c6_8A58_451406570B65__INCLUDED_)
#define EA_D5D3EA09_6434_46c6_8A58_451406570B65__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "RT_Id_T.h"
#include "RT_Object_Class_T.h"
#include "RT_Movement_Status_T.h"
#include "RT_Motion_Model_Info_2D_T.h"
#include "RT_Bounding_Box_Var_2D_T.h"
#include "RT_Track_Property_Flags_T.h"
#include "RT_Bounding_Box_2D_T.h"
#include "RT_Vector_2D_T.h"

/**
 * This structure provides generic motion model data, the accumulated perceived
 * radar cross section in db per square meter, as well as a movement status
 * classification. 
 */
struct RT_Radar_Track_T
{
	/**
	 * Unit: [m/s^2]
	 * 
	 * x and y component of (over the ground) acceleration of object’s reference
	 * point/centroid.
	 */
	RT_Vector_2D_T acceleration_otg;
	/**
	 * Unit: [m^2/s^4]
	 * 
	 * Error covariance of the x and y over the ground acceleration of the object.
	 */
	RT_Vector_Elem_T acceleration_otg_covariance;
	/**
	 * [m^2/s^4]
	 * 
	 * Variance of the x and y over the ground acceleration of the object.
	 */
	RT_Vector_2D_T acceleration_otg_variance;
	/**
	 * Specifies the length and width of the object’s bounding box, it's orientation
	 * and the relative position of the reference point of the bounding box.
	 */
	RT_Bounding_Box_2D_T bounding_box;
	/**
	 * Measures the uncertainty of the distances between the centroid/reference point
	 * and the different edges of the bounding box and the uncertainty of the
	 * orientation angle.
	 */
	RT_Bounding_Box_Var_2D_T bounding_box_pseudo_variance;
	/**
	 * Unit: []                 Range: [0,1]
	 * 
	 * Measure for how likely it is that the reported object corresponds to a real-
	 * world entity.
	 */
	RT_Probability_T existence_indicator;
	/**
	 * Unit: []
	 * 
	 * Id of the object in the current cycle. Object id will only change in case of
	 * splits/merges. Apart from that, an object’s id is constant throughout its
	 * lifetime.
	 */
	RT_Id_T id;
	/**
	 * Unit: []
	 * 
	 * This signal is used to indicate whether the object was part of a split/merge in
	 * the current cycle. If this id agrees with the id of the current cycle (see
	 * above), then no split or merge has happened. If it disagrees, the following
	 * semantics apply:
	 * 
	 * Object split: Both objects resulting from the split will have the id of their
	 * origin object filled in here.
	 * Example: An object with id=12 is divided into two new objects with id = 13 and
	 * id = 14. Then both object 13 and object 14 will have id_split_merge = 12.
	 * Object merge: The object “surviving” the merge will have the id of the object
	 * that has disappeared filled in here. Be aware that this does not support a
	 * clear indication of a merge of multiple objects into one in a single cycle (e.g.
	 * , four small objects being merged to form a truck). In that case, some logic is
	 * needed for deciding which id is filled in here (e.g., the one of the oldest
	 * merged object).
	 * Example: Object with id=42 and id = 15 are merged, and the surviving object is
	 * the one with id=42. This object will then have id_split_merge = 15.
	 */
	RT_Id_T id_split_merge;
	/**
	 * Indicates which motion model is used for the object (currently supported: CTCA,
	 * CCA, PCT)
	 */
	RT_Motion_Model_Info_2D_T motion_model;
	/**
	 * Indicates whether the object is moving, stopped (currently not moving, but has
	 * been seen moving before) or stationary (currently not moving and has never been
	 * seen moving).
	 */
	RT_Movement_Status_T movement_status;
	/**
	 * This vector lists the two most probable object classes. The available classes
	 * will, for the beginning, be the same as those currently used in the Bayesian
	 * classificatory in the GDSR Tracker (Pedestrian, two-wheel, car, truck, unknown).
	 */
	RT_Object_Class_T object_class;
	/**
	 * Unit: []                Range: [0,1]
	 * 
	 * Probabilities of the two most probable object classes. The reason for only
	 * reporting two probabilities instead of probabilities for all classes is that
	 * this way new classes can be added without major interface adaptations.
	 */
	RT_Vector_Elem_T object_class_probability;
	/**
	 * Unit: [m]
	 * 
	 * x and y position of object’s reference point/centroid.
	 */
	RT_Vector_2D_T position;
	/**
	 * Unit: [m^2]
	 * 
	 * Error covariance of the x and y position of the object.
	 */
	RT_Vector_Elem_T position_covariance;
	/**
	 * Unit: [m^2]
	 * 
	 * Variance of the x and y position of the object.
	 */
	RT_Vector_2D_T position_variance;
	/**
	 * Unit: []
	 * 
	 * Provides the ids of up to four radar sensors whose detections have been used to
	 * update the object in the current radar cycle. Sensor ids have to be provided at
	 * Tracker input and have to be consistent across the system.
	 */
	RT_Sensor_Id_T sensor_confirmation;
	/**
	 * Unit: [Ticks] (related to seconds through the constant Common::
	 * RT_Relative_Time_Ticks_Per_Sec_T)Timestamp of the radar cycle
	 * Timestamp of the radar cycle in which the object was initiated.
	 */
	RT_Relative_Time_In_Ticks_T ticks_since_created;
	/**
	 * Unit: [Ticks] (related to seconds through the constant Common::
	 * RT_Relative_Time_Ticks_Per_Sec_T)
	 * 
	 * Timestamp of the last radar cycle in which a detection was associated to the
	 * object. If this disagrees with the timestamp of the current cycle, this
	 * corresponds to what is currently called a "Coasting object".
	 */
	RT_Relative_Time_In_Ticks_T ticks_since_last_detected;
	/**
	 * Unit: [Ticks] (related to seconds through the constant Common::
	 * RT_Relative_Time_Ticks_Per_Sec_T)
	 * 
	 * Timestamp of the radar cycle in which the object's state variables were last
	 * updated (which will typically be the current cycle, as a time update/prediction
	 * is currently always performed on all objects)
	 */
	RT_Relative_Time_In_Ticks_T ticks_since_state_updated;
	/**
	 * Unit: []
	 * 
	 * Contains flags for indicating that the reported track has certain properties.
	 * Currently, these are that is is likely to be a reflection, a ghost or that it
	 * was part of a split or merge in the current cycle. There are 5 reserved bits
	 * which might, e.g., be used for things like under-/overdrivability
	 * classification in the future.
	 */
	RT_Track_Property_Flags_T track_property_flags;
	/**
	 * Unit: [m/s]
	 * 
	 * x and y component of (over the ground) velocity of object’s reference
	 * point/centroid.
	 */
	RT_Vector_2D_T velocity_otg;
	/**
	 * Unit: [m^2/s^2]
	 * 
	 * Error covariance of the x and y over the ground velocity of the object.
	 */
	RT_Vector_Elem_T velocity_otg_covariance;
	/**
	 * Unit: [m^2/s^2]
	 * 
	 * Variance of the x and y over the ground velocity of the object.
	 */
	RT_Vector_2D_T velocity_otg_variance;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_D5D3EA09_6434_46c6_8A58_451406570B65__INCLUDED_)*/
 