/****************************************************
 *  RT_Movement_Status_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Enumeration RT_Movement_Status_T       
 *  Original author: lzrq93                     
 ****************************************************/

#if !defined(EA_F47493BE_AF40_4016_A0E0_683D7FBA2CA7__INCLUDED_)
#define EA_F47493BE_AF40_4016_A0E0_683D7FBA2CA7__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Indicates whether the object is moving, stopped (currently not moving, but has
 * been seen moving before) or stationary (currently not moving and has never been
 * seen moving).
 */
enum RT_Movement_Status_T
{
	/**
	 * No classification
	 */
	RT_Movement_Status_UNDEFINED = 0,
	/**
	 * General object
	 */
	RT_Movement_Status_STATIONARY = 1,
	/**
	 * General object
	 */
	RT_Movement_Status_STOPPED = 2,
	RT_Movement_Status_MOVING = 3
};


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_F47493BE_AF40_4016_A0E0_683D7FBA2CA7__INCLUDED_)*/
 