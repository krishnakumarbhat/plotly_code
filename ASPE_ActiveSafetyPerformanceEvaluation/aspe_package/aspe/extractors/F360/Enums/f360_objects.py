# coding=utf-8
"""
F360 Objects Enums (comes from C code)
"""
from enum import Enum


class F360ObjectStatus(Enum):
    """
    F360 Objects status enum
    """
    INVALID = 0
    NEW = 1
    NEW_COASTED = 2
    NEW_UPDATED = 3
    UPDATED = 4
    COASTED = 5


class F360ObjectsFilterType(Enum):
    """
    F360 Objects filter type enum
    """
    INVALID = 0
    CCV = 1
    CTCA = 2
    CCA = 3
    

class F360ObjectClass(Enum):
    """
    F360 Objects classification enum
    """
    UNDETERMINED = 0
    CAR = 1
    MOTORCYCLE = 2
    TRUCK = 3
    PEDESTRIAN = 4
    POLE = 5
    TREE = 6
    ANIMAL = 7
    GOD = 8
    BICYCLE = 9
    UNIDENTIFIED_VEHICLE = 10

