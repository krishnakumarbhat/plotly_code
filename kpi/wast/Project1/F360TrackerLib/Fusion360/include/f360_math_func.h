/*===================================================================================*\
* FILE: f360_math_func.h
*====================================================================================
* Copyright 2017 Delphi Technologies, Inc., All Rights Reserved.
* Delphi Confidential
*-----------------------------------------------------------------------------------------
* %full_filespec: %
* %version: %
* %derived_by: %
* %date_created: %
* or
* $SOURCE: $
* $REVISION: $
* $AUTHOR: $
*-----------------------------------------------------------------------------------------
*
* DESCRIPTION:
*   This file contains function signature Matlab related build in functions
*
* ABBREVIATIONS:
*  None
*
* TRACEABILITY INFO:
*   Design Document(s):
*
*   Requirements Document(s):
*
*   Applicable Standards (in order of precedence: highest first):
*     ESGW_4-2_PE-SWX_00-01-A02_EN "Delphi C Coding Standards" [12-Mar-2006]
*
* DEVIATIONS FROM STANDARDS:
*   None.
*
\*==========================================================================================*/
#ifndef F360_MATH_FUNC_H
#define F360_MATH_FUNC_H

#include "f360_math.h"
#include "f360_reuse.h"
#include "f360_constants.h"
#include "f360_iterator.h"
#include "f360_sort_data_type.h"
#include "f360_functional.h"

#include <numeric>
#include <algorithm>
#include <cassert>
#include <cstring>


namespace f360_variant_A
{
   /*===========================================================================*\
   * FUNCTION: merge_sublists()
   *============================================================================*/
   template <typename T, typename Compare, size_t N>
   void merge_sublists(const uint32_t sublist1_start, const uint32_t sublist2_start, const uint32_t merged_list_end,
      T(&in_list)[N], T(&out_list)[N], const Compare comp)
   {
      uint32_t sublist1_idx = sublist1_start;
      uint32_t sublist2_idx = sublist2_start;
      uint32_t outlist_idx = sublist1_start;

      // In the last iteration we could end up here with only one sublist
      if ((sublist2_idx < merged_list_end) && (sublist1_idx < sublist2_start))
      {
         while (true)
         {
            // Place one of the sublist elements in the outlist and update indices
            // Comparing 2 with 1, instead of 1 with 2, results is stability
            bool f_break_condition = false; //misra 6-6-4
            if (comp(in_list[sublist2_idx], in_list[sublist1_idx]))
            {
               out_list[outlist_idx] = in_list[sublist2_idx];
               sublist2_idx++;
               outlist_idx++;
               if (sublist2_idx >= merged_list_end) { f_break_condition = true; }
            }
            else
            {
               out_list[outlist_idx] = in_list[sublist1_idx];
               sublist1_idx++;
               outlist_idx++;
               if (sublist1_idx >= sublist2_start) { f_break_condition = true; } 
            }
            if (f_break_condition)
            {
               break;
            }
         }
      }

      // One sublist is empty, move everything from remaining sublist
      while ((sublist1_idx < sublist2_start) && (sublist1_idx < merged_list_end))
      {
         out_list[outlist_idx] = in_list[sublist1_idx];
         outlist_idx++;
         sublist1_idx++;
      }

      while (sublist2_idx < merged_list_end)
      {
         out_list[outlist_idx] = in_list[sublist2_idx];
         outlist_idx++;
         sublist2_idx++;
      }
   }

   /*===========================================================================*\
   * FUNCTION: f360_mergesort()
   *============================================================================*/
   template <typename T, typename Compare, size_t N>
   void f360_mergesort(T(&array_to_sort)[N], const uint32_t count, const Compare comp)
   {
      T buffer[N];
      bool f_result_in_buffer = false;

      // First iteration. Sublist length 1 -> 2
      for (uint32_t i = 1U; i < count; i += 2U)
      {
         if (comp(array_to_sort[i], array_to_sort[i - 1U]))
         {
            const T temp = array_to_sort[i];
            array_to_sort[i] = array_to_sort[i - 1U];
            array_to_sort[i - 1U] = temp;
         }
      }

      // Subsequent iterations
      uint32_t sublist_length = 2U;
      while (sublist_length < count)
      {
         // Call merge_sublists with either array_to_sort or buffer as input list
         if (f_result_in_buffer)
         {
            // Loop over all sublists
            for (uint32_t i = 0U; i < count; i += 2U * sublist_length)
            {
               // Make sure we don't go outside the size of the array to sort and call merge function
               const uint32_t merged_list_end = (count < (i + 2U * sublist_length)) ? count : (i + 2U * sublist_length);
               merge_sublists(i, i + sublist_length, merged_list_end, buffer, array_to_sort, comp);
            }
         }
         else
         {
            for (uint32_t i = 0U; i < count; i += 2U * sublist_length)
            {
               const uint32_t merged_list_end = (count < (i + 2U * sublist_length)) ? count : (i + 2U * sublist_length);
               merge_sublists(i, i + sublist_length, merged_list_end, array_to_sort, buffer, comp);
            }
         }

         // Keep track if the result is in array_to_sort or in buffer
         f_result_in_buffer = !f_result_in_buffer;
         sublist_length = 2U * sublist_length;
      }

      // If the merged list is currently in the buffer
      if (f_result_in_buffer)
      {
         const size_t list_size = static_cast<size_t>(count) * sizeof(T);
         (void)memcpy(array_to_sort, &buffer[0], list_size);
      }
   }


