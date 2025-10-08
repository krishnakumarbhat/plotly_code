import logging
import warnings

import h5py
import numpy as np
from tqdm import tqdm

from aspe.parsers.XtrkParser.XtrkParser import XtrkParser
from aspe.utilities.log_to_keg_converter.ConvertToKeg import ConvertToKeg


class ConvertXtrkToKeg(ConvertToKeg):
    def __init__(self, existing_log_file, keg_file_path):
        super().__init__(existing_log_file, keg_file_path)
        self._keys_to_export = ['host',
                                'host_raw',
                                'host_props',
                                'host_calib',
                                'raw_dets',
                                'det_props',
                                'hist_det',
                                'hist_det_props',
                                'sensors',
                                'sensor_props',
                                'sens_prop_int_ref_buf',
                                'object_track']

        self._key_mapper = {'host': 'host',
                            'host_raw': 'host_raw',
                            'host_props': 'host_props',
                            'host_calib': 'host_calib',
                            'sensors': 'sensors',
                            'sensor_props': 'sensor_props',
                            'sens_prop_int_ref_buf': 'sens_prop_int_ref_buf',
                            'det_props': 'dets_prop',
                            'raw_dets': 'dets_raw',
                            'hist_det': 'hist_det',
                            'hist_det_props': 'hist_det_props',
                            'object_track': 'object_track'}

    def convert_to_keg_readable_format(self):
        xtrk_data = XtrkParser().parse(self.existing_log_file)
        data_to_save = self.extract_xtrk_by_keys(xtrk_data)
        return data_to_save

    def extract_xtrk_by_keys(self, xtrk_data) -> dict:
        data_to_save = {}

        logging.info('Extract data from .xtrk file.')
        for key in tqdm(self._keys_to_export):
            data_to_save[key] = xtrk_data.get(key, None)
            if data_to_save[key] is None:
                err = RuntimeError(f'Missing xtrk data: {key}')
                logging.error(err, stack_info=True)
                raise err

        trkrInfo = xtrk_data.get('trkrInfo', None)

        if trkrInfo is not None:
            data_to_save['time_us'] = trkrInfo.get('time_us', None)
            if data_to_save['time_us'] is None:
                err = RuntimeError('Missing xtrk data: time_us in trkrInfo')
                logging.error(err, stack_info=True)
                raise err
            data_to_save['cnt_loops'] = trkrInfo.get('cnt_loops', None)
            if data_to_save['cnt_loops'] is None:
                warnings.warn('Missing xtrk data: cnt_loops in trkrInfo')
        else:
            err = RuntimeError('Missing xtrk data: trkrInfo')
            logging.error(err, stack_info=True)
            raise err

        return data_to_save

    def create_groups(self, h5file: h5py.File):
        super().create_groups(h5file)
        h5file['data/uninitialized'].create_group(name='DETECTIONS')
        h5file['data/uninitialized'].create_group(name='VEHICLE')
        h5file['data/uninitialized'].create_group(name='RADAR_PARAMS')
        h5file['data/uninitialized'].create_group(name='OBJECTS')

    def fill_primary_signals(self, h5file: h5py.File, data_to_save: dict):
        super().fill_primary_signals(h5file, data_to_save)
        # timestamps is in seconds thus conversion is needed from time_us (microseconds)
        h5file.create_dataset('timestamps',
                              data=np.array(data_to_save['time_us'], dtype=float) / 1e+06,
                              chunks=True,
                              compression='gzip',
                              compression_opts=9)
        h5file['data/uninitialized'].create_dataset('cnt_loops',
                                                    data=data_to_save['cnt_loops'])

    def fill_log_signals(self, h5file: h5py.File, data_to_save: dict):
        for save_key in tqdm(data_to_save.keys()):
            mapped_key = self._key_mapper.get(save_key, None)
            if mapped_key is not None:
                group_name = self.choose_group(mapped_key)
                h5file['data/uninitialized'][group_name].create_group(name=mapped_key)
                for key, item in data_to_save[save_key].items():
                    if key == 'variable.number_of_valid_detections':
                        self.handle_special_case(h5file, data_to_save)

                    h5file['data/uninitialized'][group_name][mapped_key].create_dataset(key,
                                                                                        data=item,
                                                                                        chunks=True,
                                                                                        compression='gzip',
                                                                                        compression_opts=9)
                    self.add_signal_attributes(h5file, group_name, mapped_key, key, type=item.dtype)

    @staticmethod
    def choose_group(mapped_key: str) -> str:
        if 'host' in mapped_key:
            group_name = 'VEHICLE'
        elif 'det' in mapped_key:
            group_name = 'DETECTIONS'
        elif 'sens' in mapped_key:
            group_name = 'RADAR_PARAMS'
        elif 'object' in mapped_key:
            group_name = 'OBJECTS'
        return group_name

    @staticmethod
    def add_signal_attributes(h5file: h5py.File, group_name: str, mapped_key: str, key: str, type: np.dtype):
        h5file['data/uninitialized'][group_name][mapped_key][key].attrs['signal_name'] = '/'.join(
            [group_name, mapped_key, key])
        h5file['data/uninitialized'][group_name][mapped_key][key].attrs['signal_type'] = ''
        h5file['data/uninitialized'][group_name][mapped_key][key].attrs['signal_underlying_type'] = str(type)
        h5file['data/uninitialized'][group_name][mapped_key][key].attrs['signal_interface_type'] = 'INPUT'

    @staticmethod
    def handle_special_case(h5file: h5py.File, data_to_save: dict):
        """Add processed number_of_valid_detections signal from sensors to DETECTION. Needed for OCG CR."""
        data = np.sum(data_to_save['sensors']['variable.number_of_valid_detections'], axis=1).reshape(-1, 1)
        h5file['data/uninitialized/DETECTIONS'].create_dataset('number_of_valid_detections',
                                                               data=data, chunks=True,
                                                               compression='gzip',
                                                               compression_opts=9)
        h5file['data/uninitialized/DETECTIONS/number_of_valid_detections'].attrs['signal_name'] = \
            'DETECTIONS/number_of_valid_detections'
        h5file['data/uninitialized/DETECTIONS/number_of_valid_detections'].attrs['signal_type'] = ''
        h5file['data/uninitialized/DETECTIONS/number_of_valid_detections'].attrs['signal_underlying_type'] = str(
            data.dtype)
        h5file['data/uninitialized/DETECTIONS/number_of_valid_detections'].attrs['signal_interface_type'] = 'INPUT'
