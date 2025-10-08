from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.providers.Preprocessing.TimeSynch.time_synch_iface import TimeSynchInterface


class SyncByPTP(TimeSynchInterface):
    """
    Synchronization by utilizing information from Precision Time Protocol. PTP provides mapping between publish time
    (in vigem timestamp domain) and global time (in radar time domain). Synchronization is done by:
    - transform slave time domain (vigem) to master time domain (radar)

    Assumptions: data was already updated with system latency information within extraction process -> object list and
    detection list headers contain evaluated system latency as one of the columns in signal data frames.
    """

    def __init__(self, reference_dt=0, **kwargs):
        """
        Object initializing function.
        :param kwargs:
        """
        super().__init__(**kwargs)
        self.master = None
        self.slave = None
        self.reference_dt = reference_dt
        self.vigem_to_global_time_diff_median = None
        self.vigem_to_global_time_diff_spread = None

    def synch(self,
              master_extracted_data: ExtractedData,
              slave_extracted_data: ExtractedData,
              *args, **kwargs) -> (ExtractedData, ExtractedData):
        """
        :param master_extracted_data: extracted data which will not be modified
        :type master_extracted_data: ExtractedData
        :param slave_extracted_data: extracted data which will  be modified - timestamp of each data set will be shifted
        :type slave_extracted_data: ExtractedData
        :param args:
        :param kwargs:
        :return:
        """
        self.master = master_extracted_data
        self.slave = slave_extracted_data

        try:
            self.get_vigem_to_global_time_diff()
            self.update_slave_timestamps_to_master_domain()
        except AttributeError:
            print('Cannot map vigem time domain to system domain, skipping synchronization ...')
        return master_extracted_data, slave_extracted_data

    def get_vigem_to_global_time_diff(self):
        vigem_mapping_signal_exists = False
        if hasattr(self.master.ptp_data, 'vigem_to_global_time_diff_median'):
            if self.master.ptp_data.vigem_to_global_time_diff_median is not None:
                vigem_mapping_signal_exists = True
        if vigem_mapping_signal_exists:
            self.vigem_to_global_time_diff_median = self.master.ptp_data.vigem_to_global_time_diff_median
            self.vigem_to_global_time_diff_spread = self.master.ptp_data.vigem_to_global_time_diff_spread
        else:
            raise AttributeError

    def update_slave_timestamps_to_master_domain(self):
        slave_data_sets = self.slave.get_data_sets_list()
        for ds in slave_data_sets:
            old_ts = ds.signals.loc[:, 'timestamp']
            new_ts = old_ts - self.vigem_to_global_time_diff_median
            ds.signals.loc[:, 'timestamp'] = new_ts + self.reference_dt
