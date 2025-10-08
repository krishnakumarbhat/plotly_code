from typing import Iterable

import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.Flags.IFlag import IFlag


class LogicalAndFlag(IFlag):
    def __init__(self, flags=Iterable[IFlag], flag_signature='logical_and', *args, **kwargs):
        flag_signatures = [f.flag_signature for f in flags]
        super().__init__('_and_'.join(flag_signatures), *args, **kwargs)
        self.flags = flags

    def calc_flag(self, data_frame: pd.DataFrame, *args, **kwargs) -> pd.Series:
        output = pd.Series(np.full(len(data_frame), True), index=data_frame.index)
        for flag in self.flags:
            output = output & flag.calc_flag(data_frame)
        return output

    def __repr__(self):
        name = ''
        for f in self.flags:
            name = f'{name} & {f.__repr__()}'
        return f'({name.lstrip(" &")})'
