from aspe.providers.extraction_pipelines import F360MudpExtractionPipeline, RtRangeDvlExtractionPipeline
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.Preprocessing.ScanIndexSynch.nearest_timestamp_synch import NearestTimestampSynch
from aspe.providers.Preprocessing.TimeSynch.shift_slave_extracted_data import ShiftSlaveExtractedDataTimestamp

from .support_functions import get_single_log_pe_input_paths


class F360MudpRTRangeDataProvider(IPerfEvalInputDataProvider):
    def __init__(self, dbc_config: dict, msg_sets_config: dict, mudp_stream_def_path=None, f360_mudp_extractor_kwargs={}, shift_offset=0.03):
        self._mudp_extraction_pipeline = F360MudpExtractionPipeline(mudp_stream_def_path, f360_mudp_extractor_kwargs)
        self._rt_range_extraction_pipeline = RtRangeDvlExtractionPipeline(dbc_config, msg_sets_config)
        self._time_synch = ShiftSlaveExtractedDataTimestamp(offset=shift_offset, inplace=True)
        self._scan_index_synch = NearestTimestampSynch(inplace=True)

    def get_single_log_data(self, log_path):
        estimated_data_path, reference_data_path = get_single_log_pe_input_paths(log_path, '.mudp', '.dvl')

        estimated_data = self._mudp_extraction_pipeline.process(estimated_data_path)
        reference_data = self._rt_range_extraction_pipeline.process(reference_data_path)

        # Synchronization is done inplace - optimization
        self._time_synch.synch(master_extracted_data=estimated_data, slave_extracted_data=reference_data)
        self._scan_index_synch.synch(estimated_data=estimated_data, reference_data=reference_data,
                                     master_data_set=estimated_data.tracker_info,
                                     slave_data_set=reference_data.host)

        return estimated_data, reference_data
