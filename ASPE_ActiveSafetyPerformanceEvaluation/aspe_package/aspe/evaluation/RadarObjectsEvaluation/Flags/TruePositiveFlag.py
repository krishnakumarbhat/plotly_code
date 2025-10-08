from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification import BCType
from aspe.evaluation.RadarObjectsEvaluation.Flags import SignalEqualityFlag


class TruePositiveFlag(SignalEqualityFlag):
    def __init__(self, flag_signature='TP'):
        super().__init__(signal_name='binary_classification',
                         signal_expected_value=BCType.TruePositive,
                         flag_signature=flag_signature)
