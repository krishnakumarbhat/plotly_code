import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures import EvaluationSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class SGSegments(IDataSet):
    """
    Class which stores "segments" - a part of contour (part of line between two vertices)
    Each row contains information about one segment, either reference or estimated one.
    """

    def __init__(self) -> None:
        super().__init__()
        signal_names = [EvaluationSignals.start_position_x,
                        EvaluationSignals.start_position_y,
                        EvaluationSignals.end_position_x,
                        EvaluationSignals.end_position_y,
                        EvaluationSignals.contour_unique_id]
        self.update_signals_definition(signal_names)

    def insert_segments(self, df: pd.DataFrame) -> None:
        """
        Fill internal dataframe with calculated segments dataframe
        Verify if passed dataframe has all required columns

        :raises:
            AttributeError: When there are missing columns inside dataframe
        """
        required_columns = list(self.signals.columns)
        if not set(required_columns).issubset(df.columns):
            raise AttributeError("Missing columns inside SGSegments dataframe!")
        self.signals.loc[:, required_columns] = df.loc[:, required_columns]

    def get_base_name(self) -> str:
        return 'SGSegments'
