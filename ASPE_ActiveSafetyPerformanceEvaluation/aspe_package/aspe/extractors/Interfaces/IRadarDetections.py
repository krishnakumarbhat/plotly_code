# coding=utf-8
"""
Main Radar Detection interface
"""
from typing import Optional, Union

import pandas as pd

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals
from aspe.utilities.MathFunctions import scs2vcs, vcs2scs
from aspe.utilities.nearest_neigbor_sync import sync_by_nearest_neighbor
from aspe.utilities.SupportingFunctions import filter_out_nan_columns, map_array_values


class IRadarDetections(IDataSet):
    """
    Main Radar Detections data set interface class
    """
    def __init__(self):
        super().__init__()
        self.angular_signals.extend(['azimuth', 'elevation'])
        signal_names = [
            GeneralSignals.sensor_id,
            PhysicalSignals.range,
            PhysicalSignals.range_variance,
            PhysicalSignals.azimuth,
            PhysicalSignals.azimuth_variance,
            PhysicalSignals.range_rate,
            PhysicalSignals.range_rate_variance,
            PhysicalSignals.amplitude,
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
        ]

        self.update_signals_definition(signal_names)

        self.max_slots_number: Optional[int] = None
        self.coordinate_system: Optional[str] = None

    def interpolate_values(self, new_timestamp_vals, new_scan_index_vals, timestamp_signal_name='timestamp'):
        grouped = self.signals.groupby(by=[timestamp_signal_name, 'scan_index']).size().reset_index()
        ts_signal = grouped.loc[:, timestamp_signal_name].to_numpy()
        si_signal = grouped.loc[:, 'scan_index'].to_numpy()

        ts_beg, ts_end = ts_signal[0], ts_signal[-1]
        within_mask = (ts_beg <= new_timestamp_vals) & (new_timestamp_vals <= ts_end)
        ts_within = new_timestamp_vals[within_mask]

        filtered_ts, coresponding_indexes = sync_by_nearest_neighbor(ts_signal, ts_signal, ts_within)
        filtered_si = si_signal[coresponding_indexes]

        filtered_signals = filter_out_nan_columns(self.signals)
        synced = filtered_signals.set_index(timestamp_signal_name).loc[filtered_ts, :].reset_index()

        filtered_ts_to_filtered_si_map = dict(zip(filtered_ts.tolist(), filtered_si.tolist()))
        updated_ts = synced.loc[:, timestamp_signal_name].to_numpy()
        new_si = map_array_values(updated_ts, filtered_ts_to_filtered_si_map, -1).astype(int)

        synced.loc[:, 'scan_index'] = new_si

        nan_columns = list(self.signals.columns.difference(synced.columns))
        synced = pd.concat([synced, pd.DataFrame(columns=nan_columns, index=synced.index)], axis=1)
        self.signals = synced

    def _transfer_to_iso_vracs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'VCS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] + dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_y'] = -self.signals.loc[:, 'position_y']
            self.signals.loc[:, 'azimuth'] = -self.signals.loc[:, 'azimuth']
            self.coordinate_system = 'ISO_VRACS'

    def _transfer_to_vcs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'ISO_VRACS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_y'] = -self.signals.loc[:, 'position_y']
            self.signals.loc[:, 'azimuth'] = -self.signals.loc[:, 'azimuth']
            self.coordinate_system = 'VCS'

    def get_base_name(self):
        """
        Get radars base name
        :return: detection base name
        """
        return 'detections'

    def vcs2scs(self, scs_location: Union[pd.Series, pd.DataFrame]):
        """
        Conversion from VCS coordinate to SCS coordinate.
        It affects:
            self.signals['position_x']
            self.signals['position_y']

        :param scs_location: SCS location series with several fields valid:
            scs_location.position_x
            scs_location.position_y
            scs_location.boresight_az_angle
        :type scs_location: Union[pd.Series, pd.DataFrame]

        :return:
        """
        if not self.coordinate_system == 'VCS':
            raise ValueError(f"Object can be converted only if it is in VCS, current coordinate is: "
                             f"{self.coordinate_system}")

        scs_x, scs_y = vcs2scs(self.signals['position_x'], self.signals['position_y'],
                               scs_location.position_x, scs_location.position_y, scs_location.boresight_az_angle)

        self.signals['position_x'] = scs_x
        self.signals['position_y'] = scs_y
        self.coordinate_system = 'SCS'

    def scs2vcs(self, scs_location: Union[pd.Series, pd.DataFrame]):
        """
        Conversion from SCS coordinate to VCS coordinate.
        It affects:
            self.signals['position_x']
            self.signals['position_y']

        :param scs_location: SCS location series with several fields valid:
            scs_location.position_x
            scs_location.position_y
            scs_location.boresight_az_angle
        :type scs_location: Union[pd.Series, pd.DataFrame]

        :return:
        """
        if not self.coordinate_system == 'SCS':
            raise ValueError(f"Object can be converted only if it is in SCS, current coordinate is: "
                             f"{self.coordinate_system}")

        vcs_x, vcs_y = scs2vcs(self.signals['position_x'], self.signals['position_y'],
                               scs_location.position_x, scs_location.position_y, scs_location.boresight_az_angle)

        self.signals['position_x'] = vcs_x
        self.signals['position_y'] = vcs_y
        self.coordinate_system = 'VCS'

    def filter_single_radar(self, radar_id: int = 1):
        """
        Filter single radar data based on sensor ID
        :param radar_id: ID of radar which should be kept
        :type radar_id int
        :return:
        """
        self.signals = self.signals[self.signals['sensor_id'] == radar_id]
