import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures import EvaluationSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class SGSamples(IDataSet):
    """
    Class which stores "samples" - points on (x,y) plane which are projected from segments
    Each row contains information about samples which belongs to one segment pair.
    """

    def __init__(self) -> None:
        super().__init__()
        signal_names = [EvaluationSignals.segment_unique_id,
                        EvaluationSignals.sample_position_x,
                        EvaluationSignals.sample_position_y,
                        EvaluationSignals.projection_sample_position_x,
                        EvaluationSignals.projection_sample_position_y,
                        EvaluationSignals.distance,
                        EvaluationSignals.deviation_x,
                        EvaluationSignals.deviation_y,
                        EvaluationSignals.classification,
                        EvaluationSignals.multiple_segmentation]
        self.update_signals_definition(signal_names)

    def insert_samples(self, calculated_samples: pd.DataFrame) -> None:
        """
        Fill internal dataframe with calculated samples dataframe
        Verify if passed dataframe has all required columns

        :raises:
            AttributeError: When there are missing columns inside dataframe
        """
        required_columns = list(self.signals.columns)

        if not set(required_columns).issubset(calculated_samples.columns):
            raise AttributeError("Missing columns inside SGSamples dataframe!")
        self.signals.loc[:, required_columns] = calculated_samples.reset_index().loc[:, required_columns]

    def get_base_name(self) -> str:
        return 'SGSamples'
