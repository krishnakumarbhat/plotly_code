#ifndef ROT_CALS_H
#define ROT_CALS_H

/*===========================================================================*\
 * File: ROT_cals.h
 *===========================================================================
 * Copyright 2023 Aptiv, Inc., All Rights Reserved.
 * Aptiv Confidential
 *---------------------------------------------------------------------------
 * Created on: October 10, 2023
 * Author: Dikshant Patel
 *---------------------------------------------------------------------------
 *
 * Description:
 *    Calibration and project specific values
 *
\*===========================================================================*/

// Radar cals
#define MRR_ELEVATION_MISALIGNMENT_ANGLE (0.0f)
#define MRR3_MIN_ALIASED_RANGE_RATE      (-23.0F)

// Vehicle cals
#define DIST_FRONT_2_REAR_AXLE    (3.555f) /* V40 */
#define WHEELBASE                 (2.65f)
#define VEHICLE_WIDTH             (1.9f)
#define VEHICLE_LENGTH            (4.37f)
#define REAR_CORNERING_COMPLIANCE (0.0053f)

// Platform specific and build cals
#ifndef SAE_OUTPUT
   #define SAE_OUTPUT (1U)
#endif
#ifndef ISO_OUTPUT
   #define ISO_OUTPUT (0U)
#endif
#define SINGLE_SENSOR_CONFIG (0U)
#define DUAL_SENSOR_CONFIG   (1U)
#define HIL_ENABLED          (0x5555)
#define F360_HIL_STUB        (0)
#ifndef F360_DEBUG
   #define F360_DEBUG 1
#endif
#endif // ROT_CALS_H
