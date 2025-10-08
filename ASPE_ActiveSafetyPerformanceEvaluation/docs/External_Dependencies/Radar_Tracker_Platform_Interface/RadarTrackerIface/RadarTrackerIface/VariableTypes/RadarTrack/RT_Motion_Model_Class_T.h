/****************************************************
 *  RT_Motion_Model_Class_T.h                                         
 *  Created on: 11-Oct-2019 2:20:15 PM                      
 *  Implementation of the Enumeration RT_Motion_Model_Class_T       
 *  Original author: lzrq93                     
 ****************************************************/

#if !defined(EA_204FDA71_A08F_4866_A833_BBC7BA4B955B__INCLUDED_)
#define EA_204FDA71_A08F_4866_A833_BBC7BA4B955B__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Indicates which motion model is used for the object
 */
enum RT_Motion_Model_Class_T
{
	/**
	 * No classification
	 */
	RT_Motion_Model_Class_UNDEFINED = 0,
	/**
	 * General object
	 */
	RT_Motion_Model_Class_CCA = 1,
	/**
	 * General object
	 */
	RT_Motion_Model_Class_CTCA = 2,
	RT_Motion_Model_Class_PCT = 3
};


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_204FDA71_A08F_4866_A833_BBC7BA4B955B__INCLUDED_)*/
 