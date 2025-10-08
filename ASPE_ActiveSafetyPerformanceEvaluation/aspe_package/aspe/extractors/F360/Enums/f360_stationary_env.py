from enum import Enum


class StationaryEnvType(Enum):
    INVALID = 0,
    HOST_CURV_BASED = 1  # Host curvature based guardrail estimate
    LSC = 2  # Longitudinal static curves
    CWD = 3  # Concrete wall detector
