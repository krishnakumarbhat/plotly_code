/*===================================================================================*\
* FILE:  f360_sort_data_type.h
*====================================================================================

*Copyright (C) 2020 Aptiv Advanced Safety and User Experience. All rights reserved.

* Confidential – Restricted Aptiv information. Do not disclose."

*-----------------------------------------------------------------------------------------
* DESCRIPTION:
* This file contains declaration of struct used in F360_Sort function.
*
*

* Applicable Standards (in order of precedence: highest first):

* ESGW_4-2_PE-SWX_00-01-A01_EN, "APTIV C++ Coding Standards"[May 26, 2019]

* ESGW_4-2_PE-SWX_00-01-A02_EN, "APTIV C Coding Standards" [12-Mar-2006]

***/
#ifndef F360_SORT_DATA_TYPE_H
#define F360_SORT_DATA_TYPE_H

#include "f360_reuse.h"

namespace f360_variant_A
{
   struct F360_Sort_Data_T
   {
      float32_t data;
      uint32_t index;

      /*===========================================================================*\
      * Function: F360_Sort_Data_T::operator<
      *===========================================================================
      * RETURN VALUE:
      * bool - flag indicating whether this struct parameter is smaller than other 
      * struct parameter.
      *
      * PARAMETERS:
      * const F360_Sort_Data_T& other - reference to struct that we are comparing to
      *
      * EXTERNAL REFERENCES:
      * None.
      *
      * DEVIATIONS FROM STANDARDS:
      * None.
      *
      * --------------------------------------------------------------------------
      * ABSTRACT:
      * --------------------------------------------------------------------------
      * This function compares whether this struct field 'data' is smaller than other
      * struct field 'data'.
      *
      * PRECONDITIONS:
      * None.
      *
      * POSTCONDITIONS:
      * None
      *
      \*===========================================================================*/
      bool operator<(const F360_Sort_Data_T& other) const
      {
         return (this->data < other.data);
      }

      /*===========================================================================*\
      * Function: F360_Sort_Data_T::operator>
      *===========================================================================
      * RETURN VALUE:
      * bool - flag indicating whether this struct parameter is greater than other
      * struct parameter.
      *
      * PARAMETERS:
      * const F360_Sort_Data_T& other - reference to struct that we are comparing to
      *
      * EXTERNAL REFERENCES:
      * None.
      *
      * DEVIATIONS FROM STANDARDS:
      * None.
      *
      * --------------------------------------------------------------------------
      * ABSTRACT:
      * --------------------------------------------------------------------------
      * This function compares whether this struct field 'data' is greater than other
      * struct field 'data'.
      *
      * PRECONDITIONS:
      * None.
      *
      * POSTCONDITIONS:
      * None
      *
      \*===========================================================================*/
      bool operator>(const F360_Sort_Data_T& other) const
      {
         return (this->data > other.data);
      }
   };
}
#endif
