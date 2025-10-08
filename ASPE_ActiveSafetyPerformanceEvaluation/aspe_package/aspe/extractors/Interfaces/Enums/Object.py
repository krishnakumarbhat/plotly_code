from enum import Enum


class MovementStatus(Enum):
    UNDEFINED = 0
    STATIONARY = 1
    STOPPED = 2
    MOVING = 3


class ObjectClass(Enum):
    UNDEFINED = 0
    PEDESTRIAN = 1
    TWOWHEEL = 2
    CAR = 3
    TRUCK = 4
    UNKNOWN = 5


class MeasurementStatus(Enum):
    INVALID = 0
    MEASURED = 1
    NEW = 2
    PREDICTED = 3
    PARTLY_MEASURED = 4
