import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures import EvaluationSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class SGSegmentPairs(IDataSet):
    """
    Class stores information about paired reference and estimated segments.

    Each row represents one estimated and one reference segments, which have been paired.
    """

    def __init__(self) -> None:
        super().__init__()
        signal_names = [EvaluationSignals.segment_unique_id,
                        EvaluationSignals.segment_unique_id_paired,
                        EvaluationSignals.projection_start_position_x,
                        EvaluationSignals.projection_start_position_y,
                        EvaluationSignals.unit_vector_x_paired,
                        EvaluationSignals.unit_vector_y_paired,
                        EvaluationSignals.dot_product]
        self.update_signals_definition(signal_names)

    def insert_pairs(self, df: pd.DataFrame) -> None:
        """
        Fill internal dataframe with calculated segment pairs dataframe
        Verify if passed dataframe has all required columns

        :raises:
            AttributeError: When there are missing columns inside dataframe
        """
        required_columns = list(self.signals.columns)

        if not set(required_columns).issubset(df.columns):
            raise AttributeError("Missing columns inside SGSegmentPairs dataframe!")
        self.signals.loc[:, required_columns] = df.reset_index().loc[:, required_columns]
        self.update_signals_dtypes()

    def get_base_name(self) -> str:
        return 'SGSegmentPairs'
