/****************************************************
 *  RT_Bounding_Box_Var_2D_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Class RT_Bounding_Box_Var_2D_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_4B958A81_E017_441a_8641_D58110EE90B3__INCLUDED_)
#define EA_4B958A81_E017_441a_8641_D58110EE90B3__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * These attributes measure the uncertainty of the distances between the
 * centroid/reference point and the different edges of the bounding box and the
 * uncertainty of the orientation angle.
 */
struct RT_Bounding_Box_Var_2D_T
{
	RT_Vector_Element_T lat_shift_left;
	RT_Vector_Element_T lat_shift_right;
	RT_Vector_Element_T long_shift_front;
	RT_Vector_Element_T long_shift_rear;
	RT_Angle_T orientation_local;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_4B958A81_E017_441a_8641_D58110EE90B3__INCLUDED_)*/
 