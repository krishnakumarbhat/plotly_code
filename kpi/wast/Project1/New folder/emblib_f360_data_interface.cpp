/*===========================================================================*\
 * File: f360_data_interface.c
 *===========================================================================
 * Copyright 2018 Aptiv, Inc., All Rights Reserved.
 * Aptiv Confidential
 *---------------------------------------------------------------------------
 * Created on: October 10, 2023
 * Author: Dikshant Patel
 *---------------------------------------------------------------------------
 *
 * Description:
 *    Interface between .cpp and .c files required to populate F360 inputs and
 *    outputs
 *
\*===========================================================================*/

/*===========================================================================*\
 * Standard Header Includes
\*===========================================================================*/

/*===========================================================================*\
 * Project Header Includes
\*===========================================================================*/
#include "emblib_f360_data_interface.h"
// #include "emblib_T360_Types.h"
// #include "helper_modules/ti/timing_helpers.h"
#include "reuse.h"
// #include "smc_cal.h"

/*===========================================================================*\
 * Local Preprocessor Constants
\*===========================================================================*/
#define DEG2RAD (0.017453f)
#ifndef S_2_US
   #define S_2_US (1e+6f)
#endif

#ifndef US_2_S
   #define US_2_S (1e-6f)
#endif

/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/

/*===========================================================================*\
 * Function Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Function: Get_f360_prndl_state
 *===========================================================================
 * Return Value:
 *  - void
 *
 * Parameters:
 *  - INPUT_SSTS_GEAR_POSITION gear_position
 *
 * External References:
 *  - none
 *
 * Description:
 *    Maps ISO gear position to F360 PRNDL.
 *
\*===========================================================================*/
F360_PRNDL_STATE Get_f360_prndl_state(int gear_position)
{
   F360_PRNDL_STATE prndl = F360_PRNDL_STATE_DRIVE;

   switch (gear_position)
   {
      case 1: // park
         prndl = F360_PRNDL_STATE_PARK;
         break;
      case 3: // drive
      default:
         prndl = F360_PRNDL_STATE_DRIVE;
         break;
      case 2: // reverse
         prndl = F360_PRNDL_STATE_REVERSE;
         break;
      case 4: // Neutral
         prndl = F360_PRNDL_STATE_NEUTRAL;
         break;
      case 5:
         prndl = F360_PRNDL_STATE_LOW;
         break;
   }

   return prndl;
}

float32_t timing_helpers_get_time_usec()
{
   return 0;
}
/*===========================================================================*\
 * Function: Get_System_Time
 *===========================================================================
 * Return Value:
 *  - uint32_t
 *
 * Parameters:
 *  - void
 *
 * External References:
 *  - none
 *
 * Description:
 *    Returns the current system time in microseconds
 *
\*===========================================================================*/
uint32_t Get_System_Time(void)
{
   return timing_helpers_get_time_usec();
}
