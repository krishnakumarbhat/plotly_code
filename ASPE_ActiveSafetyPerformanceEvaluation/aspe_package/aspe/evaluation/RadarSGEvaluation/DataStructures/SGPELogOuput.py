from aspe.evaluation.RadarSGEvaluation.DataStructures.SGKPIOutput import SGKPIOutput
from aspe.evaluation.RadarSGEvaluation.DataStructures.SGLogMetadata import SGLogMetadata


class SGPELogOutput:
    """
    Class which stores various whole log output.
    """

    def __init__(self, log_metadata: SGLogMetadata, kpi_output: SGKPIOutput):
        self.log_metadata = log_metadata
        self.kpi_output = kpi_output
