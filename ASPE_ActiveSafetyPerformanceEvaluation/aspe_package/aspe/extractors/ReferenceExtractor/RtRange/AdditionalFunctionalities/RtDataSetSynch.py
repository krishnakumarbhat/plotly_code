from warnings import warn

from aspe.extractors.Interfaces import IDataSet


class RtDataSetSynch:
    """
    For synchronization between Host - Master and Objects - Slave. Synchronize by UTC timestamp, treat Master as
    reference point.
    """

    def __init__(self, f_clear_nans=True, f_crop_master_slave=True, time_instances_slave_compensation=None):
        self.f_clear_nans = f_clear_nans
        self.f_crop_master_slave = f_crop_master_slave
        self.time_instances_slave_compensation = time_instances_slave_compensation

    def synch(self, master: IDataSet, slave: IDataSet):
        self._master = master
        self._slave = slave

        if self._check_if_utc_signal_available():
            intersection_si_ts_utc, master_intersection_mask = self._get_intersection_master_slave_si_utc()
            self._synchronize_slave(intersection_si_ts_utc)

            if self.f_clear_nans:
                RtDataSetSynch._remove_rows_by_name_nans(data_set=self._master, name='scan_index')
                RtDataSetSynch._remove_rows_by_name_nans(data_set=self._slave, name='scan_index')

                RtDataSetSynch._remove_rows_by_name_nans(data_set=self._master, name='timestamp')
                RtDataSetSynch._remove_rows_by_name_nans(data_set=self._slave, name='timestamp')
            if self.f_crop_master_slave:
                self._drop_master_mismatch_to_slave(master_intersection_mask)

            self._reset_indexes(drop=True)
            self._set_slave_to_master_synch_flag(self._master, True)
            self._set_slave_to_master_synch_flag(self._slave, True)

        elif self.time_instances_slave_compensation is not None:
            warn('UTC signal not present in master or slave. Using constant shift')
            if self.time_instances_slave_compensation > 0:
                self._slave.signals = \
                    self._slave.signals.iloc[self.time_instances_slave_compensation:, :].reset_index(drop=True)
            else:
                self._master.signals = \
                    self._master.signals.iloc[self.time_instances_slave_compensation:, :].reset_index(drop=True)

        else:
            warn('Master and Slave had not been synchronized')
            self._set_slave_to_master_synch_flag(self._master, False)
            self._set_slave_to_master_synch_flag(self._slave, False)

        slave_rows_count = len(self._slave.signals)
        master_rows_count = len(self._master.signals)
        if slave_rows_count > master_rows_count:
            self._slave.signals = self._slave.signals.iloc[: master_rows_count, :]
        else:
            self._master.signals = self._master.signals.iloc[: slave_rows_count, :]
        self._slave.signals.loc[:, 'scan_index'] = self._master.signals.loc[:, 'scan_index']
        self._slave.signals.loc[:, 'timestamp'] = self._master.signals.loc[:, 'timestamp']
        self._set_slave_to_master_synch_flag(self._master, True)
        self._set_slave_to_master_synch_flag(self._slave, True)

        return self._master, self._slave

    def _check_if_utc_signal_available(self):
        """
        Checking if any time data are not only nan values, if so synchronization is not performed
        :return: bool
        """
        f_master_si_nan = self._master.signals['scan_index'].isnull().all()  # scan_index
        f_master_ts_nan = self._master.signals['timestamp'].isnull().all()  # timestamp
        f_master_utc_nan = self._master.signals['utc_timestamp'].isnull().all()  # utc_timestamp

        f_slave_utc_nan = self._slave.signals['utc_timestamp'].isnull().all()  # utc_timestamp

        if f_master_si_nan or f_master_ts_nan or f_master_utc_nan:
            return False
        elif f_slave_utc_nan:
            return False
        else:
            return True

    def _set_utc_timestamp_as_index(self):
        self._master.signals = self._master.signals.set_index('utc_timestamp')
        self._slave.signals = self._slave.signals.set_index('utc_timestamp')

    def _reset_indexes(self, drop=False):
        """
        Resetting indexes,
        :param drop: if True does not add current index to dataframe
        """
        self._master.signals = self._master.signals.reset_index(drop=drop)
        self._slave.signals = self._slave.signals.reset_index(drop=drop)

    def _get_intersection_master_slave_si_utc(self):
        """
        Getting a common part if time data from Master and Slave
        :return: intersection_si_ts_utc - dataframe that contains common scan_index, timestamps and utc_timestamp
                                          for Master and Slave.
                 master_intersection_si_utc_mask - Series of bool values of which indexes for Master are common with
                                                   Slave.
        """
        master_si_utc = self._master.signals[['scan_index', 'timestamp', 'utc_timestamp']]
        slave_si_utc = self._slave.signals[['scan_index', 'timestamp', 'utc_timestamp']]

        master_intersection_si_utc_mask = master_si_utc['utc_timestamp'].isin(slave_si_utc['utc_timestamp'])

        master_intersection_si_ts_utc = master_si_utc[master_intersection_si_utc_mask]

        intersection_si_ts_utc = master_intersection_si_ts_utc.set_index('utc_timestamp')

        return intersection_si_ts_utc, master_intersection_si_utc_mask

    def _synchronize_slave(self, intersection_si_ts_utc):
        """
        Setting to Slave synchronized scan_index and timestamp
        """
        self._slave.signals = self._slave.signals.drop(columns=['scan_index', 'timestamp'])
        self._slave.signals = self._slave.signals.join(intersection_si_ts_utc, rsuffix='', on='utc_timestamp')

    @staticmethod
    def _remove_rows_by_name_nans(data_set: IDataSet,  name):
        """
        After synchronization, missing frames are filled with nans.
        Removing rows where name == nan for data_set's dataframe
        :data_set: Master or Slave data_set
        :name: string name of dataframe column
        """
        data_set.signals.dropna(subset=[name], inplace=True)

    def _drop_master_mismatch_to_slave(self, master_intersection_mask):
        """
        In case of different size of Master and Slave this function, setting both dataframe to same size
        Removing rows from Master which Slave does not contain (by utc_timestamp)
        :master_intersection_mask: bool mask which indexes are available
        """
        self._master.signals = self._master.signals[master_intersection_mask]

    @staticmethod
    def _set_slave_to_master_synch_flag(data_set: IDataSet, bool_value: bool):
        """
        Setting to True synchronization slave to master flag in data_set's properties
        :return:
        """
        data_set.f_slave_to_master_synch = bool_value