   /*===========================================================================*\
   * FUNCTION: F360_Max_Element()
   *===========================================================================
   * RETURN VALUE:
   * T max_value
   *
   * PARAMETERS:
   * const T (&input_arr)[N] -  Array to search for max value
   * const uint32_t num_ele = N - Number of elements to compare (Default: All elements)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds the max value of the first num_ele elements in the array.
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the input array.
   *
   \*===========================================================================*/
   template<typename T, std::size_t N>
   T F360_Max_Element(
      const T(&input_arr)[N],
      const uint32_t num_ele = N)
   {
      assert(num_ele <= N);

      const T * const end = &input_arr[num_ele];

      return *std::max_element(cmn::begin(input_arr), end);
   }

   /*===========================================================================*\
   * FUNCTION: F360_Max_Element_Bounded()
   *===========================================================================
   * RETURN VALUE:
   * T max_value
   *
   * PARAMETERS:
   * const T (&input_arr)[N] -  Array to search for max value
   * const uint32_t start_idx - index from which the array iteration begins
   * const uint32_t end_idx - index of the element after the last element considered in the range
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds the max value element in the array between specified indexes.
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the index range.
   *
   \*===========================================================================*/

   template<typename T, std::size_t N>
   T F360_Max_Element_Bounded(
      const T(&input_arr)[N],
      const uint32_t start_idx,
      const uint32_t end_idx)
   {
      assert(end_idx - start_idx <= N);

      return *std::max_element(&input_arr[start_idx], &input_arr[end_idx]);
   }

   /*===========================================================================*\
   * FUNCTION: F360_Min_Index()
   *===========================================================================
   * RETURN VALUE:
   * uint32_t min_idx - Index in array where minimum value is located.
   *
   * PARAMETERS:
   * const T (&input_arr)[N] -  Array to search for min value
   * const uint32_t num_ele = N - Number of elements to compare (Default: All elements)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds the min value of the first num_ele elements in the array and returns
   * the index to where the min value is located in the array.
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the input array.
   *
   \*===========================================================================*/
   template<typename T, std::size_t N>
   uint32_t F360_Min_Index(
      const T (&input_arr)[N],
      const uint32_t num_ele = N)
   {
      assert(num_ele <= N);

      const T * const begin = cmn::begin(input_arr);
      const T * const end = &input_arr[num_ele];
      const T * const p_min_value = std::min_element(begin, end);

      return static_cast<uint32_t>(std::distance(begin, p_min_value));
   }

   /*===========================================================================*\
* FUNCTION: F360_Max_Index()
*===========================================================================
* RETURN VALUE:
* uint32_t max_idx - Index in array where maximum value is located.
*
* PARAMETERS:
* const T (&input_arr)[N] -  Array to search for max value
* const uint32_t num_ele = N - Number of elements to compare (Default: All elements)
*
* --------------------------------------------------------------------------
* ABSTRACT:
* --------------------------------------------------------------------------
* This function finds the max value of the first num_ele elements in the array and returns
* the index to where the max value is located in the array.
*
* PRECONDITIONS:
* The number of elements to check must be less than or equal to the length of the input array.
*
\*===========================================================================*/
   template<typename T, std::size_t N>
   uint32_t F360_Max_Index(
      const T(&input_arr)[N],
      const uint32_t num_ele = N)
   {
      assert(num_ele <= N);

      const T * const begin = cmn::begin(input_arr);
      const T * const end = &input_arr[num_ele];
      const T * const p_max_value = std::max_element(begin, end);

      return static_cast<uint32_t>(std::distance(begin, p_max_value));
   }

   /*===========================================================================*\
   * FUNCTION: F360_Min_Element()
   *===========================================================================
   * RETURN VALUE:
   * T min_value
   *
   * PARAMETERS:
   * const T (&input_arr)[N] - Array to search for min value
   * uint32_t num_ele = N - Number of elements to compare (Default: All elements)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds the min value of the first num_ele elements in the array.
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the input array.
   *
   \*===========================================================================*/
   template<typename T, std::size_t N>
   T F360_Min_Element(
      const T(&input_arr)[N],
      const uint32_t num_ele = N)
   {
      assert(num_ele <= N);

      const T * const end = &input_arr[num_ele];

      return *std::min_element(cmn::begin(input_arr), end);
   }

