import logging
import os.path
import shutil
from abc import ABC, abstractmethod
from datetime import datetime

import h5py
import numpy as np


class ConvertToKeg(ABC):
    def __init__(self, existing_log_file, keg_file_path):
        self.existing_log_file = existing_log_file
        self.keg_file_path = keg_file_path
        self._metadata_list = ['annotations',
                               'conversion_history',
                               'creation_date',
                               'version_component_resim_interface',
                               'version_component_resim_serialization',
                               'version_keg']

    def save_to_keg_file(self):
        data_to_save = self.convert_to_keg_readable_format()
        self.save(data_to_save)

    @abstractmethod
    def convert_to_keg_readable_format(self):
        pass

    def save(self, data_to_save):
        """Function creates .keg file """
        logging.info('Save data to .keg file : %s', self.keg_file_path)
        try:
            if self.confirm_file_overwriting():
                with h5py.File(self.keg_file_path, 'w') as h5file:
                    self.fill_keg_file(h5file, data_to_save)
        except OSError as exc:
            err = RuntimeError('Failed to create .keg file. File is in use.')
            logging.error(err, exc_info=True, stack_info=True)
            raise err from exc
        finally:
            self.cleanup_routine()
            logging.info('FINISHED.')

    def confirm_file_overwriting(self):
        f_proceed = True
        if os.path.exists(self.keg_file_path):  # prevent unintentional .keg overwriting
            logging.info('File already exists. Do you want to overwrite? [y/n]:')
            while True:
                f_overwrite = str(input()).lower()
                if f_overwrite == 'y':
                    logging.info('File will be overwritten.')
                    f_proceed = True
                    break
                elif f_overwrite == 'n':
                    logging.info('File will not be overwritten.')
                    f_proceed = False
                    break
                else:
                    logging.info('Please enter [y/n]?')
        return f_proceed

    def fill_keg_file(self, h5file: h5py.File, data_to_save: dict):
        self.add_metadata(h5file)
        self.create_groups(h5file)
        self.fill_primary_signals(h5file, data_to_save)
        self.fill_log_signals(h5file, data_to_save)

    def add_metadata(self, h5file: h5py.File):
        """Adds hardcoded metadata to .keg file. It is necessary for Component Resim."""
        for data in self._metadata_list:
            if data == 'conversion_history':
                h5file.attrs[data] = ''
            elif data == 'creation_date':
                h5file.attrs[data] = str(datetime.now())
            elif data == 'version_keg':
                h5file.attrs[data] = 4
            else:
                h5file.attrs[data] = 0

    def create_groups(self, h5file: h5py.File):
        h5file.create_group(name='data')
        h5file['data'].create_group(name='uninitialized')
        h5file.create_group(name='serialization')
        h5file.create_group(name='metadata')

    def fill_primary_signals(self, h5file: h5py.File, data_to_save: dict):

        h5file['serialization'].create_dataset('initialize',
                                               data=np.array([0]),
                                               chunks=True,
                                               compression='gzip',
                                               compression_opts=9)
        h5file['serialization'].create_dataset('terminate',
                                               data=np.array([0]),
                                               chunks=True,
                                               compression='gzip',
                                               compression_opts=9)

    @abstractmethod
    def fill_log_signals(self, h5file: h5py.File, data_to_save: dict):
        pass

    def cleanup_routine(self):
        """Cleanup routine, deletes temporary files."""
        if os.path.exists('temp'):
            shutil.rmtree('temp')
