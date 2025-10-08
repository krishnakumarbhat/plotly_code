/****************************************************
 *  RT_Radar_Track_Collection_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Class RT_Radar_Track_Collection_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_1BD9C189_9DFE_4717_A247_12BF88EADB02__INCLUDED_)
#define EA_1BD9C189_9DFE_4717_A247_12BF88EADB02__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

#include "RT_Radar_Track_T.h"

struct RT_Radar_Track_Collection_T
{
	RT_Collection_Element_Count_T element_count;
	RT_Radar_Track_T radar_tracks[RT_RADAR_TRACK_COLLECTION_ARRAY_SIZE];
	RT_Timestamp_T timestamp;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_1BD9C189_9DFE_4717_A247_12BF88EADB02__INCLUDED_)*/
 