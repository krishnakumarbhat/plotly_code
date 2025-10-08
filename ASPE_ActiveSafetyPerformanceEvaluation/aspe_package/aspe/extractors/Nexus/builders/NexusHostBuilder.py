from collections import defaultdict
from datetime import datetime
from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.Interfaces.IHost import IHost


class NexusHostBuilder(IBuilder):
    signal_mapper = {
        # ASPE signature    'pandora signature
        'raw_speed':        'speed',
        'velocity_otg_x':   'speed',
        'yaw_rate':         'yawRate',
        'scan_index':       'sampleIndex',
    }
    irrelevant_data_columns = [
        'createdBy',
        'updatedBy',
        'createdAt',
        'updatedAt',
    ]

    def __init__(self, parsed_data, save_raw_signals=False, rear_axle_to_front_bumper_distance=None):
        super().__init__(parsed_data)
        self._lifespan_mask = None
        self._parsed_data_shape = None
        self._raw_signals = None
        self.data_set = IHost()
        self.save_raw_signals = save_raw_signals
        self.rear_axle_to_front_bumper_distance = rear_axle_to_front_bumper_distance

    def build(self):
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()
        self._extract_properties()
        return self.data_set

    def _extract_raw_signals(self):
        host_signals = self._parsed_data['host']['signals']

        raw_signals_dict = defaultdict(list)
        for sample_index, sample in enumerate(host_signals):
            raw_signals_dict['sampleIndex'].append(sample_index)
            for signal_name, signal_value in sample.toDict().items():
                raw_signals_dict[signal_name].append(signal_value)

        self._raw_signals = pd.DataFrame(raw_signals_dict).drop(self.irrelevant_data_columns, axis=1)

        if self.save_raw_signals:
            self.data_set.raw_signals = self._raw_signals

    def _extract_non_mappable_signals(self):
        # It is assumed that there is only one column (one host)
        self.data_set.signals['unique_id'] = np.full(self._parsed_data_shape, 0)
        self.data_set.signals['slot_id'] = 0

        self._extract_timestamp()
        self._extract_position()
        self._extract_abs_raw_speed()

    def _extract_timestamp(self):
        timestamps = self._raw_signals.loc[:, 'timestamp'].to_numpy().astype('str')
        timestamps = np.char.replace(timestamps, 'Z', '')
        self.data_set.signals.loc[:, 'timestamp'] = np.array(
            [datetime.fromisoformat(t).timestamp() for t in timestamps])

    def _extract_position(self):
        # Position in VCS is always fixed and equal to zero
        self.data_set.signals['position_x'] = 0
        self.data_set.signals['position_y'] = 0

    def _extract_abs_raw_speed(self):
        self.data_set.signals['abs_raw_speed'] = self.data_set.signals['raw_speed'].abs()

    def _extract_properties(self):
        host_info = self._parsed_data['host']['properties']

        self.data_set.coordinate_system = 'VCS'  # center of front bumper
        if self.rear_axle_to_front_bumper_distance is not None:
            self.data_set.dist_of_rear_axle_to_front_bumper = self.rear_axle_to_front_bumper_distance
        elif hasattr(host_info, 'vehicleDimensions'):
            self.data_set.dist_of_rear_axle_to_front_bumper = host_info.vehicleDimensions['wheelBase'] + \
                                                              host_info.vehicleDimensions['frontOverhang']
