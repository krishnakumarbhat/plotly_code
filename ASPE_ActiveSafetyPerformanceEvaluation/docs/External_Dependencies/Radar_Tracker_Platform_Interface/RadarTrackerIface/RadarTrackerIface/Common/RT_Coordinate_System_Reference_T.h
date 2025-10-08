/****************************************************
 *  RT_Coordinate_System_Reference_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Enumeration RT_Coordinate_System_Reference_T       
 *  Original author: lzrq93                     
 ****************************************************/

#if !defined(EA_C85AF8CE_7C2B_4f99_865A_D72300373746__INCLUDED_)
#define EA_C85AF8CE_7C2B_4f99_865A_D72300373746__INCLUDED_


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
	RT_COORDINATE_SYSTEM_REFERENCE_UNDEFINED = 0,
	RT_COORDINATE_SYSTEM_REFERENCE_CENTER_FRONT_BUMPER = 1,
	RT_COORDINATE_SYSTEM_REFERENCE_CENTER_REAR_AXLE = 2,
	/**
	 * This constant indicates that the reference is invalid.
	 */
	RT_COORDINATE_SYSTEM_REFERENCE_INVALID = 255
} RT_Coordinate_System_Reference_T;


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_C85AF8CE_7C2B_4f99_865A_D72300373746__INCLUDED_)*/
 