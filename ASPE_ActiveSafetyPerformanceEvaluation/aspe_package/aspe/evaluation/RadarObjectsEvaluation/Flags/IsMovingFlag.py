import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags import IFlag, SignalEqualityFlag
from aspe.extractors.Interfaces.Enums.Object import MovementStatus


class IsMovingFlag(IFlag):
    def __init__(self, flag_signature='is_moving', *args, **kwargs):
        super().__init__(flag_signature, *args, **kwargs)
        self.is_moving_flag = SignalEqualityFlag(signal_name='movement_status',
                                                 signal_expected_value=MovementStatus.MOVING)

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        return self.is_moving_flag.calc_flag(data_frame)
