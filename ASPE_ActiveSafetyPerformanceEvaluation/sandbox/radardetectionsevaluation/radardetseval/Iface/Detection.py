from dataclasses import dataclass


@dataclass
class Detection:
    """
    Definition of detection structure
    """
    det_range: float
    det_azimuth: float
    det_range_rate: float
    det_range_var: float = 0.0
    det_azimuth_var: float = 0.0
    det_range_rate_var: float = 0.0
