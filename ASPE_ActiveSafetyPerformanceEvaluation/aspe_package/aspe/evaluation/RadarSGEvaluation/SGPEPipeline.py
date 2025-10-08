from aspe.evaluation.RadarSGEvaluation.Builders.SGSampleFeatures import get_sample_features
from aspe.evaluation.RadarSGEvaluation.Builders.SGSamples import associate_samples
from aspe.evaluation.RadarSGEvaluation.Builders.SGSegmentPairs import build_pairs
from aspe.evaluation.RadarSGEvaluation.Builders.SGSegments import build_segments
from aspe.evaluation.RadarSGEvaluation.Enums.BinaryClassificationType import BCType
from aspe.evaluation.RadarSGEvaluation.Enums.SegmentationType import SegmentationType
from aspe.evaluation.RadarSGEvaluation.KPI.KPIManager import calculate_kpis
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData


class SGPEPipeline:
    """
    Class for SG KPI evaluation.
    """

    def __init__(self,
                 relevancy_filter_estimated=None,
                 relevancy_filter_reference=None,
                 ):
        """
        Pipeline configuration happens here.

        :param relevancy_filter_estimated: object instance of the estimated relevancy filters
        :type relevancy_filter_estimated: RelevancyInterface
        :param relevancy_filter_reference: object instance of the reference relevancy filters
        :type relevancy_filter_reference: RelevancyInterface
        """

        self.relevancy_filter_estimated = relevancy_filter_estimated
        self.relevancy_filter_reference = relevancy_filter_reference

        self.segments_est = None
        self.segments_ref = None
        self.filtered_segments_est = None
        self.filtered_segments_ref = None
        self.segment_pairs_est = None
        self.segment_pairs_ref = None
        self.samples_est = None
        self.samples_ref = None
        self.sample_features_est = None
        self.sample_features_ref = None
        self.kpi_output = None
        self.log_output_metadata = None

    def evaluate(self, estimated_data: ENVExtractedData, reference_data: ENVExtractedData) -> None:
        """
        This is a single log pipeline for Stationary Geometries evaluation.

        :param estimated_data: estimated Stationary Geometries data
        :type estimated_data: ENVExtractedData
        :param reference_data: reference Stationary Geometries data
        :type reference_data: ENVExtractedData
        """

        self._create_segments(estimated_data, reference_data)
        self._select_relevant_segments()
        self._create_paired_segments()
        self._associate_samples()
        self._calculate_sample_features()
        self._calculate_kpis()
        self._calculate_log_features()
        self._create_single_log_output()

    def _create_segments(self, estimated_data: ENVExtractedData, reference_data: ENVExtractedData):
        """
        Method takes extracted data and creates SGSegments and SGSegmentPairs 
        of estimated and reference data.

        :param estimated_data: estimated Stationary Geometries data
        :type estimated_data: ENVExtractedData
        :param reference_data: reference Stationary Geometries data
        :type reference_data: ENVExtractedData
        """

        self.segments_est = build_segments(estimated_data)
        self.segments_ref = build_segments(reference_data)

    def _select_relevant_segments(self):
        """
        Method takes SGSegmentPairs of estimated and reference data
        and returns only relevant records according to selected flags
        """
        pass

    def _create_paired_segments(self):
        """
        Method  takes extracted data and creates SGSegments and SGSegmentPairs 
        of estimated and reference data
        """
        self.segment_pairs_est, self.segment_pairs_ref = build_pairs(self.segments_est.signals,
                                                                     self.segments_ref.signals)

    def _associate_samples(self):
        """
        Method takes SGSegmentPairs of estimated and reference data
        and creates SGSamples for estimated and reference data.
        """
        self.samples_est = associate_samples(self.segments_est.signals, self.segments_ref.signals,
                                             self.segment_pairs_est.signals)
        self.samples_ref = associate_samples(self.segments_ref.signals, self.segments_est.signals,
                                             self.segment_pairs_ref.signals)

    def _calculate_sample_features(self):
        """
        Method takes SGSamples of estimated and reference data
        and creates SGSampleFeatures, which contains information
        about deviations and distance and remove samples associated multiple times.
        """
        self.sample_features_est = get_sample_features(self.samples_est.signals, BCType.FalsePositive,
                                                       SegmentationType.UnderSegmentation)
        self.sample_features_ref = get_sample_features(self.samples_ref.signals, BCType.FalseNegative,
                                                       SegmentationType.OverSegmentation)

    def _calculate_kpis(self):
        """
        Method takes SGSampleFeatures of estimated and reference data
        and creates SGKPIOutput, which contains information about calculated KPI metrics.
        """
        self.kpi_output = calculate_kpis(self.sample_features_est.signals, self.sample_features_ref.signals)

    def _calculate_log_features(self):
        """
        Method takes SGSampleFeatures of estimated and reference data and
        creates SGLogMetadata, which contains information
        about log parameters for calculated KPI metrics.
        """
        pass

    def _create_single_log_output(self):
        """
        Method takes SGLogMetadata and SGKPIOutput for evaluated log and calculates SGPELogOutput.
        """
        pass
