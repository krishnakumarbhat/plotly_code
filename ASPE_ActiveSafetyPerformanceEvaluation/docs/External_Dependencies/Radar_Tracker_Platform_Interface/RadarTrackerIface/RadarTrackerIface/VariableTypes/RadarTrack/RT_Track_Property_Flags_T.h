/****************************************************
 *  RT_Track_Property_Flags_T.h                                         
 *  Created on: 11-Oct-2019 2:20:16 PM                      
 *  Implementation of the Class RT_Track_Property_Flags_T       
 *  Original author: hjschg                     
 ****************************************************/

#if !defined(EA_212F2956_6C51_4c84_9D5E_886CF194064A__INCLUDED_)
#define EA_212F2956_6C51_4c84_9D5E_886CF194064A__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Unit: []
 * 
 * Contains flags for indicating that the reported track has certain properties.
 * Currently, these are that is is likely to be a reflection, a ghost or that it
 * was part of a split or merge in the current cycle. There are 5 reserved bits
 * which might, e.g., be used for things like under-/overdrivability
 * classification in the future.
 */
struct RT_Track_Property_Flags_T
{
	/**
	 * Indicates whether the object likely is a ghost (i.e., not corresponding to a
	 * real-world entity)
	 */
	bitfield8_t ghost:1;
	/**
	 * Indicates whether the object is assumed to be caused by a reflection (i.e., not
	 * corresponding to a real-world entity)
	 */
	bitfield8_t reflection:1;
	/**
	 * Indicates whether the object was part of a split or merge in the current cycle
	 */
	bitfield8_t split_merge:1;
} ;



#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_212F2956_6C51_4c84_9D5E_886CF194064A__INCLUDED_)*/
 