   /*===========================================================================*\
   * FUNCTION: F360_Min_Kth_Element()
   *===========================================================================
   * RETURN VALUE:
   * T min_value
   *
   * PARAMETERS:
   * const T (&input_arr)[N] - Array to search for kth min value
   * uint32_t num_ele = N - Number of elements to compare (Default: All elements)
   * kth_element - value representing kth min element in array
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds the kth min value in the array.
   * For kth_element = 1 function returns just the minimum element like the function F360_Min_Element()
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the input array.
   * kth_element must be greater or equal than 1.
   \*===========================================================================*/
   template<typename T, std::size_t N>
   T F360_Min_Kth_Element(
      const T(&input_arr)[N], 
      const uint32_t kth_element,
      const uint32_t num_ele = N)
   {
      assert(num_ele <= N);
      assert(kth_element > 0U);
      assert(kth_element <= N);
      assert(kth_element <= num_ele);

      T tmp_array[N];
      for (uint32_t i = 0U; i < num_ele; i++)
      {
         tmp_array[i] = input_arr[i];
      }
      f360_mergesort(tmp_array, num_ele, cmn::f360_less<T>());
      return tmp_array[kth_element - 1U];
   }

   /*===========================================================================*\
   * FUNCTION: F360_Any()
   *===========================================================================
   * RETURN VALUE:
   * bool
   *
   * PARAMETERS:
   * const bool (&array)[N] 
   * uint32_t num_elements
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function find if any non - false element present in the array
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the input array.
   *
   \*===========================================================================*/
   template<std::size_t N>
   bool F360_Any(
      const bool (&array)[N],
      const uint32_t num_elements = N)
   {
      assert(num_elements <= N);

      const bool * const end = &array[num_elements];
      const bool * const p_non_zero_element = std::find(cmn::begin(array), end, true);
      
      return p_non_zero_element != end;
   }

   void F360_matmul_6x6_6x6(
      const float32_t (&X)[6][6],
      const float32_t (&Y)[6][6],
      float32_t (&result)[6][6]);

   void F360_matmul_4x4_4x4(
      const float32_t(&X)[4][4],
      const float32_t(&Y)[4][4],
      float32_t(&result)[4][4]);

   void F360_matmul_6x6_6x6T(
      const float32_t (&X)[6][6],
      const float32_t (&Y)[6][6],
      float32_t (&result)[6][6]);

   void F360_matmul_6x6_6x6T_symmetric_matrix(
      const float32_t(&X)[6][6],
      const float32_t(&Y)[6][6],
      float32_t(&result)[6][6]);

   void F360_matmul_4x4_4x4T(
      const float32_t(&X)[4][4],
      const float32_t(&Y)[4][4],
      float32_t(&result)[4][4]);

   void F360_matmul_4x4_4x4T_symmetric_matrix(
      const float32_t(&X)[4][4],
      const float32_t(&Y)[4][4],
      float32_t(&result)[4][4]);

   void F360_matadd_6x6(
      const float32_t (&X)[6][6],
      const float32_t (&Y)[6][6],
      float32_t (&result)[6][6]);

   void F360_matadd_6x6_symmetric_matrix(
      const float32_t(&X)[6][6],
      const float32_t(&Y)[6][6],
      float32_t(&result)[6][6]);

   void F360_matadd_4x4(
      const float32_t (&X)[4][4],
      const float32_t (&Y)[4][4],
      float32_t (&result)[4][4]);

   void F360_matadd_4x4_symmetric_matrix(
      const float32_t(&X)[4][4],
      const float32_t(&Y)[4][4],
      float32_t(&result)[4][4]);

   void F360_matsub_6x6(
      const float32_t(&X)[6][6],
      const float32_t(&Y)[6][6],
      float32_t(&result)[6][6]);

   void F360_matsub_4x4(
      const float32_t (&X)[4][4],
      const float32_t (&Y)[4][4],
      float32_t (&result)[4][4]);

