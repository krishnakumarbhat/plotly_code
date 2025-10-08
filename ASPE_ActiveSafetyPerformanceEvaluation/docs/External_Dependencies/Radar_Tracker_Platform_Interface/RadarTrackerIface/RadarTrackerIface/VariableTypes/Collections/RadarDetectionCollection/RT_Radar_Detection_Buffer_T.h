/****************************************************
 *  RT_Radar_Detection_Buffer_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Class RT_Radar_Detection_Buffer_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_DCB82BA5_BBDD_438e_B09E_55F1537DA54B__INCLUDED_)
#define EA_DCB82BA5_BBDD_438e_B09E_55F1537DA54B__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "RT_Single_Sensor_Detection_Buffer_T.h"

struct RT_Radar_Detection_Buffer_T
{
	Single_Sensor_Detection_Buffer_T single_sensor_detection_buffer;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_DCB82BA5_BBDD_438e_B09E_55F1537DA54B__INCLUDED_)*/
 