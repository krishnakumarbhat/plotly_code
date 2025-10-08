from aspe.evaluation.RadarObjectsEvaluation.DataStructures import EvaluationSignals
from aspe.extractors.Interfaces.IDataSet import IDataSet


class PEPairedObjects(IDataSet):
    '''
    Class which stores tracker objects evaluation results for associated tracks. Each row contains information about
    associated objects pair - reference object to estimated objects - signals and deviations between them.
    '''

    def __init__(self):
        super().__init__()
        signal_names = [EvaluationSignals.index_ref,
                        EvaluationSignals.index_est,

                        EvaluationSignals.unique_id_ref,
                        EvaluationSignals.unique_id_est,

                        EvaluationSignals.relevancy_flag_ref,
                        EvaluationSignals.relevancy_flag_est,

                        EvaluationSignals.is_associated,

                        EvaluationSignals.binary_classification,
                        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'PEObjectPairs'
