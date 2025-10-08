from enum import Enum


class ENVMountingLocationState(Enum):
    # Mounting position invalid
    MOUNTING_POSITION_NOT_SET = 0
    # Mounting position has been set
    MOUNTING_POSITION_SET = 128


class ENVLongitudinalMountingLocation(Enum):
    MOUNTING_LONGITUDINAL_NOT_SET = 0
    MOUNTING_FORWARD = 8
    MOUNTING_SIDE = 16
    MOUNTING_REAR = 24


class ENVLateralMountingLocation(Enum):
    MOUNTING_LATERAL_NOT_SET = 0
    MOUNTING_LEFT = 1
    MOUNTING_CENTER = 2
    MOUNTING_RIGHT = 3


class ENVMountingLocation(Enum):
    MOUNTING_NOT_SET = ENVMountingLocationState.MOUNTING_POSITION_NOT_SET.value
    MOUNTING_LEFT_FORWARD = (
        ENVLateralMountingLocation.MOUNTING_LEFT.value
        + ENVLongitudinalMountingLocation.MOUNTING_FORWARD.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_LEFT_SIDE = (
        ENVLateralMountingLocation.MOUNTING_LEFT.value
        + ENVLongitudinalMountingLocation.MOUNTING_SIDE.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_LEFT_REAR = (
        ENVLateralMountingLocation.MOUNTING_LEFT.value
        + ENVLongitudinalMountingLocation.MOUNTING_REAR.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_CENTER_FORWARD = (
        ENVLateralMountingLocation.MOUNTING_CENTER.value
        + ENVLongitudinalMountingLocation.MOUNTING_FORWARD.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_CENTER_REAR = (
        ENVLateralMountingLocation.MOUNTING_CENTER.value
        + ENVLongitudinalMountingLocation.MOUNTING_REAR.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_RIGHT_FORWARD = (
        ENVLateralMountingLocation.MOUNTING_RIGHT.value
        + ENVLongitudinalMountingLocation.MOUNTING_FORWARD.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_RIGHT_SIDE = (
        ENVLateralMountingLocation.MOUNTING_RIGHT.value
        + ENVLongitudinalMountingLocation.MOUNTING_SIDE.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_RIGHT_REAR = (
        ENVLateralMountingLocation.MOUNTING_RIGHT.value
        + ENVLongitudinalMountingLocation.MOUNTING_REAR.value
        + ENVMountingLocationState.MOUNTING_POSITION_SET.value
    )


class ENVRadarSensorType(Enum):
    UNKNOWN = 0
