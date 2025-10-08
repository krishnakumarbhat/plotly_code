from dataclasses import dataclass

import pandas as pd


@dataclass
class SignalDescription:
    signature: str
    dtype: type
    unit: str = 'None'
    description: str = 'None'

    def to_series(self):
        return pd.Series(self.__dict__)

    def __hash__(self):
        return self.signature.__hash__()
