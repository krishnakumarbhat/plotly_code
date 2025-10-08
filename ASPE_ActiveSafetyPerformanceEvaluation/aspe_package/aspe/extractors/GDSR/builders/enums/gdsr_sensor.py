from enum import Enum


class GdsrMountingLocationState(Enum):
    # Mounting position invalid
    MOUNTING_POSITION_NOT_SET = 0
    # Mounting position has been set
    MOUNTING_POSITION_SET = 128


class GdsrLongitudinalMountingLocation(Enum):
    MOUNTING_LONGITUDINAL_NOT_SET = 0
    MOUNTING_FORWARD = 8
    MOUNTING_SIDE = 16
    MOUNTING_REAR = 24


class GdsrLateralMountingLocation(Enum):
    MOUNTING_LATERAL_NOT_SET = 0
    MOUNTING_LEFT = 1
    MOUNTING_CENTER = 2
    MOUNTING_RIGHT = 3


class GdsrMountingLocation(Enum):
    MOUNTING_NOT_SET = GdsrMountingLocationState.MOUNTING_POSITION_NOT_SET.value
    MOUNTING_LEFT_FORWARD = (
        GdsrLateralMountingLocation.MOUNTING_LEFT.value
        + GdsrLongitudinalMountingLocation.MOUNTING_FORWARD.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_LEFT_SIDE = (
        GdsrLateralMountingLocation.MOUNTING_LEFT.value
        + GdsrLongitudinalMountingLocation.MOUNTING_SIDE.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_LEFT_REAR = (
        GdsrLateralMountingLocation.MOUNTING_LEFT.value
        + GdsrLongitudinalMountingLocation.MOUNTING_REAR.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_CENTER_FORWARD = (
        GdsrLateralMountingLocation.MOUNTING_CENTER.value
        + GdsrLongitudinalMountingLocation.MOUNTING_FORWARD.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_CENTER_REAR = (
        GdsrLateralMountingLocation.MOUNTING_CENTER.value
        + GdsrLongitudinalMountingLocation.MOUNTING_REAR.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_RIGHT_FORWARD = (
        GdsrLateralMountingLocation.MOUNTING_RIGHT.value
        + GdsrLongitudinalMountingLocation.MOUNTING_FORWARD.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_RIGHT_SIDE = (
        GdsrLateralMountingLocation.MOUNTING_RIGHT.value
        + GdsrLongitudinalMountingLocation.MOUNTING_SIDE.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )
    MOUNTING_RIGHT_REAR = (
        GdsrLateralMountingLocation.MOUNTING_RIGHT.value
        + GdsrLongitudinalMountingLocation.MOUNTING_REAR.value
        + GdsrMountingLocationState.MOUNTING_POSITION_SET.value
    )


class GdsrRadarSensorType(Enum):
    UNKNOWN = 0
