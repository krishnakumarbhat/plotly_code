import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.providers.Preprocessing.ScanIndexSynch.scan_index_synch_iface import ScanIndexSynchInterface


class ScanIndexInterpolationSynch(ScanIndexSynchInterface):
    def __init__(self, ts_master_name='timestamp', ts_slave_name='timestamp'):
        self.ts_master_name = ts_master_name
        self.ts_slave_name = ts_slave_name

    def synch(self, estimated_data: ExtractedData, reference_data: ExtractedData, master_data_set: IDataSet,
              slave_data_set: IDataSet, *args, **kwargs) -> (ExtractedData, ExtractedData):

        master_signals, slave_signals = master_data_set.signals, slave_data_set.signals
        slave_extracted_data = self._get_slave_extracted_data(estimated_data, reference_data, slave_data_set)

        master_ts, master_scan_index = self._get_master_timestamp_and_scan_index(master_signals)
        slave_extracted_data.interpolate_values(master_ts, master_scan_index)

    def _get_master_timestamp_and_scan_index(self, master_signals: pd.DataFrame):
        master_ts = []
        master_scan_index = []
        for scan_index, scan in master_signals[['scan_index', self.ts_master_name]].groupby(by='scan_index'):
            master_scan_index.append(scan_index)
            master_ts.append(scan[self.ts_master_name].mean())
        master_ts = np.array(master_ts)
        master_scan_index = np.array(master_scan_index)
        return master_ts, master_scan_index

    def _get_slave_extracted_data(self, estimated_data: ExtractedData, reference_data: ExtractedData,
                                  slave_data_set: IDataSet):
        est_data_sets = estimated_data.get_data_sets_list()
        if any([ds for ds in est_data_sets if ds is slave_data_set]):
            slave_extracted_data = estimated_data
        else:
            slave_extracted_data = reference_data
        return slave_extracted_data
