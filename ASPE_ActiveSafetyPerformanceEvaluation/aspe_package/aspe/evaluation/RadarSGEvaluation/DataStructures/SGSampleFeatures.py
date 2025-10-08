import pandas as pd

from aspe.evaluation.RadarSGEvaluation.DataStructures import EvaluationSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class SGSampleFeatures(IDataSet):
    """
    Class which stores sample features of samples for example:
    cartesian distance ref/est, position deviation ref/est.
    """

    def __init__(self) -> None:
        super().__init__()

        signal_names = [EvaluationSignals.distance,
                        EvaluationSignals.deviation_x,
                        EvaluationSignals.deviation_y,
                        EvaluationSignals.binary_classification,
                        EvaluationSignals.segmentation_type]
        self.update_signals_definition(signal_names)

    def insert_feature(self, df: pd.DataFrame) -> None:
        """
        Fill internal dataframe with calculated feature dataframe
        Verify if passed dataframe contains all required features.

        :raises:
            AttributeError: When there is an invalid column inside dataframe
        """
        required_columns = list(self.signals.columns)

        if not set(required_columns).issubset(df.columns):
            raise AttributeError("Missing columns inside SGSampleFeatures dataframe!")

        self.signals.loc[:, required_columns] = df.reset_index().loc[:, required_columns]

    def get_base_name(self) -> str:
        return 'SGSampleFeatures'
