/****************************************************
 *  RT_Bounding_Box_2D_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Class RT_Bounding_Box_2D_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_EC7910F5_80F8_44be_9C35_DB57849319E3__INCLUDED_)
#define EA_EC7910F5_80F8_44be_9C35_DB57849319E3__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "RT_Vector_2D_T.h"

struct RT_Bounding_Box_2D_T
{
	/**
	 * Unit: [m]
	 * 
	 * These attributes specify the length and width of the object’s bounding box.
	 */
	RT_Vector_2D_T dimensions;
	/**
	 * Unit: [rad]
	 * 
	 * Orientation angle of object's bounding box.
	 */
	RT_Angle_T orientation;
	/**
	 * Unit: []                 Range: [0, 1]
	 * 
	 * This attribute specifies the relative position of the object’s reference
	 * point/centroid inside its bounding box in orthogonal direction.
	 */
	RT_Vector_Element_T refpoint_lat_offset_ratio;
	/**
	 * Unit: []                 Range: [0, 1]
	 * 
	 * This attribute specifies the relative position of the object’s reference
	 * point/centroid inside its bounding box in orthogonal direction.
	 */
	RT_Vector_Element_T refpoint_long_offset_ratio;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_EC7910F5_80F8_44be_9C35_DB57849319E3__INCLUDED_)*/
 