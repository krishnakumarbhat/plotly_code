# coding=utf-8
"""
F360 Radars Enums (comes from C code)
"""
from enum import Enum


class F360RadarSensorMountingLocation(Enum):
    """
    F360 Mounting location for radar sensors. This is a mirror of F360_Mounting_Location.h in the radar object tracker
    repository.
    """
    UNKNOWN = -1
    LEFT_FORWARD = 0
    LEFT_SIDE1 = 8
    LEFT_SIDE2 = 16
    LEFT_REAR = 24
    CENTER_FORWARD = 1
    CENTER_REAR = 25
    RIGHT_FORWARD = 2
    RIGHT_SIDE1 = 10
    RIGHT_SIDE2 = 18
    RIGHT_REAR = 26
    CENTER2_FORWARD = 3
    CENTER2_REAR = 27
    CENTER3_FORWARD = 4
    CENTER3_REAR = 28

class F360RadarSensorType(Enum):
    """
    F360 Types of Radar sensors. This is a mirror of f360_sensor_type.h in the radar object tracker repository.
    """
    UNKNOWN = -1
    SRR2_RADAR = 0
    SRR4_RADAR = 1
    SRR4_MM_RADAR = 2
    ESR_RADAR = 3
    MRR1_RADAR = 4
    MRR2_RADAR = 5
    MRR3_RADAR = 6
    LIDAR = 7
    VISION = 8
    VEHICLE = 9
    SRR5_RADAR = 10
    MRR360_RADAR = 11
    FLR4_RADAR = 12
    FLR4_PLUS_RADAR = 13
    SRR6_RADAR = 14
    SRR6_PLUS_RADAR = 15
    SRR7_PLUS_RADAR = 16
    FLR7_RADAR = 17
    SRR6_PLUS_PLT_RADAR = 18
    SRR6_PLUS_PLT_1XCAN_RADAR = 19
    SRR6_PLUS_PLT_1XETH_RADAR = 20
    SRR6_PLUS_PLT_1GB_EHT_RADAR = 21
    SRR7_PLUS_PLT_RADAR = 22
    FLR7_PLT_RADAR = 23
    FLR4_PLUS_PLT_RADAR = 24


