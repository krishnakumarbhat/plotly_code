/****************************************************
 *  RT_Coordinate_System_Type_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Enumeration RT_Coordinate_System_Type_T       
 *  Original author: lzrq93                     
 ****************************************************/

#if !defined(EA_2BEE0FE2_4F8D_4d8b_917E_F14CF53C0915__INCLUDED_)
#define EA_2BEE0FE2_4F8D_4d8b_917E_F14CF53C0915__INCLUDED_


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
	RT_COORDINATE_SYSTEM_TYPE_UNDEFINED = 0,
	/**
	 * This constant indicates ..
	 */
	RT_COORDINATE_SYSTEM_TYPE_ISO_WORLD = 1,
	RT_COORDINATE_SYSTEM_TYPE_ISO_INTERMEDIATE = 2,
	RT_COORDINATE_SYSTEM_TYPE_ISO_VCS = 3,
	/**
	 * This constant indicates that the reference is invalid.
	 */
	RT_COORDINATE_SYSTEM_TYPE_INVALID = 255
} RT_Coordinate_System_Type_T;


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_2BEE0FE2_4F8D_4d8b_917E_F14CF53C0915__INCLUDED_)*/
 