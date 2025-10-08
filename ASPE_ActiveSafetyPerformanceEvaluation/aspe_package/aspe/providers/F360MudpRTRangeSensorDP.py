from copy import deepcopy

import numpy as np

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IRadarSensors import IRadarSensors
from aspe.providers.extraction_pipelines import F360MudpExtractionPipeline, RtRangeDvlExtractionPipeline
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.Preprocessing.TimeSynch.shift_slave_extracted_data import ShiftSlaveExtractedDataTimestamp
from aspe.providers.support_functions import get_single_log_pe_input_paths


class F360MudpRTRangeSensorDP(IPerfEvalInputDataProvider):
    """
    Data Provider for RT range reference and sensor data
    Note:
        Data is not scan index synchronized since this need to be done for each radar separately
    """
    def __init__(self, dbc_config: dict, msg_sets_config: dict, mudp_stream_def_path=None, shift_offset=0.03):
        f360_mudp_extractor_kwargs = {'f_extract_objects': False,
                                      'f_extract_internal_objects': False,
                                      'f_extract_sensors': True,
                                      'f_extract_host': False,
                                      'f_extract_detections': True}
        self._mudp_extraction_pipeline = F360MudpExtractionPipeline(mudp_stream_def_path,  f360_mudp_extractor_kwargs)
        self._rt_range_extraction_pipeline = RtRangeDvlExtractionPipeline(dbc_config, msg_sets_config)
        self._time_synch = ShiftSlaveExtractedDataTimestamp(offset=shift_offset, inplace=True)

    def get_single_log_data(self, log_path: str) -> (ExtractedData, ExtractedData):
        """
        Get single log data for given log path
        Note:
            Data is time synchronized but not scan index synchronized.
        :param log_path: path to log from which data should be provided. .mudp and .dlv logs will be automatically
                         found based on log name.
        :type log_path: str
        :return: tuple of (ExtractedData, ExtractedData),
                 First extracted data should contain estimated data sets:
                 -sensors
                 -detections
                 Second extracted data should contain reference data sets:
                 -objects
                 -host
        """
        estimated_data_path, reference_data_path = get_single_log_pe_input_paths(log_path, '.mudp', '.dvl')

        estimated_data = self._mudp_extraction_pipeline.process(estimated_data_path)
        reference_data = self._rt_range_extraction_pipeline.process(reference_data_path)

        print('Extraction Post-processing ...')
        self._time_synch.synch(master_extracted_data=estimated_data, slave_extracted_data=reference_data)
        est_data_time_synch, ref_data_time_synch = self._post_process_data(estimated_data, reference_data)

        print('Data extracted.')
        return est_data_time_synch, ref_data_time_synch

    @staticmethod
    def _post_process_data(estimated_data, reference_data):
        """
        Adding missing data.
        :param estimated_data:
        :param reference_data:
        :return: tuple of (ExtractedData, ExtractedData)
        """
        estimated_data.sensors.per_look['min_azimuth'] = np.deg2rad(-75.0)
        estimated_data.sensors.per_look['max_azimuth'] = np.deg2rad(75.0)
        estimated_data.sensors.per_look['min_elevation'] = np.deg2rad(-5.0)
        estimated_data.sensors.per_look['max_elevation'] = np.deg2rad(5.0)

        reference_data.sensors = IRadarSensors()
        reference_data.sensors.per_sensor = deepcopy(estimated_data.sensors.per_sensor)

        return estimated_data, reference_data