   /*===========================================================================*\
   * FUNCTION: F360_Matmul_MxN_NxP()
   *===========================================================================
   * RETURN VALUE:
   *
   * PARAMETERS:
   * const T (&mat1)[M][N] - First Matrix to be multiplied
   * const T (&mat2)[N][P] - Second Matrix to be multiplied
   * T (&result_mat)[M][P]
   * const uint32_t row1 - Number of rows to multiply in the first matrix (Default: Max)
   * const uint32_t col1 - Number of columns to multiply in the first matrix (Default: Max)
   * const uint32_t col2 - Number of columns to multiply in the second matrix (Default: Max)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function find matrix multiplication of MxN * NxP
   *
   \*===========================================================================*/
   template<typename T, std::size_t M, std::size_t N, std::size_t P>
   void F360_Matmul_MxN_NxP(
      const T (&mat1)[M][N],
      const T (&mat2)[N][P],
      T (&result_mat)[M][P],
      const uint32_t row1 = M,
      const uint32_t col1 = N,
      const uint32_t col2 = P)
   {
      assert(row1 <= M);
      assert(col1 <= N);
      assert(col2 <= P);

      for (uint32_t row_mat_1 = 0U; row_mat_1 < row1; row_mat_1++)
      {
         for (uint32_t col_mat_2 = 0U; col_mat_2 < col2; col_mat_2++)
         {
            T sum{}; 
            for(uint32_t col_mat_1 = 0U; col_mat_1 < col1; col_mat_1++)
            {
               sum = sum + (mat1[row_mat_1][col_mat_1] * mat2[col_mat_1][col_mat_2]);
            }
            result_mat[row_mat_1][col_mat_2] = sum;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Matmul_MxN_NxP_Transpose()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const T (&mat1)[M][N] - First Matrix to be multiplied
   * const T (&mat2)[P][N] - Second Matrix to be multiplied
   * T (&result_mat)[M][P]
   * const uint32_t row1 - Number of rows to multiply in the first matrix (Default: Max)
   * const uint32_t col1 - Number of columns to multiply in the first matrix (Default: Max)
   * const uint32_t row2 - Number of rows to multiply in the second matrix (Default: Max)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function find matrix multiplication of MxN * Transpose(PxN)
   *
   * PRECONDITIONS:
   * The number of rows in mat1 to multiply must be less than or equal to M.
   * The number of columns in mat1 to multiply must be less than or equal to N.
   * The number of rows in mat2 to multiply must be less than or equal to P.
   *
   \*===========================================================================*/
   template<typename T, std::size_t M, std::size_t N, std::size_t P>
   void F360_Matmul_MxN_PxN_Transpose(
      const T (&mat1)[M][N],
      const T (&mat2)[P][N],
      T (&result_mat)[M][P],
      const uint32_t row1 = M,
      const uint32_t col1 = N,
      const uint32_t row2 = P)
   {
      assert(row1 <= M);
      assert(col1 <= N);
      assert(row2 <= P);
      
      for (uint32_t row_mat_1 = 0U; row_mat_1 < row1; row_mat_1++)
      {
         // The second matrix is not transposed, row_mat_2 is col_mat_2 in the transposed matrix
         for (uint32_t row_mat_2 = 0U; row_mat_2  < row2; row_mat_2++) 
         {
            T sum{};
            for (uint32_t col_mat_1 = 0U; col_mat_1 < col1; col_mat_1++)
            {
               sum = sum + (mat1[row_mat_1][col_mat_1] * mat2[row_mat_2][col_mat_1]);
            }
            result_mat[row_mat_1][row_mat_2] = sum;
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Matmul_MxN_NxP_Transpose_symmetric_matrix()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const T (&mat1)[M][N] - First Matrix to be multiplied
   * const T (&mat2)[P][N] - Second Matrix to be multiplied
   * T (&result_mat)[M][P]
   * const uint32_t row1 - Number of rows to multiply in the first matrix (Default: Max)
   * const uint32_t col1 - Number of columns to multiply in the first matrix (Default: Max)
   * const uint32_t row2 - Number of rows to multiply in the second matrix (Default: Max)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function find matrix multiplication of MxN * Transpose(PxN) under the 
   * pre-condition that the resulting matrix is symmetric. (Function can for
   * example be used when computing K * R * K' in Kalman filter measurement update
   * if X = K * R and Y = K since it is known that the resulting K * R * K matrix is
   * a symmetric covariance matrix)
   *
   * PRECONDITIONS:
   * The number of rows in mat1 to multiply must be less than or equal to M.
   * The number of columns in mat1 to multiply must be less than or equal to N.
   * The number of rows in mat2 to multiply must be less than or equal to P.
   * mat1 *mat2 is symmetric
   *
   \*===========================================================================*/
   template<typename T, std::size_t M, std::size_t N, std::size_t P>
   void F360_Matmul_MxN_PxN_Transpose_symmetric_matrix(
      const T(&mat1)[M][N],
      const T(&mat2)[P][N],
      T(&result_mat)[M][P],
      const uint32_t row1 = M,
      const uint32_t col1 = N,
      const uint32_t row2 = P)
   {
      assert(row1 <= M);
      assert(col1 <= N);
      assert(row2 <= P);

      for (uint32_t row_mat_1 = 0U; row_mat_1 < row1; row_mat_1++)
      {
         // The second matrix is not transposed, row_mat_2 is col_mat_2 in the transposed matrix
         for (uint32_t row_mat_2 = 0U; row_mat_2 < row2; row_mat_2++)
         {
            if (row_mat_1 <= row_mat_2)
            {
               T sum{};
               for (uint32_t col_mat_1 = 0U; col_mat_1 < col1; col_mat_1++)
               {
                  sum = sum + (mat1[row_mat_1][col_mat_1] * mat2[row_mat_2][col_mat_1]);
               }
               result_mat[row_mat_1][row_mat_2] = sum;
            }
            else
            {
               result_mat[row_mat_1][row_mat_2] = result_mat[row_mat_2][row_mat_1];
            }
         }
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Mat_Vec_Mul_MxN_N()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const T (&mat)[M][N] - Matrix to be multiplied
   * const T (&vec)[N] - Vector to be multiplied 
   * T (&result_vec)[L] - Result vector 
   * const uint32_t num_row = M - Number of rows to be multiplied (Default: all rows)
   * const uint32_t num_col = N - Number of columns to be multiplied (Default: all columns)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function calculates matrix vector multiplication of MxN * N
   *
   \*===========================================================================*/
   template<typename T, std::size_t M, std::size_t N>
   void F360_Mat_Vec_Mul_MxN_N(
      const T (&mat)[M][N],
      const T (&vec)[N],
      T (&result_vec)[M],
      const uint32_t num_row = M,
      const uint32_t num_col = N)
   {
      assert(num_row <= M);
      assert(num_col <= N);

      for (uint32_t row = 0U; row < num_row; row++)
      {
         T sum{};
         for (uint32_t col = 0U; col < num_col; col++)
         {
            sum = sum + (mat[row][col] * vec[col]);
         }
         result_vec[row] = sum;
      }
   }

   /*===========================================================================*\
   * FUNCTION: F360_Vector_Multiplication()
   *===========================================================================
   * RETURN VALUE:
   * T result - Resulting scalar value of the vector multiplication
   *
   * PARAMETERS:
   * const T (&vec1)[N] - First vector to multiply 
   * const T (&vec2)[N] - Second vector to multiply 
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function multiplies two vectors of size N with each other 
   *
   \*===========================================================================*/
   template<typename T, std::size_t N>
   T F360_Vector_Multiplication(
      const T (&vec1)[N],
      const T (&vec2)[N])
   {
      T result{};
      for (uint32_t elem = 0U; elem < N; elem++)
      {
         result += vec1[elem] * vec2[elem];
      }
      return result;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Mean()
   *===========================================================================
   * RETURN VALUE:
   * float32_t : Mean of array 
   *
   * PARAMETERS:
   * const float32_t (&input_arr)[N] - Input array
   * const uint32_t num_ele - Number of elements in the array
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds the mean of the passed array
   * If the number of elements are given to be 0, then the function returns a zero mean.
   *
   * PRECONDITIONS:
   * The number of elements to check must be less than or equal to the length of the input array.
   *
   \*===========================================================================*/
   template<std::size_t N>
   float32_t F360_Mean(
      const float32_t (&input_arr)[N], 
      const uint32_t num_ele)
   {
      assert(num_ele <= N);

      float32_t mean;
      if (num_ele > 0U)
      {
         const float32_t sum = std::accumulate(&input_arr[0], &input_arr[num_ele], 0.0F);
         mean = (sum / static_cast<float32_t>(num_ele));
      }
      else
      {
         // Safe state to avoid division with zero is to return 0 mean
         mean = 0.0F;
      }

      return mean;
   }

   float32_t F360_Get_Hypotenuse(
      const float32_t a,
      const float32_t b);

   float32_t F360_Get_Hypotenuse_Squared(
      const float32_t a,
      const float32_t b);

   /*===========================================================================*\
   * FUNCTION: F360_Sort()
   *===========================================================================
   * RETURN VALUE:
   * bool sort_successful - Indicates that sorting was done correctly.
   *
   * PARAMETERS:
   * float32_t (&arr)[N] - Array to sort
   * const uint32_t num_elements - Number of elements to sort
   * const bool f_ascend - True if sorting should be done in ascending order
   * uint32_t (&perm)[N] - Permutation
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function does sorting for a given array and provides the permutation.
   * If num_elements is larger than N, then nothing is done and function returns false
   *
   * PRECONDITIONS:
   * The number of elements to sort must be less than or equal to N.
   *
   \*===========================================================================*/
   template <std::size_t N>
   bool F360_Sort(
      float32_t (&arr)[N],
      const uint32_t num_elements,
      const bool f_ascending,
      uint32_t (&perm)[N])
   {
      bool sort_successful;
      if (num_elements == 0U)
      {
         // Nothing to sort
         sort_successful = true;
      }
      else if(num_elements <= N)
      {
         F360_Sort_Data_T work_data[N];
         for (uint32_t i = 0U; i < num_elements; i++)
         {
            work_data[i].index = i;
            work_data[i].data = arr[i];
         }

         if (f_ascending)
         {
            f360_mergesort(work_data, num_elements, cmn::f360_less<F360_Sort_Data_T>());

         }
         else
         {
            f360_mergesort(work_data, num_elements, cmn::f360_greater<F360_Sort_Data_T>());
         }

         for (uint32_t i = 0U; i < num_elements; i++)
         {
            perm[i] = work_data[i].index;
            arr[i] = work_data[i].data;
         }
         sort_successful = true;
      }
      else
      {
         sort_successful = false;
      }

      return sort_successful;
   }

   /*===========================================================================*\
   * FUNCTION: F360_Transpose_2D()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const T (&mat1)[M][N] - input 2D matrix of dimension MxN
   * T (&mat2)[N][M] - transposed 2D matrix of dimension NxM
   * const uint32_t rows - num rows of input matrix to be transposed (default M)
   * const uint32_t cols - num columns of input matrix to be transposed (default N)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function find the transpose of a given matrix.
   *
   * PRECONDITIONS:
   * - The number of rows in the first matrix and the number of columns in the second matrix 
   *   is less than or equal to the number of rows to be transposed.
   * - The number of columns in the first matrix and the number of rows in the second matrix 
   *   is less than or equal to the number of columns to be transposed.
   *
   \*===========================================================================*/
   template<typename T, std::size_t M, std::size_t N, std::size_t L, std::size_t P>
   void F360_Transpose_2D(
      const T (&mat1)[M][N],
      T (&mat2)[L][P],
      const uint32_t rows = M,
      const uint32_t cols = N)
   {
      assert(rows <= P);
      assert(rows <= M);
      assert(cols <= L);
      assert(cols <= N);

      for (uint32_t row_mat_1 = 0U; row_mat_1 < rows; row_mat_1++)
      {
         for (uint32_t col_mat_1 = 0U; col_mat_1 < cols; col_mat_1++)
         {
            mat2[col_mat_1][row_mat_1] = mat1[row_mat_1][col_mat_1];
         }
      }
   }

   float32_t F360_Saturate(
      const float32_t input,
      const float32_t min_value,
      const float32_t max_value);

   float32_t F360_Linear_Equation(
      const float32_t x,
      const float32_t x1,
      const float32_t x2,
      const float32_t y1,
      const float32_t y2);

   float32_t F360_Linear_Equation_With_Saturation(
      const float32_t x,
      const float32_t x1,
      const float32_t x2,
      const float32_t y1,
      const float32_t y2);

   /*===========================================================================*\
   * FUNCTION: F360_Piecewise_Linear_Equation()
   *===========================================================================
   * RETURN VALUE:
   * float32_t - the value of the piecewise linear equation in point x
   *
   * PARAMETERS:
   * const float32_t x - the x coordinate to evaluate the piecewise linear equation in
   * const float32_t (&breakpoints_x)[N] - An array containing the breakpoints of the linear piecewise equation
   * const float32_t (&breakpoints_y)[N] - An array containing the values of the linear piecewise equation in the breakpoints
   * const int32_t num_breakpoints - number of breakpoints
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * Execution of a piecewise linear function in a given point
   *
   * PRECONDITIONS:
   * - Elements in breakpoints_x must be unique and strictly increasing.
   * - breakpoints_x and breakpoints_y shall have the same number of elements which in turn corresponds to num_breakpoints
   *   and breakpoints_y[ind] should correspond to breakpoints_x[ind].*
   *
   * POSTCONDITIONS:
   * - If the given x_question is lower than lowest element in breakpoints_x or larger than the highest element of breakpoints_x,
   * then return y_question equal to corresponding first/last value breakpoints_y respectively.
   *
   \*===========================================================================*/
   template<std::size_t N>
   float32_t F360_Piecewise_Linear_Equation(
      const float32_t x_question,
      const float32_t (&breakpoints_x)[N],
      const float32_t (&breakpoints_y)[N],
      const uint32_t num_breakpoints = N)
   {
      float32_t y_question;
      // Compute value of piecewise linear function
      if (x_question < breakpoints_x[0U])
      {
         // If x is less than the smallest breakpoint_x value then y
         // is choosen as the breakpoint_y value that corresponds to
         // the smallest breakpoint_x value
         y_question = breakpoints_y[0U];
      }
      else if (x_question > breakpoints_x[num_breakpoints - 1U])
      {
         // If x is larger than than the smallest breakpoint_x value
         // then y is choosen as the breakpoint_y value that corresponds
         // to the largest breakpoint_x value
         y_question = breakpoints_y[num_breakpoints - 1U];
      }
      else
      {
         // Find first value in sorted breakpoints_x that are smaller
         // than x
         uint32_t ind = 1U;
         while (x_question > breakpoints_x[ind])
         {
            ind++;
         }
         // We now know that:
         // sorted_breakpoints_x[ind-1]) <= x_question < sorted_breakpoints_x[ind]
         // Define a line y(x) = k*x + m through the points defined by
         // sorted_breakpoints_x[ind-1] and sorted_breakpoints_x[ind] and evaluate
         // y_question for the given input x_question.
         
         // Coordinates of two points on the line y(x) = k*x + m;
         const float32_t point1_x = breakpoints_x[ind - 1U];
         const float32_t point1_y = breakpoints_y[ind - 1U];
         const float32_t point2_x = breakpoints_x[ind];
         const float32_t point2_y = breakpoints_y[ind];
         
         const float32_t min_denominator = 1e-6F;
         if (std::abs(point2_x - point1_x) < min_denominator) // Protect from division with 0
         {
            // Take the mean of point2_y and point1_y
            y_question = 0.5F*(point2_y + point1_y);
         }
         else
         {
            const float32_t k = (point2_y - point1_y) / (point2_x - point1_x);
            const float32_t m = point1_y - k*point1_x;

            y_question = k*x_question + m;
         }
      }

      return y_question;
   }

   float32_t F360_Low_Pass_Filter_First_Order(
      const float32_t new_input,
      const float32_t prev_filt,
      const float32_t filter_coef);

   float32_t F360_Low_Pass_Filter_Angle_First_Order(
      const float32_t new_input,
      const float32_t prev_filt,
      const float32_t filter_coef);

   float32_t F360_2d_Matrix_Determinant(float32_t const (&matrix)[2][2]);

   float32_t F360_Safe_Logf(const float32_t value);

   float32_t F360_Safe_Log10f(const float32_t value);

   float32_t F360_Power_Scale_Prob(
      float32_t prob_in,
      uint32_t exponent);

   float32_t F360_Accumulate_Probabilities(
      const float32_t prob_old,
      const float32_t prob_new);

   void Rotate_2D_Covariance_Matrix_With_Precalc_Coeff(
      const float32_t (&cov_mat)[2][2],
      float32_t (&rotated_cov_mat)[2][2],
      const float32_t sin_sq_angle,
      const float32_t cos_sq_angle,
      const float32_t sin_cos_angle,
      const float32_t cos_2_angle);

   void Rotate_2D_Covariance_Matrix(
      const float32_t cos_rot_angle,
      const float32_t sin_rot_angle,
      const float32_t (&cov_mat)[2][2],
      float32_t (&rotated_cov_mat)[2][2]);

   /*===========================================================================*\
   * FUNCTION: F360_Multiply_2D_Matrix_With_Constant()
   *===========================================================================
   * RETURN VALUE:
   * None.
   *
   * PARAMETERS:
   * const T constant - Constant to multiply matrix with
   * T (&matrix)[M][N] - Matrix of size MxN
   * const uint32_t row = M - Number of rows to multiply with constant (Default: All rows)
   * const uint32_t col = N - Number of columns to multiply with constant (Default: All columns)
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function computes the multiplication of a matrix with a constant.
   * If the number of rows and columns is not explicitly specified
   * all elements of the matrix will be multiplied with the constant. 
   *
   * PRECONDITIONS:
   * - The number of rows to multiply the constant with should be less than or equal to M.
   * - The number of columns to multiply the constant with should be less than or equal to N.
   *
   \*===========================================================================*/
   template<typename T, std::size_t N, std::size_t M>
   void F360_Multiply_2D_Matrix_With_Constant(
      const T constant,
      T (&matrix)[M][N],
      const uint32_t row = M,
      const uint32_t col = N)
   {
      assert(row <= M);
      assert(col <= N);

      for (uint32_t row_idx = 0U; row_idx < row; row_idx++)
      {
         for (uint32_t col_idx = 0U; col_idx < col; col_idx++)
         {
            matrix[row_idx][col_idx] *= constant;
         }
      }
   }

   void F360_Translate_2D_Position(
      const float32_t prev_x,
      const float32_t prev_y,
      const float32_t diff_x,
      const float32_t diff_y,
      float32_t & next_x,
      float32_t & next_y);

   void F360_Rotate_2D_Vector(
      const float32_t prev_x,
      const float32_t prev_y,
      const float32_t cos_angle,
      const float32_t sin_angle,
      float32_t & next_x,
      float32_t & next_y);

   float32_t F360_Bisquare_Weight(
      const float32_t residual_squared, 
      const float32_t tuning_constant_squared);

   float32_t F360_Huber_Weight(
      const float32_t residual,
      const float32_t tuning_constant);

   float32_t Compute_Dist_From_Point_To_Line_Squared(
      const float32_t x,
      const float32_t y,
      const float32_t k,
      const float32_t m);

   void Compute_Projection_Of_Point_On_Line(
      const float32_t x,
      const float32_t y,
      const float32_t k,
      const float32_t m,
      float32_t & proj_x,
      float32_t & proj_y);

   float32_t F360_Cross_Product(
      const float32_t x1,
      const float32_t y1,
      const float32_t x2,
      const float32_t y2);

   void Get_Vector_As_Linear_Equation(
      const float32_t start_point_x,
      const float32_t start_point_y,
      const float32_t end_point_x,
      const float32_t end_point_y,
      float32_t& k,
      float32_t& m
   );

   bool Find_X_Intersect_Between_Two_Lines(
      const float32_t k1,
      const float32_t m1,
      const float32_t k2,
      const float32_t m2,
      float32_t& x_int
   );

   bool Find_X_Intersect_Between_Line_And_2_Deg_Poly(
      const float32_t p2,
      const float32_t p1,
      const float32_t p0,
      const float32_t k,
      const float32_t m,
      float32_t& x1,
      float32_t& x2
   );

   /*===========================================================================*\
   * FUNCTION: F360_Sign()
   *===========================================================================
   * RETURN VALUE:
   * int sign
   *
   * PARAMETERS:
   * const T val - Value to find sign for.
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function finds whether the input value is positive, negetive or zero
   *
   \*===========================================================================*/
   template <typename T>
   int32_t F360_Sign(const T val)
   {
      const T zero = {};
      int32_t ret_val;
      if (val > zero)
      {
         ret_val = 1;
      }
      else if (val < zero)
      {
         ret_val = -1;
      }
      else
      {
         ret_val = 0;
      }
      return ret_val;
   }

   /*===========================================================================*\
   * FUNCTION: F360_matmul_2x2_2x1()
   *===========================================================================
   * RETURN VALUE:
   * none
   *
   * PARAMETERS:
   * const float32_t (&mat1)[2][2]
   * const float32_t (&mat2)[2][1]
   * float32_t (&result)[2][1]
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
   * This function find the matrix multiplication of 2x2 * 2x1
   *
   *
   \*===========================================================================*/
   inline void F360_matmul_2x2_2x1(
      const float32_t (&mat1)[2][2],
      const float32_t (&mat2)[2][1],
      float32_t (&result)[2][1])
   {
      result[0][0] = (mat1[0][0] * mat2[0][0]) + (mat1[0][1] * mat2[1][0]);
      result[1][0] = (mat1[1][0] * mat2[0][0]) + (mat1[1][1] * mat2[1][0]);
   }

    /*===========================================================================*\
    * FUNCTION: F360_Mod()
    *===========================================================================
    * RETURN VALUE:
    * float32_t: Returns floating point modulus(a,b)
    *
    * PARAMETERS:
    * const float32_t a
    * const float32_t b
    *
    * --------------------------------------------------------------------------
    * ABSTRACT:
    * --------------------------------------------------------------------------
    * This function computes the floating point modulus (as defined by the floor
    * function modulus(a,b) = (a/b - F360_Floorf(a/b))*b which means that the output
    * always take the same sign as the divisor b).
    *
    * If input divisor is close to 0 then function outputs its first input a with the sign of the second input b.
    *
    \*===========================================================================*/
   inline float32_t F360_Mod(const float32_t a, const float32_t b)
   {
      float32_t r;
      if (F360_MIN_DENOMINATOR > std::abs(b))
      {
         // To avoid division with zero
         r = F360_Copysignf(a, b);
      }
      else if (std::abs(b - F360_Floorf(b)) < F360_EPSILON) {
         r = a - F360_Floorf(a / b) * b;
      }
      else {
         r = a / b;
         r = (r - F360_Floorf(r)) * b;
      }

      return r;
   }

   /*===========================================================================*\
   * FUNCTION: Is_In_Bounds()
   *===========================================================================
   * RETURN VALUE:
   * bool f_in_bounds - flag indicating whether value is inside boundaries
   *
   * PARAMETERS:
   * const T val - tested value
   * const T lower_limit - minimal value
   * const T upper_limit - maximum value
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function verifies whether value is inside boundaries (lower limit
   * inclusive and upper limit exclusive)
   *
   \*===========================================================================*/
   template<typename T>
   bool Is_In_Bounds(
      const T val,
      const T lower_limit,
      const T upper_limit)
   {
      const bool f_in_bounds = (lower_limit <= val) && (val < upper_limit);

      return f_in_bounds;
   }

   /*===========================================================================*\
   * FUNCTION: Clamp()
   *===========================================================================
   * RETURN VALUE:
   * T clamped_val - updated value of clamped variable
   *
   * PARAMETERS:
   * const T val - value that should be clamped
   * const T bottom_limit - minimal value
   * const T upper_limit - maximum value
   *
   * --------------------------------------------------------------------------
   * ABSTRACT:
   * --------------------------------------------------------------------------
   * This function clamps value into boundaries defined by minimal and maximal value
   *
   \*===========================================================================*/
   template<typename T>
   T Clamp(
      const T val,
      const T bottom_limit,
      const T upper_limit)
   {
      T clamped_val = std::max(val, bottom_limit);
      clamped_val = std::min(clamped_val, upper_limit);
      return clamped_val;
   }
}
#endif
