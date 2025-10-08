/****************************************************
 *  RT_Motion_Model_Info_2D_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Class RT_Motion_Model_Info_2D_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_E1B32B71_9420_40ba_B27F_285ABA327FA1__INCLUDED_)
#define EA_E1B32B71_9420_40ba_B27F_285ABA327FA1__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "RT_Motion_Model_Class_T.h"

struct RT_Motion_Model_Info_2D_T
{
	/**
	 * Unit: Depending on the motion model and specific attribute
	 * 
	 * This contains some covariances that are not covered by the other object
	 * attributes. For the currently supported motion models, at most four entries are
	 * needed.
	 * 
	 * In case of CCA: No additional information needed
	 * In case of PCT: Covariance of heading and heading rate
	 * In case of CTCA: Covariance of speed and heading, covariance of tangential
	 * acceleration and curvature
	 */
	RT_Vector_Elem_T[3] covariances_suppl;
	/**
	 * Unit: Depending on the motion model and specific attribute
	 * 
	 * This contains the state variables of the motion model that are not covered by
	 * the other object attributes. For the currently supported motion models, at most
	 * four entries are needed.
	 * 
	 * In case of CCA: No additional information needed (as the states are x position,
	 * y position, x velocity, y velocity, x acceleration, y acceleration)
	 * In case of PCT: heading and heading rate (Not directly states of the motion
	 * model, but still needed)
	 * In case of CTCA: speed, heading, tangential acceleration, curvature
	 */
	RT_Vector_Elem_T[6] state_suppl;
	RT_Motion_Model_Class_T type;
	/**
	 * Unit: Depending on the motion model and specific attribute
	 * 
	 * This contains the variances the state variables of the motion model that are
	 * not covered by the other object attributes. For the currently supported motion
	 * models, at most four entries are needed.
	 * 
	 * In case of CCA: No additional information needed (see above)
	 * In case of PCT: Variance of heading and heading rate
	 * In case of CTCA: Variance speed, heading, tangential acceleration, curvature
	 */
	RT_Vector_Elem_T[6] variances_suppl;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_E1B32B71_9420_40ba_B27F_285ABA327FA1__INCLUDED_)*/
 