from aspe.evaluation.RadarObjectsEvaluation.DataStructures import EvaluationSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class PESingleObjects(IDataSet):
    '''
    Class which stores tracker objects evaluation results for reference or estimated data. Each row contains single
    object data within single time instance.
    '''

    def __init__(self):
        super().__init__()
        signal_names = [
            EvaluationSignals.relevancy_flag,
            EvaluationSignals.is_associated,
            EvaluationSignals.binary_classification,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'PESingleObjects'
