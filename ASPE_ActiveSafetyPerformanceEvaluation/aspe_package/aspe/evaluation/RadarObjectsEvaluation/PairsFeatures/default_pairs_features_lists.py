from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.distance_metrics.CartesianDistance2D import (
    CartesianDistance2D,
)

from .deviation import *
from .deviation.deviation_cr2cr import PositionDeviationCR2CR, VelocityDeviationCRC2R
from .intersection_over_union_ratio import *
from .nees import *

DEFAULT_PAIRS_FEATURES_LIST = [
    PositionDeviationR2R(),
    SpeedDeviation(),
    OrientationDeviation(),
    VelocityDeviationR2R(),
    VelocityRelativeDeviationR2R(),
    AccelerationDeviationR2R(),
    AccelerationRelativeDeviationR2R(),
    DimensionsDeviation(),
    YawRateDeviation(),
    IntersectionOverUnionRatio(),
]

DEF_NO_ACC_REL_PAIRS_FEATURES_LIST = [
    PositionDeviationR2R(),
    SpeedDeviation(),
    OrientationDeviation(),
    VelocityDeviationR2R(),
    VelocityRelativeDeviationR2R(),
    AccelerationDeviationR2R(),
    DimensionsDeviation(),
    YawRateDeviation(),
]

EXTENDED_PAIRS_FEATURES_LIST = [
    *DEFAULT_PAIRS_FEATURES_LIST,
    PositionNeesValuesR2R(),
    VelocityNeesValuesR2R(),
]

EXT_NO_ACC_REL_PAIRS_FEATURES_LIST = [
    *DEF_NO_ACC_REL_PAIRS_FEATURES_LIST,
    PositionNeesValuesR2R(),
    VelocityNeesValuesR2R(),
]

C2C_DEVIATION_METHOD_LIST = [
    CartesianDistance2D(),
    PositionDeviationC2C(),
]
CR2CR_DEVIATION_METHOD_LIST = [
    CartesianDistance2D(),
    PositionDeviationCR2CR(),
    VelocityDeviationCRC2R(),
]
R2R_DEVIATION_METHOD_LIST = [
    CartesianDistance2D(),
    PositionDeviationR2R(),
    VelocityDeviationR2R(),
]
