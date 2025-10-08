"""
ENV Occupancy Grid Enums
"""
from enum import Enum


class OCGUnderdrivabilityStatus(Enum):
    CAN_NOT_PASS_UNDER = 0
    IS_LIKELY_TO_PASS_UNDER = 1
    CAN_PASS_UNDER = 2
    NOT_TO_CONSIDER = 3

