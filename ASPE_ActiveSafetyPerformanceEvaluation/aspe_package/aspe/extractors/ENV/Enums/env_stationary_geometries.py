"""
ENV Stationary Geometries Enums
"""
from enum import Enum


class SGContourType(Enum):
    INVALID = 0
    POINT = 1
    POLYLINE = 2
    POLYGON = 3


class SGDrivabilityClass(Enum):
    UNKNOWN = 0
    NON_DRIVABLE = 1
    UNDERDRIVABLE = 2
    OVERDRIVABLE = 3
