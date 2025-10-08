import numpy as np
from AptivDataExtractors.F360.Mudp.F360MUDPExtractor import F360MUDPExtractor
from AptivDataParser.MudpParser import MudpHandler
from AptivPerformanceEvaluation.DataProviders.support_functions import get_single_log_pe_input_paths
from AptivPerformanceEvaluation.RadarObjectsEvaluation.Flags import IsMovableFlag
from radardetseval.configs.default_radar import DefaultRadar

from radardetseval.configs.defaut_reference import DefaultReference

class F360MudpObjectAndSensorDP:
    def __init__(self, mudp_parser_config_path, mudp_stream_def_path=None, ref_relevancy_flag=IsMovableFlag()):
        self._ref_extractor = F360MUDPExtractor(f_extract_objects = True,
                                                f_extract_internal_objects = False,
                                                f_extract_sensors=False,
                                                f_extract_host=True,
                                                f_extract_detections=False)

        self._est_extractor = F360MUDPExtractor(f_extract_objects=False,
                                                f_extract_internal_objects=False,
                                                f_extract_sensors=True,
                                                f_extract_host=False,
                                                f_extract_detections=True)

        self._mudp_parser_config_path = mudp_parser_config_path
        self._mudp_stream_def_path = mudp_stream_def_path

        self._ref_rel_falg = ref_relevancy_flag

    def get_single_log_data(self, log_path):
        estimated_data_path, _ = get_single_log_pe_input_paths(log_path, '.mudp', '')

        print('Parsing .mudp data ...')
        parser = MudpHandler(estimated_data_path, self._mudp_parser_config_path, self._mudp_stream_def_path)
        parsed_data = parser.decode()

        print('Extracting estimated data ...')
        estimated_data = self._est_extractor.extract_data(parsed_data)

        print('Extracting reference data ...')
        reference_data = self._ref_extractor.extract_data(parsed_data)

        print('Extraction Post-processing ...')
        flag = self._ref_rel_falg.calc_flag(reference_data.objects.signals)
        reference_data.objects.signals = reference_data.objects.signals[flag]
        reference_data.objects.raw_signals = reference_data.objects.raw_signals[flag]

        estimated_data, reference_data = self._post_process_data(estimated_data, reference_data)

        print('Data extracted.')
        return estimated_data, reference_data

    @staticmethod
    def _post_process_data(estimated_data, reference_data):
        # TODO: Fix me in extractors DOH-21
        reference_data.host.signals['yaw_rate_variance'] = \
            DefaultReference.angle_rate_std(reference_data.host.signals['yaw_rate']) ** 2

        reference_data.host.signals['velocity_otg_variance_x'] = DefaultReference.vel_std ** 2
        reference_data.host.signals['velocity_otg_variance_y'] = DefaultReference.vel_std ** 2
        reference_data.host.signals['velocity_otg_covariance'] = 0.0

        estimated_data.sensors.per_look['min_azimuth'] = np.deg2rad(-75.0)
        estimated_data.sensors.per_look['max_azimuth'] = np.deg2rad(75.0)
        estimated_data.sensors.per_look['min_elevation'] = np.deg2rad(-5.0)
        estimated_data.sensors.per_look['max_elevation'] = np.deg2rad(5.0)

        estimated_data.detections.signals['range_variance'] = DefaultRadar.range_std ** 2
        estimated_data.detections.signals['azimuth_variance'] = DefaultRadar.azimuth_std ** 2
        estimated_data.detections.signals['range_rate_variance'] = DefaultRadar.range_rate_std ** 2

        return estimated_data, reference_data
