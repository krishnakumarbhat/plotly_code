import numpy as np

from aspe.evaluation.RadarDetectionEvaluation.Features.IdealRRBasedAzimuthErrorFeatures import (
    IdealRRBasedAzimuthErrorFeatures,
)
from aspe.evaluation.RadarDetectionEvaluation.Features.Nees3dFeatures import Ness3dFeatures

DEFAULT_DET_EVAL_PAIRS_FEATURES_LIST = [Ness3dFeatures(),
                                        IdealRRBasedAzimuthErrorFeatures(target_yaw_rate_to_ignore=np.deg2rad(5.0))]
