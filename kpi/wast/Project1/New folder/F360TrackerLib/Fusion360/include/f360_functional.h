/*===========================================================================*\
* FILE: f360_functional.h
*============================================================================
* Copyright (C) 2021 Aptiv Advanced Safety and User Experience. All rights reserved.
* Confidential – Restricted Aptiv information. Do not disclose.
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
* This file contains definition of predicates:
* cmn::less()
* cmn::greater()
*
* Applicable Standards (in order of precedence: highest first):
*    ESGW_4-2_PE-SWX_00-01-A01_EN "Aptiv C++ Coding Standards" [26-May-2019]
*    ESGW_4-2_PE-SWX_00-01-A02_EN "Aptiv C Coding Standards" [12-Mar-2006]
*
\*==========================================================================================*/
#ifndef F360_FUNCTIONAL_H
#define F360_FUNCTIONAL_H

namespace f360_variant_A
{
namespace cmn
{

   /*===========================================================================*\
   * PREDICATE: less()
   *===========================================================================
   * RETURN VALUE:
   * Bool indicating if first element is less than second element. 
   *
   * PARAMETERS:
   * const T & first_elem - First element to compare
   * const T & second_elem - Second element to compare
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Predicate to compare two elements, returns true if the first element is less 
   * than the second element. Can be used to sort a collection in ascending order.
   *
   \*===========================================================================*/
   template <typename T>
   struct f360_less
   {
      bool operator()(const T & first_elem, const T & second_elem) const
      {
         return first_elem < second_elem;
      }
   };

   /*===========================================================================*\
   * PREDICATE: greater()
   *===========================================================================
   * RETURN VALUE:
   * Bool indicating if first element is greater than second element.
   *
   * PARAMETERS:
   * const T & first_elem - First element to compare
   * const T & second_elem - Second element to compare
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Predicate to compare two elements, returns true if the first element is greater
   * than the second element. Can be used to sort a collection in descending order.
   *
   \*===========================================================================*/
   template <typename T>
   struct f360_greater
   {
      bool operator()(const T & first_elem, const T & second_elem) const
      {
         return first_elem > second_elem;
      }
   };

}
}

#endif
