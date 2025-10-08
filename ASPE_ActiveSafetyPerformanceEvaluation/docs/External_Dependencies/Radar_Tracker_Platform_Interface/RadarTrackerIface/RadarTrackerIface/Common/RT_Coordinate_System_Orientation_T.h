/****************************************************
 *  RT_Coordinate_System_Orientation_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Enumeration RT_Coordinate_System_Orientation_T       
 *  Original author: lzrq93                     
 ****************************************************/

#if !defined(EA_C6AAF9BC_C0A3_44ee_BB6C_1642D0E7FAA0__INCLUDED_)
#define EA_C6AAF9BC_C0A3_44ee_BB6C_1642D0E7FAA0__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Comment to this enum definition.
 */
typedef enum
{
	/**
	 * This constant indicates that no statement is given.
	 */
	RT_COORDINATE_SYSTEM_ORIENTATION_UNDEFINED = 0,
	RT_COORDINATE_SYSTEM_ORIENTATION_RHS_XBOW_ZDOWN = 2,
	RT_COORDINATE_SYSTEM_ORIENTATION_RHS_XBOW_ZUP = 1,
	/**
	 * This constant indicates that the reference is invalid.
	 */
	RT_COORDINATE_SYSTEM_ORIENTATION_INVALID = 255
} RT_Coordinate_System_Orientation_T;


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_C6AAF9BC_C0A3_44ee_BB6C_1642D0E7FAA0__INCLUDED_)*/
 