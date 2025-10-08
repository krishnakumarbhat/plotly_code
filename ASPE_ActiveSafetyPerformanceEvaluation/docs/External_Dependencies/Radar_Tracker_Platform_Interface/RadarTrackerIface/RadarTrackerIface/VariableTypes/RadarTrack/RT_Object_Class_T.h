/****************************************************
 *  RT_Object_Class_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Enumeration RT_Object_Class_T       
 *  Original author: lj3w05                     
 ****************************************************/

#if !defined(EA_129EF11B_51D2_454e_AF7A_01CBC03F2A6D__INCLUDED_)
#define EA_129EF11B_51D2_454e_AF7A_01CBC03F2A6D__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

enum RT_Object_Class_T
{
	RT_Object_Class_UNDEFINED = 0,
	RT_Object_Class_PEDESTRIAN = 1,
	RT_Object_Class_TWOWHEEL = 2,
	RT_Object_Class_CAR = 3,
	RT_Object_Class_TRUCK = 4,
	RT_Object_Class_UNKNOWN = 5
};


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_129EF11B_51D2_454e_AF7A_01CBC03F2A6D__INCLUDED_)*/
 