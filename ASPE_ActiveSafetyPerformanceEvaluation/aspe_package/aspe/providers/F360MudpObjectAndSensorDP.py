import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Flags import IsMovableFlag
from aspe.extractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from aspe.extractors.Transform.CrossDataSet.host2sensor import host2sensor_motion
from aspe.parsers.MudpParser import MudpParser
from aspe.parsers.MudpParser.f360_defaults_configs import streams_to_read, unknown_size_per_stream
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.support_functions import get_single_log_pe_input_paths


class F360MudpObjectAndSensorDP(IPerfEvalInputDataProvider):
    """
    Data provider to separate Object data and Sensor Data into two different DataSets
    """
    def __init__(self,
                 mudp_stream_def_path=None,
                 ref_relevancy_flag=IsMovableFlag(),
                 f_clean_object_cov=True):
        self._parser = MudpParser(streams_to_read, unknown_size_per_stream, mudp_stream_def_path)
        self._ref_extractor = F360MUDPExtractor(f_extract_objects=True,
                                                f_extract_internal_objects=True,
                                                f_extract_sensors=True,
                                                f_extract_host=True,
                                                f_extract_trailer=False,
                                                f_extract_detections=False)

        self._est_extractor = F360MUDPExtractor(f_extract_objects=False,
                                                f_extract_internal_objects=True,
                                                f_extract_sensors=False,
                                                f_extract_host=False,
                                                f_extract_trailer=False,
                                                f_extract_detections=True)

        self._ref_rel_flag = ref_relevancy_flag
        self._f_clean_object_cov = f_clean_object_cov

    def get_single_log_data(self, log_path):
        """
        Get single log data for given log path
        Note:
            Data is time synchronized but not scan index synchronized.
        :param log_path: path to log from which data should be provided. .mudp log will be automatically
                         found based on log name.
        :type log_path: str
        :return: tuple of (ExtractedData, ExtractedData),
                 First extracted data should contain estimated data sets:
                 -sensors
                 -detections
                 Second extracted data should contain reference data sets (mudp data based):
                 -objects
                 -host
        """
        estimated_data_path, _ = get_single_log_pe_input_paths(log_path, '.mudp', '')

        print('Parsing .mudp data ...')
        parsed_data = self._parser.parse(estimated_data_path)

        print('Extracting estimated data ...')
        estimated_data = self._est_extractor.extract_data(parsed_data)

        print('Extracting reference data ...')
        reference_data = self._ref_extractor.extract_data(parsed_data)

        print('Extraction Post-processing ...')
        flag = self._ref_rel_flag.calc_flag(reference_data.objects.signals)
        reference_data.objects.signals = reference_data.objects.signals[flag]
        reference_data.objects.raw_signals = reference_data.objects.raw_signals[flag]

        estimated_data, reference_data = self._post_process_data(estimated_data, reference_data)

        print('Data extracted.')
        return estimated_data, reference_data

    def _post_process_data(self, estimated_data, reference_data):
        """
        Adding missing data
        :param estimated_data:
        :param reference_data:
        :return: tuple of (ExtractedData, ExtractedData),
        """
        reference_data.host.signals['yaw_rate_variance'] = 0.0
        reference_data.host.signals['velocity_otg_variance_x'] = 0.0
        reference_data.host.signals['velocity_otg_variance_y'] = 0.0
        reference_data.host.signals['velocity_otg_covariance'] = 0.0

        if self._f_clean_object_cov:
            reference_data.objects.signals['position_variance_x'] = 0.0
            reference_data.objects.signals['position_variance_y'] = 0.0
            reference_data.objects.signals['position_covariance'] = 0.0
            reference_data.objects.signals['velocity_otg_variance_x'] = 0.0
            reference_data.objects.signals['velocity_otg_variance_y'] = 0.0
            reference_data.objects.signals['velocity_otg_covariance'] = 0.0

        # Note: sensor velocity is overwritten
        host2sensor_motion(reference_data.sensors, reference_data.host, inplace=True)

        reference_data.sensors.per_look['min_azimuth'] = np.deg2rad(-75.0)
        reference_data.sensors.per_look['max_azimuth'] = np.deg2rad(75.0)
        reference_data.sensors.per_look['min_elevation'] = np.deg2rad(-5.0)
        reference_data.sensors.per_look['max_elevation'] = np.deg2rad(5.0)

        return estimated_data, reference_data
