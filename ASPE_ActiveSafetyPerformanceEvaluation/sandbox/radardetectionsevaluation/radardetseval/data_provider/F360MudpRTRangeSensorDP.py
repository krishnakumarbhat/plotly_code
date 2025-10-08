import numpy as np

from AptivPerformanceEvaluation.DataProviders.extraction_pipelines import F360MudpExtractionPipeline
from AptivPerformanceEvaluation.DataProviders.extraction_pipelines import RtRangeDvlExtractionPipeline
from AptivPerformanceEvaluation.DataProviders.support_functions import get_single_log_pe_input_paths

from AptivPerformanceEvaluation.Preprocessing.TimeSynch.RtRangeSynch.shift_rt_by_offset_synch import \
    ShiftReferenceByOffsetSynchronization
from radardetseval.configs.default_radar import DefaultRadar

from radardetseval.configs.defaut_reference import DefaultReference


class F360MudpRTRangeSensorDP:
    def __init__(self, mudp_parser_config_path, dvl_parser_config_path, mudp_stream_def_path=None, shift_offset=0.03):
        f360_mudp_extractor_kwargs = {'f_extract_objects': False,
                                      'f_extract_internal_objects': False}
        self._mudp_extraction_pipeline = F360MudpExtractionPipeline(mudp_parser_config_path, mudp_stream_def_path,
                                                                    f360_mudp_extractor_kwargs)
        self._rt_range_extraction_pipeline = RtRangeDvlExtractionPipeline(dvl_parser_config_path)
        self._time_synch = ShiftReferenceByOffsetSynchronization(offset=shift_offset)

    def get_single_log_data(self, log_path):
        estimated_data_path, reference_data_path = get_single_log_pe_input_paths(log_path, '.mudp', '.dvl')

        estimated_data = self._mudp_extraction_pipeline.process(estimated_data_path)
        reference_data = self._rt_range_extraction_pipeline.process(reference_data_path)

        est_data_time_synch, ref_data_time_synch = self._synchronize_time_domains(estimated_data, reference_data)

        est_data_time_synch, ref_data_time_synch = self._post_process_data(est_data_time_synch, ref_data_time_synch)
        print('Data extracted.')
        return est_data_time_synch, ref_data_time_synch

    def _synchronize_time_domains(self, estimated_data, reference_data):
        estimated_data_synch, reference_data_synch = self._time_synch.synch(estimated_data=estimated_data,
                                                                            reference_data=reference_data, inplace=True)
        return estimated_data_synch, reference_data_synch

    @staticmethod
    def _post_process_data(estimated_data, reference_data):
        # TODO: Fix me in extractors DOH-21
        reference_data.objects.signals['position_variance_x'] = DefaultReference.pos_std ** 2
        reference_data.objects.signals['position_variance_y'] = DefaultReference.pos_std ** 2
        reference_data.objects.signals['position_covariance'] = 0.0

        reference_data.objects.signals['velocity_otg_variance_x'] = DefaultReference.vel_std ** 2
        reference_data.objects.signals['velocity_otg_variance_y'] = DefaultReference.vel_std ** 2
        reference_data.objects.signals['velocity_otg_covariance'] = 0.0

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