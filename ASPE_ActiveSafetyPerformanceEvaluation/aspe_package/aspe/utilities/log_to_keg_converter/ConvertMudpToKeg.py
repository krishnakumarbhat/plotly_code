import h5py
from tqdm import tqdm

from aspe.parsers.MudpParser import MudpParser
from aspe.parsers.MudpParser.f360_defaults_configs import source_to_parse, streams_to_read, unknown_size_per_stream
from aspe.utilities.log_to_keg_converter.ConvertToKeg import ConvertToKeg


class ConvertMudpToKeg(ConvertToKeg):

    def __init__(self, existing_log_file, keg_file_path, mudp_stream_definition):
        super().__init__(existing_log_file, keg_file_path)
        self.mudp_stream_definition = mudp_stream_definition
        self._signals_headers = {4: 'Vehicle_Info_Log_T',
                                 7: 'Tracker_Info_Log_T',
                                 70: 'F360_Object_Log_Output_T',
                                 71: 'F360_Detection_Log_Output_T',
                                 72: 'F360_Sensor_Calib_Log_Output_T'}

    def convert_to_keg_readable_format(self):

        parser = MudpParser(streams_to_read=streams_to_read, source_to_parse=source_to_parse,
                            unknown_size_per_stream=unknown_size_per_stream,
                            mudp_stream_def_path=self.mudp_stream_definition)
        parsed_data = parser.parse(self.existing_log_file)

        # withdraw data
        data_to_save = {}

        for k, v in parsed_data['parsed_data'].items():
            if k in self._signals_headers.keys():
                new_key = self._signals_headers[k]
                data_to_save[new_key] = parsed_data['parsed_data'][k]

        return data_to_save

    @staticmethod
    def fill_log_signals(h5file: h5py.File, data_to_save: dict):
        for k, v in tqdm(data_to_save.items()):
            h5file['data/uninitialized'].create_group(name=k)
            for key, value in data_to_save[k].items():
                h5file['data/uninitialized'][k].create_dataset(key,
                                                               data=value,
                                                               chunks=True,
                                                               compression='gzip',
                                                               compression_opts=9)
